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
		NF(f_Enabled, u8"�Զ��ƻ�", u8"�Զ��ƻ�", false),
		NF(f_DestroyOres, u8"�ƻ���ʯ", u8"�Զ��ƻ�", false),
		NF(f_DestroyShields, u8"�ƻ�����", u8"�Զ��ƻ�", false),
		NF(f_DestroyDoodads, u8"�ƻ�װ����", u8"�Զ��ƻ�", false),
		NF(f_DestroyPlants, u8"�ƻ�ֲ��", u8"�Զ��ƻ�", false),
		NF(f_Range, u8"��Χ", u8"�Զ��ƻ�", 10.0f)
	{
		HookManager::install(app::MoleMole_LCAbilityElement_ReduceModifierDurability, LCAbilityElement_ReduceModifierDurability_Hook);
	}

	const FeatureGUIInfo& AutoDestroy::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"�Զ��ƻ�����", u8"����", true };
		return info;
	}

	void AutoDestroy::DrawMain()
	{
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"��ע�⡣��������ڼ�ⷽ�滹û����ȫ���Թ�.\n"
			"������������Ҫ�ʻ����ֵʹ��.");

		ConfigWidget(u8"����", f_Enabled, u8"˲��ݻٷ�Χ�ڵ�����������.");
		ImGui::Indent();
		ConfigWidget(u8"��ʯ", f_DestroyOres, u8"��ʯ�ͱ��壬����羧�塢�����");
		ConfigWidget(u8"����", f_DestroyShields, u8"��Ԩ��ʦ/������/ʷ��ķ���ơ�");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"���ռ���!");
		ConfigWidget(u8"���ռ���", f_DestroyDoodads, u8"ľͰ�����ӡ���ƿ��");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"�ǳ�Σ�յ�!");
		ConfigWidget(u8"ֲ��", f_DestroyPlants, u8"�ѹ�Ӣ���ӡ�ӣ����.");
		ImGui::Unindent();
		ConfigWidget(u8"��Χ (m)", f_Range, 0.1f, 1.0f, 15.0f);
	}

	bool AutoDestroy::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void AutoDestroy::DrawStatus()
	{
		ImGui::Text(u8"�ݻ� [%.01fm%s%s%s%s%s]",
			f_Range.value(),
			f_DestroyOres || f_DestroyShields || f_DestroyDoodads || f_DestroyPlants ? u8"|" : u8"",
			f_DestroyOres ? u8"��" : u8"",
			f_DestroyShields ? u8"��" : u8"",
			f_DestroyDoodads ? u8"��" : u8"",
			f_DestroyPlants ? u8"ֲ" : u8"");
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

