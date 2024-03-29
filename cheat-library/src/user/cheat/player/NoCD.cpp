#include "pch-il2cpp.h"
#include "NoCD.h"

#include <helpers.h>
#include <fmt/chrono.h>

namespace cheat::feature
{
	static bool HumanoidMoveFSM_CheckSprintCooldown_Hook(void* __this, MethodInfo* method);
	static bool LCAvatarCombat_IsEnergyMax_Hook(void* __this, MethodInfo* method);
	static bool LCAvatarCombat_OnSkillStart(app::LCAvatarCombat* __this, uint32_t skillID, float cdMultipler, MethodInfo* method);
	static bool LCAvatarCombat_IsSkillInCD_1(app::LCAvatarCombat* __this, app::LCAvatarCombat_LCAvatarCombat_SkillInfo* skillInfo, MethodInfo* method);

	static void ActorAbilityPlugin_AddDynamicFloatWithRange_Hook(app::MoleMole_ActorAbilityPlugin* __this, app::String* key, float value, float minValue, float maxValue,
		bool forceDoAtRemote, MethodInfo* method);

	static std::list<std::string> abilityLog;

	NoCD::NoCD() : Feature(),
		NF(f_AbilityReduce, u8"减少技能/大招的冷却时间", u8"无冷却", false),
		NF(f_TimerReduce, u8"减少计时器", u8"无冷却", 1.f),
		NF(f_UtimateMaxEnergy, u8"大招的最大能量", u8"无冷却", false),
		NF(f_Sprint, u8"没有冲刺冷却时间", u8"无冷却", false),
		NF(f_InstantBow, u8"即时弓", u8"无冷却", false)
	{
		HookManager::install(app::MoleMole_LCAvatarCombat_IsEnergyMax, LCAvatarCombat_IsEnergyMax_Hook);
		HookManager::install(app::MoleMole_LCAvatarCombat_IsSkillInCD_1, LCAvatarCombat_IsSkillInCD_1);

		HookManager::install(app::MoleMole_HumanoidMoveFSM_CheckSprintCooldown, HumanoidMoveFSM_CheckSprintCooldown_Hook);
		HookManager::install(app::MoleMole_ActorAbilityPlugin_AddDynamicFloatWithRange, ActorAbilityPlugin_AddDynamicFloatWithRange_Hook);

	}

	const FeatureGUIInfo& NoCD::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"减少冷却", u8"玩家", true };
		return info;
	}

	void NoCD::DrawMain()
	{

		ConfigWidget(u8"大招最多能量", f_UtimateMaxEnergy,
			u8"消除元素爆发的能量需求.\n" \
			u8"(能源泡沫可能看起来不完整，但仍然可用。)");

		ConfigWidget(u8"## 能力降低", f_AbilityReduce); ImGui::SameLine();
		ConfigWidget(u8"减少技能/破裂的冷却时间", f_TimerReduce, 1.f, 1.f, 6.0f,
			u8"减少元素技能和爆发的冷却时间.\n"\
			u8"1.0 -无CD, 2.0或更高-增加计时器值.");

		ConfigWidget(f_Sprint, u8"消除冲刺之间的延迟.");

		ConfigWidget(u8"蓄力弓", f_InstantBow, u8"禁用弓箭冲锋的冷却时间.\n" \
			u8"Fischl的已知问题.");

		if (f_InstantBow) {
			ImGui::Text(u8"如果瞬间弓冲锋不起作用:");
			TextURL(u8"请在GitHub上投稿.", u8"https://github.com/Akebi-Group/Akebi-GC/issues/281", false, false);
			if (ImGui::TreeNode(u8"Ability Log [DEBUG]"))
			{
				if (ImGui::Button(u8"Copy to Clipboard"))
				{
					ImGui::LogToClipboard();

					ImGui::LogText(u8"Ability Log:\n");

					for (auto& logEntry : abilityLog)
						ImGui::LogText(u8"%s\n", logEntry.c_str());

					ImGui::LogFinish();
				}

				for (std::string& logEntry : abilityLog)
					ImGui::Text(logEntry.c_str());

				ImGui::TreePop();
			}
		}
	}

	bool NoCD::NeedStatusDraw() const
	{
		return f_InstantBow || f_AbilityReduce || f_Sprint;
	}

	void NoCD::DrawStatus()
	{
		ImGui::Text(u8"冷却时间\n[%s%s%s%s%s]",
			f_AbilityReduce ? fmt::format(u8"减少 x{:.1f}", f_TimerReduce.value()).c_str() : u8"",
			f_AbilityReduce && (f_InstantBow || f_Sprint) ? u8"|" : u8"",
			f_InstantBow ? u8"弓" : u8"",
			f_InstantBow && f_Sprint ? u8"|" : u8"",
			f_Sprint ? u8"跑" : u8"");
	}

	NoCD& NoCD::GetInstance()
	{
		static NoCD instance;
		return instance;
	}

	static bool LCAvatarCombat_IsEnergyMax_Hook(void* __this, MethodInfo* method)
	{
		NoCD& noCD = NoCD::GetInstance();
		if (noCD.f_UtimateMaxEnergy)
			return true;

		return CALL_ORIGIN(LCAvatarCombat_IsEnergyMax_Hook, __this, method);
	}

	// Multipler CoolDown Timer Old | RyujinZX#6666
	static bool LCAvatarCombat_OnSkillStart(app::LCAvatarCombat* __this, uint32_t skillID, float cdMultipler, MethodInfo* method) {
		NoCD& noCD = NoCD::GetInstance();
		if (noCD.f_AbilityReduce)
		{
			if (__this->fields._targetFixTimer->fields._._timer_k__BackingField > 0) {
				cdMultipler = noCD.f_TimerReduce / 3;
			}
			else {
				cdMultipler = noCD.f_TimerReduce / 1;
			}
		}
		return CALL_ORIGIN(LCAvatarCombat_OnSkillStart, __this, skillID, cdMultipler, method);
	}

	// Timer Speed Up / CoolDown Reduce New | RyujinZX#6666
	static bool LCAvatarCombat_IsSkillInCD_1(app::LCAvatarCombat* __this, app::LCAvatarCombat_LCAvatarCombat_SkillInfo* skillInfo, MethodInfo* method) {
		NoCD& noCD = NoCD::GetInstance();
		if (noCD.f_AbilityReduce)
		{
			auto cdTimer = app::MoleMole_SafeFloat_get_Value(skillInfo->fields.cdTimer, nullptr); // Timer start value in the game

			if (cdTimer > noCD.f_TimerReduce)
			{
				struct app::SafeFloat MyValueProtect = app::MoleMole_SafeFloat_set_Value(noCD.f_TimerReduce - 1.0f, nullptr); // Subtract -1 from the current timer value
				skillInfo->fields.cdTimer = MyValueProtect;
			}
		}
		return CALL_ORIGIN(LCAvatarCombat_IsSkillInCD_1, __this, skillInfo, method);
	}

	// Check sprint cooldown, we just return true if sprint no cooldown enabled.
	static bool HumanoidMoveFSM_CheckSprintCooldown_Hook(void* __this, MethodInfo* method)
	{
		NoCD& noCD = NoCD::GetInstance();
		if (noCD.f_Sprint)
			return true;

		return CALL_ORIGIN(HumanoidMoveFSM_CheckSprintCooldown_Hook, __this, method);
	}

	// This function raise when abilities, whose has charge, is charging, like a bow.
	// value - increase value
	// min and max - bounds of charge.
	// So, to charge make full charge m_Instantly, just replace value to maxValue.
	static void ActorAbilityPlugin_AddDynamicFloatWithRange_Hook(app::MoleMole_ActorAbilityPlugin* __this, app::String* key, float value, float minValue, float maxValue,
		bool forceDoAtRemote, MethodInfo* method)
	{
		std::time_t t = std::time(nullptr);
		auto logEntry = fmt::format("{:%H:%M:%S} | Key: {} value: {} | min: {} | max: {}.", fmt::localtime(t), il2cppi_to_string(key), value, minValue, maxValue);
		abilityLog.push_front(logEntry);
		if (abilityLog.size() > 50)
			abilityLog.pop_back();

		NoCD& noCD = NoCD::GetInstance();
		// This function is calling not only for bows, so if don't put key filter it cause various game mechanic bugs.
		// For now only "_Enchanted_Time" found for bow charging, maybe there are more. Need to continue research.
		if (noCD.f_InstantBow && il2cppi_to_string(key) == "_Enchanted_Time")
		{
			value = maxValue;
			__this->fields.nextValidAbilityID = 36; // HotFix Yelan, Fishl | It's essentially a game bug. | RyujinZX#7832
		}

		CALL_ORIGIN(ActorAbilityPlugin_AddDynamicFloatWithRange_Hook, __this, key, value, minValue, maxValue, forceDoAtRemote, method);
	}

}

