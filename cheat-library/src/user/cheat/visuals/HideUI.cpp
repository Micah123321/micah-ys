#include "pch-il2cpp.h"
#include "HideUI.h"

#include <helpers.h>
#include <cheat/events.h>

namespace cheat::feature
{
    app::GameObject* ui_camera{};

    HideUI::HideUI() : Feature(),
		NFEX(f_Enabled, u8"Òþ²ØUI", u8"Òþ²ØUI", u8"ÊÓ¾õ", false, false)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(HideUI::OnGameUpdate);
    }

    const FeatureGUIInfo& HideUI::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"Òþ²ØUI", u8"ÊÓ¾õ", false };
        return info;
    }

    void HideUI::DrawMain()
    {
        ConfigWidget(f_Enabled, u8"Òþ²ØÓÎÏ·ÄÚUI.");
    }

    bool HideUI::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void HideUI::DrawStatus()
    {
        ImGui::Text(u8"Òþ²ØUI");
    }

    HideUI& HideUI::GetInstance()
    {
        static HideUI instance;
        return instance;
    }

    void HideUI::OnGameUpdate()
    {
        if (f_Enabled)
        {
            if (ui_camera == nullptr)
                ui_camera = app::GameObject_Find(string_to_il2cppi(u8"/UICamera"), nullptr);

            
            if (ui_camera)
                app::GameObject_SetActive(ui_camera, false, nullptr);
        }
        else
        {
            if (ui_camera)
                app::GameObject_SetActive(ui_camera, true, nullptr);
        }
    }
}