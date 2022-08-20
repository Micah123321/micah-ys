#include "pch-il2cpp.h"
#include "AutoLoot.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/filters.h>
#include <cheat/game/Chest.h>

namespace cheat::feature
{
	static void LCSelectPickup_AddInteeBtnByID_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);
	static bool LCSelectPickup_IsInPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);
	static bool LCSelectPickup_IsOutPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);

	AutoLoot::AutoLoot() : Feature(),
		NF(f_AutoPickup, u8"自动拾取 掉落物", u8"自动捡取物品", false),
		NF(f_AutoTreasure, u8"自动拾取 宝物", u8"自动捡取物品", false),
		NF(f_UseCustomRange, u8"使用定制拾取范围", u8"自动捡取物品", false),
		NF(f_PickupFilter, u8"拾取过滤器", u8"自动捡取物品", false),
		NF(f_PickupFilter_Animals, u8"动物过滤", u8"自动捡取物品", true),
		NF(f_PickupFilter_DropItems, u8"下落物品过滤", u8"自动捡取物品", true),
		NF(f_PickupFilter_Resources, u8"资源过滤", u8"自动捡取物品", true),
		NF(f_Chest, u8"宝箱", u8"自动捡取物品", false),
		NF(f_Leyline, u8"地脉", u8"自动捡取物品", false),
		NF(f_Investigate, u8"检索点", u8"自动捡取物品", false),
		NF(f_QuestInteract, u8"探索相互", u8"自动捡取物品", false),
		NF(f_Others, u8"其他的珍宝", u8"自动捡取物品", false),
		NF(f_DelayTime, u8"延迟时间 (毫秒)", u8"自动捡取物品", 150),
		NF(f_CustomRange, u8"拾取范围", u8"自动捡取物品", 5.0f),
		toBeLootedItems(), nextLootTime(0)
	{
		// Auto loot
		HookManager::install(app::MoleMole_LCSelectPickup_AddInteeBtnByID, LCSelectPickup_AddInteeBtnByID_Hook);
		HookManager::install(app::MoleMole_LCSelectPickup_IsInPosition, LCSelectPickup_IsInPosition_Hook);
		HookManager::install(app::MoleMole_LCSelectPickup_IsOutPosition, LCSelectPickup_IsOutPosition_Hook);

		events::GameUpdateEvent += MY_METHOD_HANDLER(AutoLoot::OnGameUpdate);
	}

	const FeatureGUIInfo& AutoLoot::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"自动捡取物品", u8"世界", true };
		return info;
	}

	void AutoLoot::DrawMain()
	{
		if (ImGui::BeginTable(u8"自动战利品绘画表", 2,
			ImGuiTableFlags_NoBordersInBody))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::BeginGroupPanel(u8"自动拾取");
			{
				ConfigWidget(u8"开启", f_AutoPickup,
					u8"自动拾取掉落的物品.\n" \
					u8"注意:使用此与自定义范围和低于"
					u8"延迟时间是非常危险的.\n" \
					u8"滥用绝对应该被禁止.\n\n" \
					u8"如果使用与自定义范围，确保这是开启的");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 165, 0, 255),
					u8"看提示!");
			}
			ImGui::EndGroupPanel();

			ImGui::BeginGroupPanel(u8"自定义范围");
			{
				ConfigWidget(
					u8"开启", f_UseCustomRange,
					u8"启用自定义拾取范围.\n" \
					u8"不建议设置高值"
					u8"容易被服务器检测到.\n\n" \
					u8"如果使用自动拾取/自动宝，在"
					u8"最后开启.");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 165, 0, 255), u8"看提醒!");
				ImGui::SetNextItemWidth(100.0f);
				ConfigWidget(u8"范围 (m)", f_CustomRange, 0.1f,
					0.5f, 40.0f,
					u8"将拾取/打开范围修改为"
					u8"这个数值 (在米).");
			}
			ImGui::EndGroupPanel();

			ImGui::BeginGroupPanel(u8"拾取速度");
			{
				ImGui::SetNextItemWidth(100.0f);
				ConfigWidget(
					u8"延迟时间 (ms)", f_DelayTime, 1, 0, 1000,
					u8"战利品/打开之间的延迟(毫秒"
					u8"行动.\n" \
					u8"低于200ms的值是不安全的.\nNot"
					u8"如果没有开启自动功能，则使用.");
			}
			ImGui::EndGroupPanel();

			ImGui::TableSetColumnIndex(1);
			ImGui::BeginGroupPanel(u8"自动珍宝");
			{
				ConfigWidget(u8"启用", f_AutoTreasure,
					u8"找到打开宝箱和宝藏"
					u8"珍宝.\n" \
					u8"注意:使用此与自定义范围和低"
					u8"延迟时间是非常危险的.\n" \
					u8"滥用绝对应该被禁止.\n\n" \
					u8"如果使用自定义范围，请确保这是打开了 这个.");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 165, 0, 255),
					u8"阅读提示!");
				ImGui::Indent();
				ConfigWidget(u8"箱子", f_Chest,
					u8"普通的、珍贵的、豪华的等.");
				ConfigWidget(u8"地脉", f_Leyline, u8"莫拉/XP，地面世界/痛击boss等等.");
				ConfigWidget(u8"搜索点", f_Investigate, u8"标记为调查/搜索等.");
				ConfigWidget(u8"探索相互", f_QuestInteract, u8"有效的任务交互点.");
				ConfigWidget(u8"其他", f_Others, u8"书页、自旋晶体等.");
				ImGui::Unindent();
			}
			ImGui::EndGroupPanel();
			ImGui::EndTable();
		}

		ImGui::BeginGroupPanel(u8"拾取过滤器");
		{
			ConfigWidget(u8"开启", f_PickupFilter, u8"使传感器过滤器.\n");
			ConfigWidget(u8"动物", f_PickupFilter_Animals, u8"鱼，蜥蜴，青蛙，会飞的动物."); ImGui::SameLine();
			ConfigWidget(u8"掉落物品", f_PickupFilter_DropItems, u8"材料、矿物、工件."); ImGui::SameLine();
			ConfigWidget(u8"资源", f_PickupFilter_Resources, u8"动物和掉落物品(植物，书籍等).");
		}
		ImGui::EndGroupPanel();
	}

	bool AutoLoot::NeedStatusDraw() const
	{
		return f_AutoPickup || f_AutoTreasure || f_UseCustomRange || f_PickupFilter;
	}

	void AutoLoot::DrawStatus()
	{
		ImGui::Text(u8"自动拾取\n[%s%s%s%s%s]",
			f_AutoPickup ? u8"自动拾取" : u8"",
			f_AutoTreasure ? fmt::format(u8"{}自动珍宝", f_AutoPickup ? u8"|" : u8"").c_str() : u8"",
			f_UseCustomRange ? fmt::format(u8"{}范围{:.1f}m", f_AutoPickup || f_AutoTreasure ? u8"|" : u8"", f_CustomRange.value()).c_str() : u8"",
			f_PickupFilter ? fmt::format(u8"{}过滤", f_AutoPickup || f_AutoTreasure || f_UseCustomRange ? u8"|" : u8"").c_str() : u8"",
			f_AutoPickup || f_AutoTreasure ? fmt::format(u8"|{}ms", f_DelayTime.value()).c_str() : u8""
		);
	}

	AutoLoot& AutoLoot::GetInstance()
	{
		static AutoLoot instance;
		return instance;
	}

	bool AutoLoot::OnCreateButton(app::BaseEntity* entity)
	{
		if (!f_AutoPickup)
			return false;

		auto itemModule = GET_SINGLETON(MoleMole_ItemModule);
		if (itemModule == nullptr)
			return false;

		auto entityId = entity->fields._runtimeID_k__BackingField;
		if (f_DelayTime == 0)
		{
			app::MoleMole_ItemModule_PickItem(itemModule, entityId, nullptr);
			return true;
		}

		toBeLootedItems.push(entityId);
		return false;
	}

	void AutoLoot::OnGameUpdate()
	{
		auto currentTime = util::GetCurrentTimeMillisec();
		if (currentTime < nextLootTime)
			return;

		auto entityManager = GET_SINGLETON(MoleMole_EntityManager);
		if (entityManager == nullptr)
			return;

		// RyujinZX#6666
		if (f_AutoTreasure)
		{
			auto& manager = game::EntityManager::instance();
			for (auto& entity : manager.entities(game::filters::combined::Chests))
			{
				float range = f_UseCustomRange ? f_CustomRange : 3.5f;
				if (manager.avatar()->distance(entity) >= range)
					continue;

				auto chest = reinterpret_cast<game::Chest*>(entity);
				auto chestType = chest->itemType();

				if (!f_Investigate && chestType == game::Chest::ItemType::Investigate)
					continue;

				if (!f_QuestInteract && chestType == game::Chest::ItemType::QuestInteract)
					continue;

				if (!f_Others && (
					chestType == game::Chest::ItemType::BGM ||
					chestType == game::Chest::ItemType::BookPage
					))
					continue;

				if (!f_Leyline && chestType == game::Chest::ItemType::Flora)
					continue;

				if (chestType == game::Chest::ItemType::Chest)
				{
					if (!f_Chest)
						continue;
					auto ChestState = chest->chestState();
					if (ChestState != game::Chest::ChestState::None)
						continue;
				}

				uint32_t entityId = entity->runtimeID();
				toBeLootedItems.push(entityId);
			}
		}

		auto entityId = toBeLootedItems.pop();
		if (!entityId)
			return;

		auto itemModule = GET_SINGLETON(MoleMole_ItemModule);
		if (itemModule == nullptr)
			return;

		auto entity = app::MoleMole_EntityManager_GetValidEntity(entityManager, *entityId, nullptr);
		if (entity == nullptr)
			return;

		app::MoleMole_ItemModule_PickItem(itemModule, *entityId, nullptr);
		nextLootTime = currentTime + (int)f_DelayTime;
	}

	void AutoLoot::OnCheckIsInPosition(bool& result, app::BaseEntity* entity)
	{
		if (f_AutoPickup || f_UseCustomRange) {
			float pickupRange = f_UseCustomRange ? f_CustomRange : 3.5f;
			if (f_PickupFilter)
			{
				if (!f_PickupFilter_Animals && entity->fields.entityType == app::EntityType__Enum_1::EnvAnimal ||
					!f_PickupFilter_DropItems && entity->fields.entityType == app::EntityType__Enum_1::DropItem ||
					!f_PickupFilter_Resources && entity->fields.entityType == app::EntityType__Enum_1::GatherObject)
				{
					result = false;
					return;
				}
			}

			auto& manager = game::EntityManager::instance();
			result = manager.avatar()->distance(entity) < pickupRange;
		}
	}

	static void LCSelectPickup_AddInteeBtnByID_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		AutoLoot& autoLoot = AutoLoot::GetInstance();
		bool canceled = autoLoot.OnCreateButton(entity);
		if (!canceled)
			CALL_ORIGIN(LCSelectPickup_AddInteeBtnByID_Hook, __this, entity, method);
	}

	static bool LCSelectPickup_IsInPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		bool result = CALL_ORIGIN(LCSelectPickup_IsInPosition_Hook, __this, entity, method);

		AutoLoot& autoLoot = AutoLoot::GetInstance();
		autoLoot.OnCheckIsInPosition(result, entity);

		return result;
	}

	static bool LCSelectPickup_IsOutPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		bool result = CALL_ORIGIN(LCSelectPickup_IsOutPosition_Hook, __this, entity, method);

		AutoLoot& autoLoot = AutoLoot::GetInstance();
		autoLoot.OnCheckIsInPosition(result, entity);

		return result;
	}
}

