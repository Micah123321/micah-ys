#include "pch-il2cpp.h"
#include "NoFog.h"

#include <helpers.h>
#include <cheat/events.h>

namespace cheat::feature
{
    static bool _prevEnabledState = false;
    NoFog::NoFog() : Feature(),
        NFEX(f_Enabled, u8"去除雾", u8"去除雾", u8"视觉", false, false)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(NoFog::OnGameUpdate);
    }

    const FeatureGUIInfo& NoFog::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"去除雾", u8"视觉", false };
        return info;
    }

    void NoFog::DrawMain()
    {
        ConfigWidget(f_Enabled, u8"消除了雾.");
    }

    bool NoFog::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void NoFog::DrawStatus()
    {
        ImGui::Text(u8"无雾");
    }

    NoFog& NoFog::GetInstance()
    {
        static NoFog instance;
        return instance;
    }

    void NoFog::OnGameUpdate()
    {
		if (_prevEnabledState != f_Enabled)
		{
			app::RenderSettings_set_fog(!f_Enabled, nullptr);
			_prevEnabledState = f_Enabled;
		}
    }
}

