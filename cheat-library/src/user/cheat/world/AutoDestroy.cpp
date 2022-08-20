#include "pch-il2cpp.h"
#include "AutoDestroy.h"

#include <helpers.h>
#include <algorithm>

#include <cheat/events.h>
#include <cheat/game/SimpleFilter.h>
#include <cheat/game/EntityManager.h>
#include <cheat/world/AutoChallenge.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{
	static void LCAbilityElement_ReduceModifierDurability_Hook(app::LCAbilityElement* __this, int32_t modifierDurabilityIndex, float reduceDurability, app::Nullable_1_Single_ deltaTime, MethodInfo* method);

	AutoDestroy::AutoDestroy() : Feature(),
		NF(f_Enabled, u8"自动破坏", u8"自动破坏", false),
		NF(f_DestroyOres, u8"破坏矿石", u8"自动破坏", false),
		NF(f_DestroyShields, u8"破坏盾牌", u8"自动破坏", false),
		NF(f_DestroyDoodads, u8"破坏装饰物", u8"自动破坏", false),
		NF(f_DestroyPlants, u8"破坏植物", u8"自动破坏", false),
		NF(f_Range, u8"范围", u8"自动破坏", 10.0f)
	{
		HookManager::install(app::MoleMole_LCAbilityElement_ReduceModifierDurability, LCAbilityElement_ReduceModifierDurability_Hook);
	}

	const FeatureGUIInfo& AutoDestroy::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"自动破坏对象", u8"世界", true };
		return info;
	}

	void AutoDestroy::DrawMain()
	{
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"请注意。这个特性在检测方面还没有完全测试过.\n"
			"不建议用于主要帐户或高值使用.");

		ConfigWidget(u8"启用", f_Enabled, u8"瞬间摧毁范围内的无生命物体.");
		ImGui::Indent();
		ConfigWidget(u8"矿石", f_DestroyOres, u8"矿石和变体，例如电晶体、精髓等");
		ConfigWidget(u8"盾牌", f_DestroyShields, u8"深渊法师/丘丘王/史莱姆盾牌。");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"风险极大!");
		ConfigWidget(u8"风险极大", f_DestroyDoodads, u8"木桶、盒子、花瓶等");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"非常危险的!");
		ConfigWidget(u8"植物", f_DestroyPlants, u8"蒲公英种子、樱花等.");
		ImGui::Unindent();
		ConfigWidget(u8"范围 (m)", f_Range, 0.1f, 1.0f, 15.0f);
	}

	bool AutoDestroy::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void AutoDestroy::DrawStatus()
	{
		ImGui::Text(u8"摧毁 [%.01fm%s%s%s%s%s]",
			f_Range.value(),
			f_DestroyOres || f_DestroyShields || f_DestroyDoodads || f_DestroyPlants ? u8"|" : u8"",
			f_DestroyOres ? u8"矿" : u8"",
			f_DestroyShields ? u8"盾" : u8"",
			f_DestroyDoodads ? u8"饰" : u8"",
			f_DestroyPlants ? u8"植" : u8"");
	}

	AutoDestroy& AutoDestroy::GetInstance()
	{
		static AutoDestroy instance;
		return instance;
	}

	// Thanks to @RyujinZX
	// Every ore has ability element component
	// Durability of ability element is a ore health
	// Every tick ability element check reducing durability, for ore in calm state `reduceDurability` equals 0, means HP don't change
	// We need just change this value to current durability or above to destroy ore
	// This function also can work with some types of shields (TODO: improve killaura with this function)
	static void LCAbilityElement_ReduceModifierDurability_Hook(app::LCAbilityElement* __this, int32_t modifierDurabilityIndex, float reduceDurability, app::Nullable_1_Single_ deltaTime, MethodInfo* method)
	{
		auto& manager = game::EntityManager::instance();
		auto& autoDestroy = AutoDestroy::GetInstance();
		auto& autoChallenge = AutoChallenge::GetInstance();
		auto entity = __this->fields._._._entity;
		// call origin ReduceModifierDurability without correct modifierDurabilityIndex will coz game crash.
		// so use this hook function to destroy challenge's bombbarrel
		if (autoChallenge.f_Enabled && autoChallenge.f_BombDestroy &&
			autoChallenge.f_Range > manager.avatar()->distance(entity) &&
			game::filters::puzzle::Bombbarrel.IsValid(manager.entity(entity))
			)
		{
			reduceDurability = 1000.f;
		}
		if (autoDestroy.f_Enabled &&
			autoDestroy.f_Range > manager.avatar()->distance(entity) &&
			(
				(autoDestroy.f_DestroyOres && game::filters::combined::Ores.IsValid(manager.entity(entity))) ||
				(autoDestroy.f_DestroyDoodads && (game::filters::combined::Doodads.IsValid(manager.entity(entity)) || game::filters::chest::SBramble.IsValid(manager.entity(entity)))) ||
				(autoDestroy.f_DestroyShields && !game::filters::combined::MonsterBosses.IsValid(manager.entity(entity)) && (
					game::filters::combined::MonsterShielded.IsValid(manager.entity(entity)) ||									// For shields attached to monsters, e.g. abyss mage shields.
					game::filters::combined::MonsterEquips.IsValid(manager.entity(entity)))) ||									// For shields/weapons equipped by monsters, e.g. rock shield.
					(autoDestroy.f_DestroyPlants && game::filters::combined::PlantDestroy.IsValid(manager.entity(entity)))		// For plants e.g dandelion seeds.
				)
			)
		{
			// This value always above any ore durability
			reduceDurability = 1000;
		}
		CALL_ORIGIN(LCAbilityElement_ReduceModifierDurability_Hook, __this, modifierDurabilityIndex, reduceDurability, deltaTime, method);
	}

}

