#include "pch-il2cpp.h"
#include "AutoChallenge.h"

#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/filters.h>

namespace cheat::feature
{

	AutoChallenge::AutoChallenge() : Feature(),
		NF(f_Enabled, u8"�Զ���ս", u8"�Զ���ս", false),
		NF(f_BombDestroy, u8"ը������", u8"�Զ���ս", false),
		NF(f_Delay, u8"�ռ��ӳ�", u8"�Զ���ս", 1000),
		NF(f_Range, u8"�ռ���Χ", u8"�Զ���ս", 20.f)
	{
		events::GameUpdateEvent += MY_METHOD_HANDLER(AutoChallenge::OnGameUpdate);
	}

	const FeatureGUIInfo& AutoChallenge::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"�Զ���ս", u8"����", true };
		return info;
	}

	void AutoChallenge::DrawMain()
	{
		ConfigWidget(u8"����", f_Enabled, u8"�Զ��ռ�ʱ����ս��Ŀ");
		ImGui::SameLine();
		ConfigWidget(u8"�ݻ�ը��", f_BombDestroy, u8"�Զ��ƻ�bombbarrel");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"�һ�û�в��Թ������ж��");
		ImGui::SetNextItemWidth(200.f);
		ConfigWidget(u8"��Χ", f_Range, 0.1f, 0.f, 300.f, u8"�ռ���Χ.");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.f);
		ConfigWidget(u8"�ӳ�", f_Delay, 1, 0, 2000, u8"�ռ��ӳ�.");
	}

	bool AutoChallenge::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void AutoChallenge::DrawStatus()
	{
		ImGui::Text(u8"��ս [%.01fm]", f_Range.value());
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