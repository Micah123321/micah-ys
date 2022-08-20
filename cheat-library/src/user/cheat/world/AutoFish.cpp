#include "pch-il2cpp.h"
#include "AutoFish.h"

#include <helpers.h>
#include <algorithm>

#include <cheat/events.h>
#include <cheat/game/util.h>

namespace cheat::feature
{
    AutoFish::AutoFish() : Feature(),
        NFEX(f_Enabled, u8"自动钓鱼", u8"自动钓鱼", u8"自动钓鱼", false, false),
        NF(f_DelayBeforeCatch, u8"捕获前延迟", u8"自动钓鱼", 2000),
        NF(f_AutoRecastRod, u8"重新挥杆", u8"自动钓鱼", true),
        NF(f_DelayBeforeRecast, u8"重新挥杆延迟", u8"自动钓鱼", 500)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(AutoFish::OnGameUpdate);

        HookManager::install(app::MoleMole_FishingModule_RequestFishCastRod, FishingModule_RequestFishCastRod_Hook);
        HookManager::install(app::MoleMole_FishingModule_onFishChosenNotify, FishingModule_onFishChosenNotify_Hook);
        HookManager::install(app::MoleMole_FishingModule_OnFishBiteRsp, FishingModule_OnFishBiteRsp_Hook);
        HookManager::install(app::MoleMole_FishingModule_OnFishBattleBeginRsp, FishingModule_OnFishBattleBeginRsp_Hook);
        HookManager::install(app::MoleMole_FishingModule_OnFishBattleEndRsp, FishingModule_OnFishBattleEndRsp_Hook);
        HookManager::install(app::MoleMole_FishingModule_OnExitFishingRsp, FishingModule_OnExitFishingRsp_Hook);
    }

    const FeatureGUIInfo& AutoFish::GetGUIInfo() const
    {
      static const FeatureGUIInfo info{u8"钓鱼", u8"世界", true};
        return info;
    }

    void AutoFish::DrawMain()
    {
        ConfigWidget(u8"启动", f_Enabled, u8"自动钓鱼.");
        ConfigWidget(u8"钓鱼延迟 (ms)", f_DelayBeforeCatch, 100, 500, 4000, u8"过了这一段时间就可以捕鱼了 (in ms).");

        ImGui::Spacing();

        ConfigWidget(f_AutoRecastRod, u8"如果启用，在看不到动画的情况下重新挥杆。");
        ConfigWidget(u8"重新挥杆延迟 (ms)", f_DelayBeforeRecast, 10, 100, 4000, u8"过了这一段时间就可以重新挥杆了 (in ms).");
    }

    bool AutoFish::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void AutoFish::DrawStatus()
    {
        ImGui::Text(u8"自动钓鱼");
    }

    AutoFish& AutoFish::GetInstance()
    {
        static AutoFish instance;
        return instance;
    }

    void AutoFish::FishingModule_onFishChosenNotify_Hook(void* __this, void* notify, MethodInfo* method)
    {
        CALL_ORIGIN(FishingModule_onFishChosenNotify_Hook, __this, notify, method);

        auto& autoFish = GetInstance();
        if (!autoFish.f_Enabled)
            return;

        app::MoleMole_FishingModule_RequestFishBite(__this, nullptr);
    }

    void AutoFish::FishingModule_OnFishBiteRsp_Hook(void* __this, app::FishBiteRsp* rsp, MethodInfo* method)
    {
        auto& autoFish = GetInstance();
        if (!autoFish.f_Enabled)
        {
            CALL_ORIGIN(FishingModule_OnFishBiteRsp_Hook, __this, rsp, method);
            return;
        }

        app::MoleMole_FishingModule_RequestFishBattleBegin(__this, nullptr);
    }

    void AutoFish::FishingModule_OnFishBattleBeginRsp_Hook(void* __this, app::FishBattleBeginRsp* rsp, MethodInfo* method)
    {
        auto& autoFish = GetInstance();
        if (!autoFish.f_Enabled)
        {
            CALL_ORIGIN(FishingModule_OnFishBattleBeginRsp_Hook, __this, rsp, method);
            return;
        }

        std::lock_guard<std::mutex> catchLock(autoFish.m_BattleFinishTimestampMutex);
        autoFish.m_BattleFinishTimestamp = app::MoleMole_TimeUtil_get_NowTimeStamp(nullptr) + autoFish.f_DelayBeforeCatch;
    }

    void AutoFish::FishingModule_OnFishBattleEndRsp_Hook(void* __this, app::FishBattleEndRsp* rsp, MethodInfo* method)
    {
        CALL_ORIGIN(FishingModule_OnFishBattleEndRsp_Hook, __this, rsp, method);

        auto& autoFish = GetInstance();

        if (rsp->fields.battleResult_ == app::FishBattleResult__Enum::Cancel
            || rsp->fields.battleResult_ == app::FishBattleResult__Enum::Exit)
        {
            std::lock_guard<std::mutex> _lock2(autoFish.m_RecastTimestampMutex);
            autoFish.m_RecastTimestamp = 0;
            return;
        }

        if (!autoFish.f_Enabled)
            return;

        if (rsp->fields.retcode_ != 0)
        {
            LOG_WARNING(u8"捕鱼失败，在%u毫秒内重试", autoFish.f_DelayBeforeCatch);
            std::lock_guard<std::mutex> catchLock(autoFish.m_BattleFinishTimestampMutex);
            autoFish.m_BattleFinishTimestamp = app::MoleMole_TimeUtil_get_NowTimeStamp(nullptr) + autoFish.f_DelayBeforeCatch;
            return;
        }

        if (!autoFish.f_AutoRecastRod)
            return;

        std::lock_guard<std::mutex> _lock(autoFish.m_RecastTimestampMutex);
        autoFish.m_RecastTimestamp = app::MoleMole_TimeUtil_get_NowTimeStamp(nullptr) + autoFish.f_DelayBeforeRecast;
    }

    void AutoFish::FishingModule_OnExitFishingRsp_Hook(void* __this, void* rsp, MethodInfo* method)
    {
        CALL_ORIGIN(FishingModule_OnExitFishingRsp_Hook, __this, rsp, method);

        auto& autoFish = GetInstance();

        std::lock_guard<std::mutex> _lock(autoFish.m_RecastTimestampMutex);
        autoFish.m_LastCastData.exist = false;
    }

    void AutoFish::FishingModule_RequestFishCastRod_Hook(void* __this, uint32_t baitId, uint32_t rodId, app::Vector3 pos, uint32_t rodEntityId, MethodInfo* method)
    {
        CALL_ORIGIN(FishingModule_RequestFishCastRod_Hook, __this, baitId, rodId, pos, rodEntityId, method);

        auto& autoFish = GetInstance();

        autoFish.m_LastCastData.exist = true;
        autoFish.m_LastCastData.fishingModule = __this;
        autoFish.m_LastCastData.baitId = baitId;
        autoFish.m_LastCastData.rodId = rodId;
        autoFish.m_LastCastData.pos = pos;
        autoFish.m_LastCastData.rodEntityId = rodEntityId;

        autoFish.m_RecastTimestamp = 0;
    }

    void AutoFish::OnGameUpdate()
    {
        auto timestamp = app::MoleMole_TimeUtil_get_NowTimeStamp(nullptr);

        std::lock_guard<std::mutex> _lock(m_BattleFinishTimestampMutex);
        std::lock_guard<std::mutex> _lock2(m_RecastTimestampMutex);

        if (!m_LastCastData.exist)
            return;

        if (m_BattleFinishTimestamp != 0 && timestamp > m_BattleFinishTimestamp)
        {
            m_BattleFinishTimestamp = 0;

            app::MoleMole_FishingModule_RequestFishBattleEnd(m_LastCastData.fishingModule, app::FishBattleResult__Enum::Succ, f_DelayBeforeCatch == 4.0f,
                static_cast<float>(f_DelayBeforeCatch / 1000), nullptr);
        }

        if (m_RecastTimestamp != 0 && timestamp > m_RecastTimestamp)
        {
            m_RecastTimestamp = 0;

            app::MoleMole_FishingModule_RequestFishCastRod(m_LastCastData.fishingModule, m_LastCastData.baitId,
                m_LastCastData.rodId, m_LastCastData.pos, m_LastCastData.rodEntityId, nullptr);
        }
    }
}

