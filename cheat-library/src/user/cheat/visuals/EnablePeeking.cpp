#include "pch-il2cpp.h"
#include "EnablePeeking.h"

#include <helpers.h>

namespace cheat::feature
{
	static void MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook(app::MoleMole_VCBaseSetDitherValue* __this, float value, MethodInfo* method);



	EnablePeeking::EnablePeeking() : Feature(),
		NF(f_Enabled, u8"����͵��", u8"�Ӿ�::����͵��", false)

	{
		HookManager::install(app::MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue, MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook);
	}

	const FeatureGUIInfo& EnablePeeking::GetGUIInfo() const
	{

		static const FeatureGUIInfo info{ u8"����͵��", u8"�Ӿ�", false };

		return info;
	}

	void EnablePeeking::DrawMain()
	{
		ConfigWidget(f_Enabled, u8"͵��С����");
	}

	bool EnablePeeking::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void EnablePeeking::DrawStatus()
	{

		ImGui::Text(u8"����͵��");
	}

	EnablePeeking& EnablePeeking::GetInstance()
	{
		static EnablePeeking instance;
		return instance;
	}

	static void MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook(app::MoleMole_VCBaseSetDitherValue* __this, float value, MethodInfo* method)
	{
		EnablePeeking& EnablePeeking = EnablePeeking::GetInstance();
		if (EnablePeeking.f_Enabled)
			value = 1;

		CALL_ORIGIN(MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook, __this, value, method);
	}
}

