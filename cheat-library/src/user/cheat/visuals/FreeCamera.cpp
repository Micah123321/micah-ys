#include "pch-il2cpp.h"
#include "FreeCamera.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
	app::GameObject* freeCam = nullptr;
	app::GameObject* mainCam = nullptr;
	app::Object_1* freeCamObj = nullptr;
	app::Object_1* mainCamObj = nullptr;
	app::Transform* freeCam_Transform;
	app::Component_1* freeCam_Camera;
	app::Component_1* mainCam_Camera;
	app::Vector3 targetPosition;
	app::Vector3 smoothPosition;
	float smoothFOV;
	bool isEnabled = false;

	FreeCamera::FreeCamera() : Feature(),
		NF(f_Enabled, u8"自由相机", u8"视觉::自由相机", false),
		NF(f_Speed, u8"速度", u8"视觉::自由相机", 1.0f),
		NF(f_LookSens, u8"观看灵敏度", u8"视觉::自由相机", 1.0f),
		NF(f_RollSpeed, u8"滚动速度", u8"视觉::自由相机", 1.0f),
		NF(f_FOVSpeed, u8"FOV速度", u8"视觉::自由相机", 0.1f),
		NF(f_FOV, u8"视野", u8"视觉::自由相机", 45.0f),
		NF(f_Smoothing, u8"平滑", u8"视觉::自由相机", 1.0f),
		NF(f_Forward, u8"向前", u8"视觉::自由相机", Hotkey('W')),
		NF(f_Backward, u8"向后", u8"视觉::自由相机", Hotkey('S')),
		NF(f_Left, u8"左", u8"视觉::自由相机", Hotkey('A')),
		NF(f_Right, u8"右", u8"视觉::自由相机", Hotkey('D')),
		NF(f_Up, u8"上", u8"视觉::自由相机", Hotkey(VK_SPACE)),
		NF(f_Down, u8"下", u8"视觉::自由相机", Hotkey(VK_LCONTROL)),
		NF(f_LeftRoll, u8"左滚", u8"视觉::自由相机", Hotkey('Z')),
		NF(f_RightRoll, u8"右滚", u8"视觉::自由相机", Hotkey('X')),
		NF(f_ResetRoll, u8"重置滚动", u8"视觉::自由相机", Hotkey('C')),
		NF(f_IncFOV, u8"增加视场", u8"视觉::自由相机", Hotkey('3')),
		NF(f_DecFOV, u8"减少视场", u8"视觉::自由相机", Hotkey('1'))
	{
		events::GameUpdateEvent += MY_METHOD_HANDLER(FreeCamera::OnGameUpdate);
	}

	const FeatureGUIInfo& FreeCamera::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"自由相机", u8"视觉", true };
		return info;
	}

	void FreeCamera::DrawMain()
	{
		ConfigWidget(u8"Enable", f_Enabled);
		if (ImGui::BeginTable(u8"FreeCameraDrawTable", 1, ImGuiTableFlags_NoBordersInBody))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::BeginGroupPanel(u8"Settings");
			{
				ConfigWidget(u8"Movement Speed", f_Speed, 0.01f, 0.01f, 1000.0f);
				ConfigWidget(u8"Look Sensitivity", f_LookSens, 0.01f, 0.01f, 100.0f);
				ConfigWidget(u8"Roll Speed", f_RollSpeed, 0.01f, 0.01f, 100.0f);
				ConfigWidget(u8"FOV Speed", f_FOVSpeed, 0.01f, 0.01f, 100.0f);
				ConfigWidget(u8"Field of View", f_FOV, 0.1f, 0.01f, 200.0f);
				ConfigWidget(u8"Smoothing", f_Smoothing, 0.01f, 0.001f, 1.0f, u8"Lower = Smoother");
			}
			ImGui::EndGroupPanel();

			ImGui::BeginGroupPanel(u8"Hotkeys");
			{
				ConfigWidget(u8"Forward", f_Forward, true);
				ConfigWidget(u8"Backward", f_Backward, true);
				ConfigWidget(u8"Left", f_Left, true);
				ConfigWidget(u8"Right", f_Right, true);
				ConfigWidget(u8"Up", f_Up, true);
				ConfigWidget(u8"Down", f_Down, true);
				ConfigWidget(u8"Roll Left", f_LeftRoll, true);
				ConfigWidget(u8"Roll Right", f_RightRoll, true);
				ConfigWidget(u8"Reset Roll", f_ResetRoll, true);
				ConfigWidget(u8"Increase FOV", f_IncFOV, true);
				ConfigWidget(u8"Decrease FOV", f_DecFOV, true);
			}
			ImGui::EndGroupPanel();
			ImGui::EndTable();
		}
	}

	bool FreeCamera::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void FreeCamera::DrawStatus()
	{
		ImGui::Text(u8"Free Camera");
	}

	FreeCamera& FreeCamera::GetInstance()
	{
		static FreeCamera instance;
		return instance;
	}

	class CameraRotation
	{
	public:
		float pitch, yaw, roll;

		void InitializeFromTransform(app::Transform* t)
		{
			auto t_eulerAngles = app::Transform_get_eulerAngles(t, nullptr);
			pitch = t_eulerAngles.x;
			yaw = t_eulerAngles.y;
			roll = t_eulerAngles.z;
		}

		void LerpTowards(CameraRotation target, float rotationLerpPct)
		{
			yaw = app::Mathf_Lerp(yaw, target.yaw, rotationLerpPct, nullptr);
			pitch = app::Mathf_Lerp(pitch, target.pitch, rotationLerpPct, nullptr);
			roll = app::Mathf_Lerp(roll, target.roll, rotationLerpPct, nullptr);
		}

		void UpdateTransform(app::Transform* t)
		{
			app::Transform_set_eulerAngles(t, app::Vector3{ pitch, yaw, roll }, nullptr);
		}
	};

	auto targetRotation = CameraRotation();
	auto currentRotation = CameraRotation();

	void EnableFreeCam()
	{
		auto& settings = FreeCamera::GetInstance();
		freeCam = reinterpret_cast<app::GameObject*>(freeCamObj);

		freeCam_Transform = app::GameObject_get_transform(freeCam, nullptr);
		auto freeCam_Transform_position = app::Transform_get_position(freeCam_Transform, nullptr);

		freeCam_Camera = app::GameObject_GetComponentByName(freeCam, string_to_il2cppi(u8"Camera"), nullptr);
		mainCam_Camera = app::GameObject_GetComponentByName(mainCam, string_to_il2cppi(u8"Camera"), nullptr);

		if (isEnabled == false)
		{
			targetRotation.InitializeFromTransform(freeCam_Transform);
			currentRotation.InitializeFromTransform(freeCam_Transform);
			app::Camera_CopyFrom(reinterpret_cast<app::Camera*>(freeCam_Camera), reinterpret_cast<app::Camera*>(mainCam_Camera), nullptr);

			targetPosition = freeCam_Transform_position;
			isEnabled = true;
		}

		app::GameObject_set_active(mainCam, false, nullptr);
		app::GameObject_set_active(freeCam, true, nullptr);

		// MOVEMENT
		if (settings.f_Forward.value().IsPressed())
			targetPosition = targetPosition + app::Transform_get_forward(freeCam_Transform, nullptr) * settings.f_Speed;
		if (settings.f_Backward.value().IsPressed())
			targetPosition = targetPosition - app::Transform_get_forward(freeCam_Transform, nullptr) * settings.f_Speed;
		if (settings.f_Right.value().IsPressed())
			targetPosition = targetPosition + app::Transform_get_right(freeCam_Transform, nullptr) * settings.f_Speed;
		if (settings.f_Left.value().IsPressed())
			targetPosition = targetPosition - app::Transform_get_right(freeCam_Transform, nullptr) * settings.f_Speed;

		if (settings.f_LeftRoll.value().IsPressed())
			targetRotation.roll += settings.f_Speed;
		if (settings.f_RightRoll.value().IsPressed())
			targetRotation.roll -= settings.f_Speed;
		if (settings.f_ResetRoll.value().IsPressed())
			targetRotation.roll = 0.0f;

		if (settings.f_Up.value().IsPressed())
			targetPosition = targetPosition + app::Transform_get_up(freeCam_Transform, nullptr) * settings.f_Speed;
		if (settings.f_Down.value().IsPressed())
			targetPosition = targetPosition - app::Transform_get_up(freeCam_Transform, nullptr) * settings.f_Speed;

		if (settings.f_DecFOV.value().IsPressed())
			settings.f_FOV -= settings.f_FOVSpeed;
		if (settings.f_IncFOV.value().IsPressed())
			settings.f_FOV += settings.f_FOVSpeed;

		// Update the target rotation based on mouse input
		auto mouseX = app::Input_GetAxis(string_to_il2cppi(u8"Mouse X"), nullptr);
		auto mouseY = app::Input_GetAxis(string_to_il2cppi(u8"Mouse Y"), nullptr);
		auto mouseInput = app::Vector2{ mouseX, mouseY * -1.0f };
		targetRotation.yaw += mouseInput.x * settings.f_LookSens;
		targetRotation.pitch += mouseInput.y * settings.f_LookSens;

		// Commit the rotation changes to the transform
		currentRotation.UpdateTransform(freeCam_Transform);

		smoothPosition = app::Vector3_Lerp(freeCam_Transform_position, targetPosition, settings.f_Smoothing, nullptr);
		app::Transform_set_position(freeCam_Transform, smoothPosition, nullptr);
		smoothFOV = app::Mathf_Lerp(app::Camera_get_fieldOfView(reinterpret_cast<app::Camera*>(freeCam_Camera), nullptr), settings.f_FOV, settings.f_Smoothing, nullptr);
		app::Camera_set_fieldOfView(reinterpret_cast<app::Camera*>(freeCam_Camera), smoothFOV, nullptr);
		currentRotation.LerpTowards(targetRotation, settings.f_Smoothing);
	}

	void DisableFreeCam()
	{
		if (!isEnabled)
			return;

		if (mainCam)
		{
			app::GameObject_set_active(mainCam, true, nullptr);
			mainCam = nullptr;
		}
		if (freeCamObj)
		{
			app::Object_1_Destroy_1(freeCamObj, nullptr);
			freeCamObj = nullptr;
		}
		isEnabled = false;
	}

	void FreeCamera::OnGameUpdate()
	{
		if (f_Enabled)
		{
			if (mainCam == nullptr)
				mainCam = app::GameObject_Find(string_to_il2cppi(u8"/EntityRoot/MainCamera(Clone)"), nullptr);
			if (freeCamObj == nullptr && mainCam)
			{
				freeCamObj = app::Object_1_Instantiate_2(reinterpret_cast<app::Object_1*>(mainCam), nullptr);

				auto mainCamTransform = app::GameObject_get_transform(mainCam, nullptr);
				auto mainCamPos = app::Transform_get_position(mainCamTransform, nullptr);
				auto freeCamObjTransform = app::GameObject_get_transform(reinterpret_cast<app::GameObject*>(freeCamObj), nullptr);
				app::Transform_set_position(freeCamObjTransform, mainCamPos, nullptr);

				auto CinemachineBrain = app::GameObject_GetComponentByName(reinterpret_cast<app::GameObject*>(freeCamObj), string_to_il2cppi(u8"CinemachineBrain"), nullptr);
				auto CinemachineExternalCamera = app::GameObject_GetComponentByName(reinterpret_cast<app::GameObject*>(freeCamObj), string_to_il2cppi(u8"CinemachineExternalCamera"), nullptr);
				app::Object_1_Destroy_1(reinterpret_cast<app::Object_1*>(CinemachineBrain), nullptr);
				app::Object_1_Destroy_1(reinterpret_cast<app::Object_1*>(CinemachineExternalCamera), nullptr);

				app::GameObject_set_active(mainCam, false, nullptr);
				app::GameObject_set_active(mainCam, true, nullptr);
				app::GameObject_set_active(reinterpret_cast<app::GameObject*>(freeCamObj), false, nullptr);
			}
			if (freeCamObj)
				EnableFreeCam();
		}
		else
			DisableFreeCam();

		// Taiga#5555: There's probably be a better way of implementing this. But for now, this is just what I came up with.
		auto& manager = game::EntityManager::instance();
		auto animator = manager.avatar()->animator();
		auto rigidBody = manager.avatar()->rigidbody();
		if (animator == nullptr && rigidBody == nullptr)
			return;

		if (f_FreezeAnimation)
		{
			//auto constraints = app::Rigidbody_get_constraints(rigidBody, nullptr);
			//LOG_DEBUG("%s", magic_enum::enum_name(constraints).data());
			app::Rigidbody_set_constraints(rigidBody, app::RigidbodyConstraints__Enum::FreezePosition, nullptr);
			app::Animator_set_speed(animator, 0.f, nullptr);
		}
		else
		{
			app::Rigidbody_set_constraints(rigidBody, app::RigidbodyConstraints__Enum::FreezeRotation, nullptr);
			app::Animator_set_speed(animator, 1.f, nullptr);
		}
	}
}