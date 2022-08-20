#include "pch-il2cpp.h"
#include "OculiTeleport.h"

#include <helpers.h>
#include <cheat/game/filters.h>

namespace cheat::feature 
{
    OculiTeleport::OculiTeleport() : ItemTeleportBase(u8"神之眼传送", u8"神之眼")
    { }

	const FeatureGUIInfo& OculiTeleport::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"神之眼传送", u8"传送", true };
		return info;
	}

    OculiTeleport& OculiTeleport::GetInstance()
	{
		static OculiTeleport instance;
		return instance;
	}

	bool OculiTeleport::IsValid(game::Entity* entity) const
	{
		return game::filters::combined::Oculies.IsValid(entity);
	}

}

