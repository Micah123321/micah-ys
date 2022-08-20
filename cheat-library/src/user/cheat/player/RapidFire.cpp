#include "pch-il2cpp.h"
#include "RapidFire.h"

#include <helpers.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{
	static void LCBaseCombat_DoHitEntity_Hook(app::LCBaseCombat* __this, uint32_t targetID, app::AttackResult* attackResult,
		bool ignoreCheckCanBeHitInMP, MethodInfo* method);
	static void VCAnimatorEvent_HandleProcessItem_Hook(app::MoleMole_VCAnimatorEvent* __this,
		app::MoleMole_VCAnimatorEvent_MoleMole_VCAnimatorEvent_AnimatorEventPatternProcessItem* processItem,
		app::AnimatorStateInfo processStateInfo, app::MoleMole_VCAnimatorEvent_MoleMole_VCAnimatorEvent_TriggerMode__Enum mode, MethodInfo* method);

	RapidFire::RapidFire() : Feature(),
		NF(f_Enabled, u8"攻击乘数", u8"攻击修改", false),
		NF(f_MultiHit, u8"多重攻击", u8"攻击修改", false),
		NF(f_Multiplier, u8"打击乘数", u8"攻击修改", 2),
		NF(f_OnePunch, u8"一拳模式", u8"攻击修改", false),
		NF(f_Randomize, u8"随机化", u8"攻击修改", false),
		NF(f_minMultiplier, u8"最小乘数", u8"攻击修改", 1),
		NF(f_maxMultiplier, u8"最大乘数", u8"攻击修改", 3),
		NF(f_MultiTarget, u8"多目标", u8"攻击修改", false),
		NF(f_MultiTargetRadius, u8"多目标半径", u8"攻击修改", 20.0f),
		NF(f_MultiAnimation, u8"多目标动画", u8"攻击修改", false)
	{
		HookManager::install(app::MoleMole_LCBaseCombat_DoHitEntity, LCBaseCombat_DoHitEntity_Hook);
		HookManager::install(app::MoleMole_VCAnimatorEvent_HandleProcessItem, VCAnimatorEvent_HandleProcessItem_Hook);
	}

    const FeatureGUIInfo& RapidFire::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"攻击效果", u8"玩家", true };
        return info;
    }

    void RapidFire::DrawMain()
    {
		ConfigWidget(u8"开启", f_Enabled, u8"使攻击乘数。需要选择一种工作模式.");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"选择下面的任何一种或两种模式.");

		ConfigWidget(u8"多重攻击模式", f_MultiHit, u8"开启多重攻击.\n" \
            u8"增加你的攻击次数.\n" \
            u8"这没有经过很好的测试，可以通过反作弊检测到.\n" \
            u8"不建议与主要帐户或高值使用.\n" \
			u8"某些多命中攻击的已知问题，例如小E, Ayaka CA等.");

		ImGui::Indent();

		ConfigWidget(u8"一拳模式", f_OnePunch, u8"根据敌人的HP计算需要多少次攻击才能杀死敌人\n" \
			u8"然后用它来相应地设置乘数.\n" \
			u8"可能更安全，但乘数计算可能不正确.");

		ConfigWidget(u8"随机乘数", f_Randomize, u8"将乘数在最小乘数和最大乘数之间随机化.");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"这将覆盖一击模式!");

		if (!f_OnePunch) {
			if (!f_Randomize)
			{
				ConfigWidget(u8"乘数", f_Multiplier, 1, 2, 1000, u8"袭击数乘法器.");
			}
			else
			{
				ConfigWidget(u8"最小乘数", f_minMultiplier, 1, 2, 1000, u8"攻击次数最小乘数.");
				ConfigWidget(u8"最大乘数", f_maxMultiplier, 1, 2, 1000, u8"攻击次数最大乘数.");
			}
		}

		ImGui::Unindent();

		ConfigWidget(u8"多目标", f_MultiTarget, u8"在指定的目标半径内启用多目标攻击.\n" \
			u8"根据设定击中初始目标周围的所有有效目标.\n" \
			u8"伤害数字只会出现在初始目标上，但所有有效目标都会受到伤害.\n" \
			u8"如果关闭了多命中，并且在一个目标上仍然有多个数字，请检查Debug部分中的实体管理器，以查看是否有不可见的实体.\n" \
			u8"警告这可能会导致极端延迟和快速禁令."
		);

		ImGui::Indent();
		ConfigWidget(u8"半径 (m)", f_MultiTargetRadius, 0.1f, 5.0f, 50.0f, u8"检查有效目标的半径.");
		ImGui::Unindent();

		ConfigWidget(u8"多目标动画", f_MultiAnimation, u8"使多的动画攻击.\n" \
			u8"你是否需要记住角色的音频也会被垃圾信息所淹没.");
	}

	bool RapidFire::NeedStatusDraw() const
	{
		return f_Enabled && (f_MultiHit || f_MultiTarget || f_MultiAnimation);
	}

	void RapidFire::DrawStatus()
	{
		if (f_MultiHit)
		{
			if (f_Randomize)
				ImGui::Text(u8"多重攻击 随机[%d|%d]", f_minMultiplier.value(), f_maxMultiplier.value());
			else if (f_OnePunch)
				ImGui::Text(u8"多重攻击 [一拳]");
			else
				ImGui::Text(u8"多重攻击 [%d]", f_Multiplier.value());
		}
		if (f_MultiTarget)
			ImGui::Text(u8"多目标 [%.01fm]", f_MultiTargetRadius.value());

		if (f_MultiAnimation)
			ImGui::Text(u8"多目标动画");
	}

	RapidFire& RapidFire::GetInstance()
	{
		static RapidFire instance;
		return instance;
	}


	int RapidFire::CalcCountToKill(float attackDamage, uint32_t targetID)
	{
		if (attackDamage == 0)
			return f_Multiplier;

		auto& manager = game::EntityManager::instance();
		auto targetEntity = manager.entity(targetID);
		if (targetEntity == nullptr)
			return f_Multiplier;

		auto baseCombat = targetEntity->combat();
		if (baseCombat == nullptr)
			return f_Multiplier;

		auto safeHP = baseCombat->fields._combatProperty_k__BackingField->fields.HP;
		auto HP = app::MoleMole_SafeFloat_get_Value(safeHP, nullptr);
		int attackCount = (int)ceil(HP / attackDamage);
		return std::clamp(attackCount, 1, 200);
	}

	int RapidFire::GetAttackCount(app::LCBaseCombat* combat, uint32_t targetID, app::AttackResult* attackResult)
	{
		if (!f_MultiHit)
			return 1;

		auto& manager = game::EntityManager::instance();
		auto targetEntity = manager.entity(targetID);
		auto baseCombat = targetEntity->combat();
		if (baseCombat == nullptr)
			return 1;

		int countOfAttacks = f_Multiplier;
		if (f_OnePunch)
		{
			app::MoleMole_Formula_CalcAttackResult(combat->fields._combatProperty_k__BackingField,
				baseCombat->fields._combatProperty_k__BackingField,
				attackResult, manager.avatar()->raw(), targetEntity->raw(), nullptr);
			countOfAttacks = CalcCountToKill(attackResult->fields.damage, targetID);
		}
		if (f_Randomize)
		{
			countOfAttacks = rand() % (f_maxMultiplier.value() - f_minMultiplier.value()) + f_minMultiplier.value();
			return countOfAttacks;
		}

		return countOfAttacks;
	}

	bool IsAvatarOwner(game::Entity entity)
	{
		auto& manager = game::EntityManager::instance();
		auto avatarID = manager.avatar()->runtimeID();

		while (entity.isGadget())
		{
			game::Entity temp = entity;
			entity = game::Entity(app::MoleMole_GadgetEntity_GetOwnerEntity(reinterpret_cast<app::GadgetEntity*>(entity.raw()), nullptr));
			if (entity.runtimeID() == avatarID)
				return true;
		}

		return false;

	}

	bool IsAttackByAvatar(game::Entity& attacker)
	{
		if (attacker.raw() == nullptr)
			return false;

		auto& manager = game::EntityManager::instance();
		auto avatarID = manager.avatar()->runtimeID();
		auto attackerID = attacker.runtimeID();

		return attackerID == avatarID || IsAvatarOwner(attacker);
	}

	bool IsConfigByAvatar(game::Entity& attacker)
	{
		if (attacker.raw() == nullptr)
			return false;

		auto& manager = game::EntityManager::instance();
		auto avatarID = manager.avatar()->raw()->fields._configID_k__BackingField;
		auto attackerID = attacker.raw()->fields._configID_k__BackingField;
		// Taiga#5555: IDs can be found in ConfigAbility_Avatar_*.json or GadgetExcelConfigData.json
		bool bulletID = attackerID >= 40000160 && attackerID <= 41069999;

		return avatarID == attackerID || bulletID;
	}

	bool IsValidByFilter(game::Entity* entity)
	{
		if (game::filters::combined::OrganicTargets.IsValid(entity) ||
			game::filters::monster::SentryTurrets.IsValid(entity) ||
			game::filters::combined::Ores.IsValid(entity) ||
			game::filters::puzzle::Geogranum.IsValid(entity) ||
			game::filters::puzzle::LargeRockPile.IsValid(entity) ||
			game::filters::puzzle::SmallRockPile.IsValid(entity))
			return true;
		return false;
	}

	// Raises when any entity do hit event.
	// Just recall attack few times (regulating by combatProp)
	// It's not tested well, so, I think, anticheat can detect it.
	static void LCBaseCombat_DoHitEntity_Hook(app::LCBaseCombat* __this, uint32_t targetID, app::AttackResult* attackResult,
		bool ignoreCheckCanBeHitInMP, MethodInfo* method)
	{
		auto attacker = game::Entity(__this->fields._._._entity);
		RapidFire& rapidFire = RapidFire::GetInstance();
		if (!IsConfigByAvatar(attacker) || !IsAttackByAvatar(attacker) || !rapidFire.f_Enabled)
			return CALL_ORIGIN(LCBaseCombat_DoHitEntity_Hook, __this, targetID, attackResult, ignoreCheckCanBeHitInMP, method);

		auto& manager = game::EntityManager::instance();
		auto originalTarget = manager.entity(targetID);

		if (!IsValidByFilter(originalTarget))
			return CALL_ORIGIN(LCBaseCombat_DoHitEntity_Hook, __this, targetID, attackResult, ignoreCheckCanBeHitInMP, method);

		std::vector<cheat::game::Entity*> validEntities;
		validEntities.push_back(originalTarget);

		if (rapidFire.f_MultiTarget)
		{
			auto filteredEntities = manager.entities();
			for (const auto& entity : filteredEntities) {
				auto distance = originalTarget->distance(entity);

				if (entity->runtimeID() == manager.avatar()->runtimeID())
					continue;

				if (entity->runtimeID() == targetID)
					continue;

				if (distance > rapidFire.f_MultiTargetRadius)
					continue;

				if (!IsValidByFilter(entity))
					continue;

				validEntities.push_back(entity);
			}
		}

		for (const auto& entity : validEntities) {

			if (rapidFire.f_MultiHit) {
				int attackCount = rapidFire.GetAttackCount(__this, entity->runtimeID(), attackResult);
				for (int i = 0; i < attackCount; i++)
					app::MoleMole_LCBaseCombat_FireBeingHitEvent(__this, entity->runtimeID(), attackResult, method);
			}
			else app::MoleMole_LCBaseCombat_FireBeingHitEvent(__this, entity->runtimeID(), attackResult, method);
		}

		CALL_ORIGIN(LCBaseCombat_DoHitEntity_Hook, __this, targetID, attackResult, ignoreCheckCanBeHitInMP, method);
	}

	static void VCAnimatorEvent_HandleProcessItem_Hook(app::MoleMole_VCAnimatorEvent* __this,
		app::MoleMole_VCAnimatorEvent_MoleMole_VCAnimatorEvent_AnimatorEventPatternProcessItem* processItem,
		app::AnimatorStateInfo processStateInfo, app::MoleMole_VCAnimatorEvent_MoleMole_VCAnimatorEvent_TriggerMode__Enum mode, MethodInfo* method)
	{
		auto attacker = game::Entity(__this->fields._._._entity);
		RapidFire& rapidFire = RapidFire::GetInstance();

		if (rapidFire.f_MultiAnimation && IsAttackByAvatar(attacker))
			processItem->fields.lastTime = 0;

		CALL_ORIGIN(VCAnimatorEvent_HandleProcessItem_Hook, __this, processItem, processStateInfo, mode, method);
	}
}

