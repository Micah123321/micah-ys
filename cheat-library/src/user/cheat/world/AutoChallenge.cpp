#include "pch-il2cpp.h"
#include "AutoChallenge.h"

#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{

	AutoChallenge::AutoChallenge() : Feature(),
		NF(f_Enabled, u8"自动挑战", u8"自动挑战", false),
		NF(f_BombDestroy, u8"炸弹销毁", u8"自动挑战", false),
		NF(f_Delay, u8"收集延迟", u8"自动挑战", 1000),
		NF(f_Range, u8"收集范围", u8"自动挑战", 20.f)
	{
		events::GameUpdateEvent += MY_METHOD_HANDLER(AutoChallenge::OnGameUpdate);
	}

	const FeatureGUIInfo& AutoChallenge::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"自动挑战", u8"世界", true };
		return info;
	}

	void AutoChallenge::DrawMain()
	{
		ConfigWidget(u8"启用", f_Enabled, u8"自动收集时间挑战项目");
		ImGui::SameLine();
		ConfigWidget(u8"摧毁炸弹", f_BombDestroy, u8"自动破坏bombbarrel");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"我还没有测试过风险有多高");
		ImGui::SetNextItemWidth(200.f);
		ConfigWidget(u8"范围", f_Range, 0.1f, 0.f, 300.f, u8"收集范围.");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.f);
		ConfigWidget(u8"延迟", f_Delay, 1, 0, 2000, u8"收集延迟.");
	}

	bool AutoChallenge::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void AutoChallenge::DrawStatus()
	{
		ImGui::Text(u8"挑战 [%.01fm]", f_Range.value());
	}

	AutoChallenge& AutoChallenge::GetInstance()
	{
		static AutoChallenge instance;
		return instance;
	}

	void AutoChallenge::OnGameUpdate()
	{
		static uint64_t lastTime = 0;
		auto timestamp = app::MoleMole_TimeUtil_get_NowTimeStamp(nullptr);

		if (!f_Enabled || lastTime + f_Delay > timestamp)
			return;

		auto& entityManager = game::EntityManager::instance();
		auto avatarEntity = entityManager.avatar();

		for (auto& entity : entityManager.entities(game::filters::puzzle::TimeTrialChallengeCollection))
		{
			if (avatarEntity->distance(entity) > f_Range)
				continue;

			auto combat = entity->combat();
			if (combat != nullptr)
			{
				auto combatProp = combat->fields._combatProperty_k__BackingField;
				auto maxHP = app::MoleMole_SafeFloat_get_Value(combatProp->fields.maxHP, nullptr);
				// so many entities named u8"SkillObj_EmptyGadget", but the collection's hp is 99999.f
				if (maxHP > 99998 && maxHP < 99999.9)
				{
					entity->setRelativePosition(avatarEntity->relativePosition());
				}
			}
		}
	}
}