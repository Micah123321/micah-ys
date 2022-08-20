#include "pch-il2cpp.h"
#include "NoClip.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat-base/render/renderer.h>

namespace cheat::feature
{
	static void HumanoidMoveFSM_LateTick_Hook(app::HumanoidMoveFSM* __this, float deltaTime, MethodInfo* method);
	app::Vector3 zero;

	NoClip::NoClip() : Feature(),
		NF(f_Enabled,			u8"穿墙",				u8"穿墙", false),
		NF(f_NoAnimation,		u8"没动画",			u8"穿墙", true),
		NF(f_UseCustomKeys,		u8"使用自定义快捷键",	u8"穿墙", false),
		NF(f_ForwardKey,		u8"向前热键",		u8"穿墙", Hotkey(ImGuiKey_W)),
		NF(f_LeftKey,			u8"向左热键",			u8"穿墙", Hotkey(ImGuiKey_A)),
		NF(f_BackKey,			u8"后 热键",			u8"穿墙", Hotkey(ImGuiKey_S)),
		NF(f_RightKey,			u8"右边 热键",			u8"穿墙", Hotkey(ImGuiKey_D)),
		NF(f_AscendKey,			u8"升高 热键",		u8"穿墙", Hotkey(ImGuiKey_Space)),
		NF(f_DescendKey,		u8"下降 热键",		u8"穿墙", Hotkey(ImGuiKey_ModShift)),
		NF(f_AltSpeedKey,		u8"替代速度 热键",		u8"穿墙", Hotkey(ImGuiKey_ModCtrl)),
		NF(f_Speed,				u8"速度",				u8"穿墙", 5.5f),
		NF(f_CameraRelative,	u8"相对于相机",	u8"穿墙", true),
		NF(f_VelocityMode,		u8"速度模式",		u8"穿墙", false),
		NF(f_FreeflightMode,	u8"Freeflight模式",		u8"穿墙", false),
		NF(f_AltSpeedEnabled,	u8"Alt速度启用",	u8"穿墙", false),
		NF(f_AltSpeed,			u8"alt速度",			u8"穿墙", 1.0f)

	{
		HookManager::install(app::MoleMole_HumanoidMoveFSM_LateTick, HumanoidMoveFSM_LateTick_Hook);

		events::GameUpdateEvent += MY_METHOD_HANDLER(NoClip::OnGameUpdate);
		events::MoveSyncEvent += MY_METHOD_HANDLER(NoClip::OnMoveSync);
	}

	const FeatureGUIInfo& NoClip::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"穿墙", u8"玩家", true };
		return info;
	}

	void NoClip::DrawMain()
	{
		ConfigWidget(u8"启用", f_Enabled, u8"启用穿墙(速度快+无碰撞).\n" \
			u8"要移动，可以使用方向键、空格键(向上)和Shift键(向下)，或者自定义你自己的键.");

		ConfigWidget(u8"无动画", f_NoAnimation, u8"禁用角色动画.");

		ConfigWidget(u8"速度", f_Speed, 0.1f, 2.0f, 100.0f,
			u8"穿墙移动速度.\n" \
			u8"不建议设置高于5.0.");

		ConfigWidget(u8"摄像机相对运动", f_CameraRelative,
			u8"相对于摄像机视角移动，而不是角色视角/方向.");

		ConfigWidget(u8"交替速度最快", f_AltSpeedEnabled,
			u8"允许使用交替速度按住左键ctrl键.\n" \
			u8"如果你想暂时比穿墙速度设置快/慢，这很有用.");

		if (f_AltSpeedEnabled) {
			ConfigWidget(u8"替代速度", f_AltSpeed, 0.1f, 2.0f, 100.0f,
				u8"交替穿墙移动速度.\n" \
				u8"不建议设置高于5.0.");

			ConfigWidget(u8"速度模式", f_VelocityMode, u8"使用速度而不是位置来移动.");
			ConfigWidget(u8"Freeflight模式", f_FreeflightMode, u8"不要删除碰撞");
		}

		ConfigWidget(u8"使用自定义键", f_UseCustomKeys, u8"Enable the Use of Custom HotKeys");

		if (f_UseCustomKeys) {
			ConfigWidget(u8"Forward HotKey", f_ForwardKey, u8"Set Forward Key");
			ConfigWidget(u8"Left HotKey", f_LeftKey, u8"Set Left Key");
			ConfigWidget(u8"Back HotKey", f_BackKey, u8"Set Back Key");
			ConfigWidget(u8"Right HotKey", f_RightKey, u8"Set Right Key");
			ConfigWidget(u8"Ascend HotKey", f_AscendKey, u8"Set Ascend Key");
			ConfigWidget(u8"Descend HotKey", f_DescendKey, u8"Set Descend Key");
			ConfigWidget(u8"Alt Speed Key", f_AltSpeedKey, u8"Set AltSpeed HotKey");
		}
	}

	bool NoClip::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void NoClip::DrawStatus()
	{
		ImGui::Text(u8"穿墙%s[%.01f%s%|%s]",
			f_AltSpeedEnabled ? "+Alt" : u8"",
			f_Speed.value(),
			f_AltSpeedEnabled ? fmt::format(u8"|{:.1f}", f_AltSpeed.value()).c_str() : u8"",
			f_CameraRelative ? u8"CR" : u8"PR");
	}

	NoClip& NoClip::GetInstance()
	{
		static NoClip instance;
		return instance;
	}

	// No clip update function.
	// We just disabling collision detect and move avatar when no clip moving keys pressed.
	void NoClip::OnGameUpdate()
	{
		static bool isApplied = false;

		auto& manager = game::EntityManager::instance();

		if (!f_Enabled && isApplied)
		{
			auto avatarEntity = manager.avatar();
			auto rigidBody = avatarEntity->rigidbody();
			if (rigidBody == nullptr)
				return;

			app::Rigidbody_set_detectCollisions(rigidBody, true, nullptr);

			isApplied = false;
		}

		if (!f_Enabled)
			return;

		isApplied = true;

		auto avatarEntity = manager.avatar();
		auto baseMove = avatarEntity->moveComponent();
		if (baseMove == nullptr)
			return;

		if (renderer::IsInputLocked())
			return;

		auto rigidBody = avatarEntity->rigidbody();
		if (rigidBody == nullptr)
			return;

		if (!f_FreeflightMode)
		{
			app::Rigidbody_set_collisionDetectionMode(rigidBody, app::CollisionDetectionMode__Enum::Continuous, nullptr);
			app::Rigidbody_set_detectCollisions(rigidBody, false, nullptr);
		}

		if (!f_VelocityMode)
			app::Rigidbody_set_velocity(rigidBody, zero, nullptr);

		auto cameraEntity = game::Entity(reinterpret_cast<app::BaseEntity*>(manager.mainCamera()));
		auto relativeEntity = f_CameraRelative ? &cameraEntity : avatarEntity;

		float speed = f_Speed.value();
		if (f_AltSpeedEnabled ? f_UseCustomKeys ? f_AltSpeedKey.value().IsPressed() : Hotkey(ImGuiKey_ModCtrl).IsPressed() : NULL)
			speed = f_AltSpeed.value();

		app::Vector3 dir = {};

		if (f_UseCustomKeys ? f_ForwardKey.value().IsPressed() : Hotkey(ImGuiKey_W).IsPressed())
			dir = dir + relativeEntity->forward();

		if (f_UseCustomKeys ? f_BackKey.value().IsPressed() : Hotkey(ImGuiKey_S).IsPressed())
			dir = dir + relativeEntity->back();

		if (f_UseCustomKeys ? f_RightKey.value().IsPressed() : Hotkey(ImGuiKey_D).IsPressed())
			dir = dir + relativeEntity->right();

		if (f_UseCustomKeys ? f_LeftKey.value().IsPressed() : Hotkey(ImGuiKey_A).IsPressed())
			dir = dir + relativeEntity->left();

		if (f_UseCustomKeys ? f_AscendKey.value().IsPressed() : Hotkey(ImGuiKey_Space).IsPressed())
			dir = dir + avatarEntity->up();

		if (f_UseCustomKeys ? f_DescendKey.value().IsPressed() : Hotkey(ImGuiKey_ModShift).IsPressed())
			dir = dir + avatarEntity->down();

		app::Vector3 prevPos = avatarEntity->relativePosition();
		if (IsVectorZero(prevPos))
			return;

		float deltaTime = app::Time_get_deltaTime(nullptr);

		app::Vector3 newPos = prevPos + dir * speed * deltaTime;
		if (!f_VelocityMode)
			avatarEntity->setRelativePosition(newPos);
		else
			app::Rigidbody_set_velocity(rigidBody, dir * speed, nullptr);
	}

	// Fixing player sync packets when no clip
	void NoClip::OnMoveSync(uint32_t entityId, app::MotionInfo* syncInfo)
	{
		static app::Vector3 prevPosition = {};
		static int64_t prevSyncTime = 0;

		if (!f_Enabled)
		{
			prevSyncTime = 0;
			return;
		}

		auto& manager = game::EntityManager::instance();
		if (manager.avatar()->runtimeID() != entityId)
			return;

		auto avatarEntity = manager.avatar();
		if (avatarEntity == nullptr)
			return;

		auto avatarPosition = avatarEntity->absolutePosition();
		auto currentTime = util::GetCurrentTimeMillisec();
		if (prevSyncTime > 0)
		{
			auto posDiff = avatarPosition - prevPosition;
			auto timeDiff = ((float)(currentTime - prevSyncTime)) / 1000;
			auto velocity = posDiff / timeDiff;

			auto speed = GetVectorMagnitude(velocity);
			if (speed > 0.1)
			{
				syncInfo->fields.motionState = (speed < 2) ? app::MotionState__Enum::MotionWalk : app::MotionState__Enum::MotionRun;

				syncInfo->fields.speed_->fields.x = velocity.x;
				syncInfo->fields.speed_->fields.y = velocity.y;
				syncInfo->fields.speed_->fields.z = velocity.z;
			}

			syncInfo->fields.pos_->fields.x = avatarPosition.x;
			syncInfo->fields.pos_->fields.y = avatarPosition.y;
			syncInfo->fields.pos_->fields.z = avatarPosition.z;
		}

		prevPosition = avatarPosition;
		prevSyncTime = currentTime;
	}

	// Disabling standard motion performing.
	// This disabling any animations, climb, jump, swim and so on.
	// But when it disabled, MoveSync sending our last position, so needs to update position in packet.
	static void HumanoidMoveFSM_LateTick_Hook(app::HumanoidMoveFSM* __this, float deltaTime, MethodInfo* method)
	{
		NoClip& noClip = NoClip::GetInstance();

		if (noClip.f_Enabled) {
			if (!noClip.f_NoAnimation) {
				__this->fields._layerMaskScene = 2;
			}
			else {
				return;
			}
		}

		CALL_ORIGIN(HumanoidMoveFSM_LateTick_Hook, __this, deltaTime, method);
	}
}

