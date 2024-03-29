#include "pch-il2cpp.h"
#include "FPSUnlock.h"

#include <helpers.h>
#include <cheat/events.h>

namespace cheat::feature
{
    FPSUnlock::FPSUnlock() : Feature(),
        NF(f_Enabled, u8"帧率解锁", u8"视觉::帧率解锁", false),
        NF(f_Fps, u8"FPS", u8"视觉::帧率解锁", 240)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(FPSUnlock::OnGameUpdate);
    }

    const FeatureGUIInfo& FPSUnlock::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"帧率解锁", u8"视觉", false };
        return info;
    }

    void FPSUnlock::DrawMain()
    {
        ConfigWidget(u8"", f_Enabled); ImGui::SameLine();
        ConfigWidget(f_Fps, 1, 30, 360, u8"解锁更高的帧速率.");
    }

    bool FPSUnlock::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void FPSUnlock::DrawStatus()
    {
        ImGui::Text(u8"帧率解锁 [%d]", f_Fps.value());
    }

    FPSUnlock& FPSUnlock::GetInstance()
    {
        static FPSUnlock instance;
        return instance;
    }

    void FPSUnlock::OnGameUpdate()
    {
        static bool _lastEnabledStatus = false;
        static int _originFPS = 30;
        if (_lastEnabledStatus && !f_Enabled)
        {
            app::Application_set_targetFrameRate(_originFPS, nullptr);
        }
        else if (!_lastEnabledStatus && f_Enabled)
        {
            _originFPS = app::Application_get_targetFrameRate(nullptr);
        }
        _lastEnabledStatus = f_Enabled;

        if (f_Enabled)
            app::Application_set_targetFrameRate(f_Fps, nullptr);
    }
}