#include "pch-il2cpp.h"
#include "DialogSkip.h"

#include <helpers.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
	static void InLevelCutScenePageContext_UpdateView_Hook(app::InLevelCutScenePageContext* __this, MethodInfo* method);
	static void InLevelCutScenePageContext_ClearView_Hook(app::InLevelCutScenePageContext* __this, MethodInfo* method);
	static void CriwareMediaPlayer_Update(app::CriwareMediaPlayer* __this, MethodInfo* method);

	DialogSkip::DialogSkip() : Feature(),
		NF(f_Enabled, u8"�Զ��Ի�", u8"�Զ��Ի�", false),
		NF(f_AutoSelectDialog, u8"�Զ�ѡ��Ի���", u8"�Զ��Ի�", true),
		NF(f_ExcludeImportant, u8"�ų� ��ɪ��/Tubby/�߸���", u8"�Զ��Ի�", true),
		NF(f_FastDialog, u8"���ٵĶԻ���", u8"�Զ��Ի�", false),
		NF(f_CutsceneUSM, u8"������Ϸ����", u8"�Զ��Ի�", false),
		NF(f_TimeSpeedup, u8"ʱ����ٶ�", u8"�Զ��Ի�", 5.0f)
	{
		HookManager::install(app::MoleMole_InLevelCutScenePageContext_UpdateView, InLevelCutScenePageContext_UpdateView_Hook);
		HookManager::install(app::MoleMole_InLevelCutScenePageContext_ClearView, InLevelCutScenePageContext_ClearView_Hook);
		HookManager::install(app::CriwareMediaPlayer_Update, CriwareMediaPlayer_Update);
	}

	const FeatureGUIInfo& DialogSkip::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"�Զ��Ի�", u8"����", true };
		return info;
	}

	void DialogSkip::DrawMain()
	{
		ConfigWidget(u8"����", f_Enabled, u8"�Զ������Ի���.");
		ConfigWidget(u8"�Զ�ѡ��Ի���", f_AutoSelectDialog, u8"�Զ�ѡ��Ի���ѡ��.");
		if (f_AutoSelectDialog)
		{
			ImGui::Indent();
			ConfigWidget(u8"�ų� ��ɪ��/Tubby/�߸���", f_ExcludeImportant, u8"Exclude Kath/Tubby/Wagner from auto-select.");
			ImGui::Unindent();
		}
		ConfigWidget(u8"���ٵĶԻ���", f_FastDialog, u8"Speeds up Time");
		if (f_FastDialog)
		{
			ConfigWidget(f_TimeSpeedup, 0.1f, 2.0f, 50.0f, u8"ʱ����ٱ��������ߵ�ֵ���������������ͬ�����⣬�������������Ļ���������.");
		}
		ConfigWidget(u8"������Ϸ����", f_CutsceneUSM, u8"�Զ�������Ϸ��Ӱ.");
	}

	bool DialogSkip::NeedStatusDraw() const
	{
		return f_Enabled || f_CutsceneUSM;
	}

	void DialogSkip::DrawStatus()
	{
		if (f_Enabled)
			ImGui::Text(u8"�Ի� [%s%s%s%s%s]",
				f_AutoSelectDialog ? u8"��" : u8"��",
				f_AutoSelectDialog && (f_ExcludeImportant || f_FastDialog) ? "|" : "",
				f_ExcludeImportant ? u8"�ų�" : "",
				f_ExcludeImportant && f_FastDialog ? u8"|" : u8"",
				f_FastDialog ? u8"����" : u8"����");
		if (f_CutsceneUSM)
			ImGui::Text(f_CutsceneUSM ? u8"��������" : "");
	}

	DialogSkip& DialogSkip::GetInstance()
	{
		static DialogSkip instance;
		return instance;
	}

	// Raised when dialog view updating
	// We call free click, if auto talk enabled, that means we just emulate user click
	// When appear dialog choose we create notify with dialog select first item.
	void DialogSkip::OnCutScenePageUpdate(app::InLevelCutScenePageContext* context)
	{
		if (!f_Enabled)
			return;

		auto talkDialog = context->fields._talkDialog;
		if (talkDialog == nullptr)
			return;

        if (f_FastDialog)
            app::Time_set_timeScale(f_TimeSpeedup, nullptr);
        else
            app::Time_set_timeScale(1.0f, nullptr);

		bool isImportant = false;
		if (f_ExcludeImportant)
		{
			// TODO: Add a custom filter in the future where users can
			// add their own name substrings of entities to avoid
			// speeding up dialog on.
			std::vector<std::string> impEntitiesNames = {
				u8"Djinn",
				u8"Katheryne",
				u8"Wagner"
			};
			auto dialogPartnerID = context->fields._inteeID;
			auto& manager = game::EntityManager::instance();
			auto dialogPartner = manager.entity(dialogPartnerID);
			auto dialogPartnerName = dialogPartner->name();
			for (auto impEntityName : impEntitiesNames)
			{
				if (dialogPartnerName.find(impEntityName) != -1) {
					LOG_DEBUG(u8"%s %s %d", dialogPartnerName.c_str(), impEntityName, dialogPartnerName.find(impEntityName));
					isImportant = true;
					break;
				}
			}
		}

		if (talkDialog->fields._inSelect && f_AutoSelectDialog && !isImportant)
		{
			int32_t value = 0;
			auto object = il2cpp_value_box((Il2CppClass*)*app::Int32__TypeInfo, &value);
			app::Notify notify{};
			notify.type = app::MoleMole_NotifyTypes__Enum::DialogSelectNotify;
			notify.body = (app::Object*)object;
			app::MoleMole_TalkDialogContext_OnDialogSelectItem(talkDialog, &notify, nullptr);
		}
		else if (!talkDialog->fields._inSelect)
			app::MoleMole_InLevelCutScenePageContext_OnFreeClick(context, nullptr);
	}

	static void InLevelCutScenePageContext_UpdateView_Hook(app::InLevelCutScenePageContext* __this, MethodInfo* method)
	{
		CALL_ORIGIN(InLevelCutScenePageContext_UpdateView_Hook, __this, method);

		DialogSkip& dialogSkip = DialogSkip::GetInstance();
		dialogSkip.OnCutScenePageUpdate(__this);
	}

	// Raised when exiting a dialog. We try to hackishly return to normal value.
	// Should be a better way to store the pre-dialog speed using Time_get_timeScale.
	static void InLevelCutScenePageContext_ClearView_Hook(app::InLevelCutScenePageContext* __this, MethodInfo* method)
	{
		float gameSpeed = app::Time_get_timeScale(nullptr);
		if (gameSpeed > 1.0f)
			app::Time_set_timeScale(1.0f, nullptr);
		CALL_ORIGIN(InLevelCutScenePageContext_ClearView_Hook, __this, method);
	}

	static void CriwareMediaPlayer_Update(app::CriwareMediaPlayer* __this, MethodInfo* method)
	{
		DialogSkip& dialogSkip = DialogSkip::GetInstance();
		if (dialogSkip.f_CutsceneUSM)
			app::CriwareMediaPlayer_Skip(__this, nullptr);

		return CALL_ORIGIN(CriwareMediaPlayer_Update, __this, method);
	}
}
