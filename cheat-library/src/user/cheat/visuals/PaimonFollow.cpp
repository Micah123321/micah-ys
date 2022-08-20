#include "pch-il2cpp.h"
#include "PaimonFollow.h"
#include <helpers.h>
#include <cheat/events.h>


namespace cheat::feature
{
    namespace GameObject {
        app::GameObject* Paimon = nullptr;
        app::GameObject* ProfileLayer = nullptr;
    }

    PaimonFollow::PaimonFollow() : Feature(),
        NFEX(f_Enabled, u8"派蒙跟随", u8"派蒙跟随", u8"视觉", false, false),
        toBeUpdate(), nextUpdate(0)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(PaimonFollow::OnGameUpdate);
    }

    const FeatureGUIInfo& PaimonFollow::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"派蒙跟随", u8"视觉", true };
        return info;
    }

    void PaimonFollow::DrawMain()
    {
        ConfigWidget(f_Enabled, u8"要显示paimon，请打开该功能，打开配置文件(esc)并关闭它. \n" \
            u8"如果传送后paimon消失了，不要禁用该功能，打开和关闭配置文件.");
    }

    bool PaimonFollow::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void PaimonFollow::DrawStatus()
    {
        ImGui::Text(u8"派蒙跟随");
    }

    PaimonFollow& PaimonFollow::GetInstance()
    {
        static PaimonFollow instance;
        return instance;
    }

    void PaimonFollow::OnGameUpdate()
    {
        if (!f_Enabled)
            return;

        auto currentTime = util::GetCurrentTimeMillisec();
        if (currentTime < nextUpdate)
            return;

        if (GameObject::Paimon == nullptr) {
            GameObject::Paimon = app::GameObject_Find(string_to_il2cppi(u8"/EntityRoot/OtherGadgetRoot/NPC_Guide_Paimon(Clone)"), nullptr);  
        }

        if (GameObject::ProfileLayer == nullptr) {
            GameObject::ProfileLayer = app::GameObject_Find(string_to_il2cppi(u8"/Canvas/Pages/PlayerProfilePage"), nullptr);
        }
          
        if (GameObject::Paimon != nullptr && GameObject::ProfileLayer != nullptr) {
            auto ProfileOpen = app::GameObject_get_active(GameObject::ProfileLayer, nullptr);

            if (ProfileOpen) {
                app::GameObject_set_active(GameObject::Paimon, false, nullptr);
            }
            else {
                app::GameObject_set_active(GameObject::Paimon, true, nullptr);
            }
        }
        nextUpdate = currentTime + (int)f_DelayUpdate;
    }
}