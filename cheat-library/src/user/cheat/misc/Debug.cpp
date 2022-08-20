#include "pch-il2cpp.h"
#include "Debug.h"

#include <misc/cpp/imgui_stdlib.h>
#include <filesystem>
#include <fstream>

#include <cheat/events.h>
#include <cheat/teleport/MapTeleport.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>
#include <cheat/game/filters.h>
#include <cheat/esp/ESPRender.h>
#include <cheat/game/CacheFilterExecutor.h>
#include <cheat-base/render/renderer.h>
#include <helpers.h>
#include <imgui_internal.h>

// This module is for debug purpose, and... well.. it's shit coded ^)
namespace cheat::feature
{

	static bool ActorAbilityPlugin_OnEvent_Hook(void* __this, app::BaseEvent* e, MethodInfo* method);
	void OnGameUpdate();
	static bool csvFriendly = true;
	static bool includeHeaders = true;

	Debug::Debug() : Feature()
	{
		events::GameUpdateEvent += FUNCTION_HANDLER(OnGameUpdate);
		HookManager::install(app::MoleMole_ActorAbilityPlugin_OnEvent, ActorAbilityPlugin_OnEvent_Hook);
		// HookManager::install(app::MoleMole_LuaShellManager_ReportLuaShellResult, LuaShellManager_ReportLuaShellResult_Hook);
		// HookManager::install(app::MoleMole_LuaShellManager_DoString, LuaShellManager_DoString_Hook);
		// HookManager::install(app::LuaEnv_DoString, LuaEnv_DoString_Hook);
		// HookManager::install(app::Lua_xlua_pushasciistring, Lua_xlua_pushasciistring_Hook);

		// HookManager::install(app::GameLogin_SendInfo_2, SendInfo_Hook);
		// LOG_DEBUG(u8"Hooked GameLogin::SendGameInfo. Origin at 0x%p", HookManager::getOrigin(SendInfo_Hook));
	}

	const FeatureGUIInfo& Debug::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"调试信息", u8"调试", false };
		return info;
	}

	Debug& Debug::GetInstance()
	{
		static Debug instance;
		return instance;
	}


	// Raise when player start game log in (after press a door)
	// Contains information about player system and game integrity
	static void SendInfo_Hook(app::MoleMole_NetworkManager* __this, app::GKOJAICIOPA* info, MethodInfo* method)
	{
		LOG_TRACE(u8"Game sending game info to server.");
		LOG_TRACE(u8"Content: u8");

#define printString(i) if (info->fields.string_ ## i > (void *)1 && info->fields.string_ ## i ##->fields.length > 0)\
    LOG_TRACE(u8"\tfield#%d: %s", i ,il2cppi_to_string(info->fields.string_ ## i).c_str());

		printString(1);
		printString(2);
		printString(3);
		printString(4);
		printString(5);
		printString(6);
		printString(7);
		printString(8);
		printString(9);
		printString(10);
		printString(11);
		printString(12);
		printString(13);
		printString(14);
		printString(15);
		printString(16);

#undef printString

		CALL_ORIGIN(SendInfo_Hook, __this, info, method);
	}

	static void Lua_xlua_pushasciistring_Hook(void* __this, void* L, app::String* str, MethodInfo* method)
	{
		LOG_DEBUG(u8"Pushed string: %s", il2cppi_to_string(str).c_str());
		CALL_ORIGIN(Lua_xlua_pushasciistring_Hook, __this, L, str, method);
	}

	static int checkCount = 0;
	static void* LuaEnv_DoString_Hook(void* __this, app::Byte__Array* chunk, app::String* chunkName, void* env, MethodInfo* method)
	{
		if (checkCount > 0)
		{
			LOG_DEBUG(u8"After size %d; name: %s", chunk->bounds == nullptr ? chunk->max_length : chunk->bounds->length, il2cppi_to_string(chunkName).c_str());
			checkCount--;
		}
		return CALL_ORIGIN(LuaEnv_DoString_Hook, __this, chunk, chunkName, env, method);
	}

	static void LuaShellManager_DoString_Hook(void* __this, app::Byte__Array* byteArray, MethodInfo* method)
	{
		LOG_DEBUG(u8"Size %d", byteArray->bounds == nullptr ? byteArray->max_length : byteArray->bounds->length);
		checkCount = 10;
		CALL_ORIGIN(LuaShellManager_DoString_Hook, __this, byteArray, method);
	}

	static void LuaShellManager_ReportLuaShellResult_Hook(void* __this, app::String* type, app::String* value, MethodInfo* method)
	{
		std::cout << u8"Type: u8" << il2cppi_to_string(type) << std::endl;
		std::cout << u8"Value: u8" << il2cppi_to_string(value) << std::endl;
		CALL_ORIGIN(LuaShellManager_ReportLuaShellResult_Hook, __this, type, value, method);
	}

	static bool ActorAbilityPlugin_OnEvent_Hook(void* __this, app::BaseEvent* e, MethodInfo* method)
	{
		// LOG_DEBUG(u8"Fire event: %s, targetID %u", magic_enum::enum_name(e->fields.eventID).data(), e->fields.targetID);
		return CALL_ORIGIN(ActorAbilityPlugin_OnEvent_Hook, __this, e, method);
	}

	static void DrawWaypoints(UniDict<uint32_t, UniDict<uint32_t, app::MapModule_ScenePointData>*>* waypointsGrops)
	{
		if (waypointsGrops == nullptr)
		{
			ImGui::Text(u8"路径点数据不存在.");
			return;
		}

		auto singleton = GET_SINGLETON(MoleMole_MapModule);

		for (const auto& [sceneId, waypoints] : waypointsGrops->pairs())
		{
			if (ImGui::TreeNode((u8"WTD" + std::to_string(sceneId)).c_str(), u8"路径组id %d", sceneId))
			{
				for (const auto& [waypointId, waypoint] : waypoints->pairs())
				{
					if (ImGui::TreeNode((u8"WD" + std::to_string(waypointId)).c_str(), u8"路标id %d", waypointId))
					{
						ImGui::Text(u8"组限制: %s", waypoint.isGroupLimit ? u8"true" : u8"false");
						ImGui::Text(u8"未锁定: %s", waypoint.isUnlocked ? u8"true" : u8"false");
						ImGui::Text(u8"级别: %u", waypoint.level);
						ImGui::Text(u8"实体id: %u", waypoint.entityId);
						ImGui::Text(u8"ModelHiden: %s", waypoint.isModelHidden ? u8"true" : u8"false");

						if (waypoint.config != nullptr)
						{
							auto location = waypoint.config->fields;
							ImGui::Text(u8"Waypoint type: %s", magic_enum::enum_name(location._type).data());
							ImGui::Text(u8"Trans position: %s", il2cppi_to_string(location._tranPos).c_str());
							ImGui::Text(u8"Object position: %s", il2cppi_to_string(location._pos).c_str());
							ImGui::Text(u8"_unlocked: %s", location._unlocked ? u8"true" : u8"false");
							ImGui::Text(u8"_groupLimit: %s", location._groupLimit ? u8"true" : u8"false");
							uint16_t areaId = app::MoleMole_SimpleSafeUInt16_get_Value(location.areaIdRawNum, nullptr);
							ImGui::Text(u8"areaId: %u", areaId);
							ImGui::Text(u8"areaUnlocked: %s", app::MoleMole_MapModule_IsAreaUnlock(singleton, sceneId, areaId, nullptr) ? u8"true" : u8"false");
							ImGui::Text(u8"gadgetIdRawNum: %u", location.gadgetIdRawNum);
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
		}
	}

	void DrawManagerData()
	{
		auto singleton = GET_SINGLETON(MoleMole_MapModule);
		if (singleton == nullptr)
		{
			ImGui::Text(u8"管理器没有初始化.");
			return;
		}

		if (ImGui::TreeNode(u8"路径点"))
		{
			auto waypoints = TO_UNI_DICT(singleton->fields._scenePointDics, uint32_t, UniDict<uint32_t COMMA app::MapModule_ScenePointData>*);
			DrawWaypoints(waypoints);
			ImGui::TreePop();
		}
	}

	void DrawEntity(game::Entity* entity)
	{
		if (entity == nullptr)
		{
			ImGui::Text(u8"Entity doesn't exist.");
			return;
		}
		ImGui::Text(u8"Entity type: %s", magic_enum::enum_name(entity->type()).data());
		ImGui::Text(u8"Entity name: %s", entity->name().c_str());
	}

    void CopyEntityDetailsToClipboard(std::vector<game::Entity*> entities)
    {
        std::string entitiesDetails = "";
        if (csvFriendly && includeHeaders)
            entitiesDetails.append("Entity,RuntimeID,Name,PosX,PosY,PosZ\n");
        for (auto entity : entities) {
            auto entityPos = entity->absolutePosition();
            std::string baseString = csvFriendly ? "{},{},{},{},{},{}" : "{} {} {} x={} y={} z={}";
            auto entityDetails = fmt::format(
                fmt::runtime(baseString),
                fmt::ptr(entity),
                entity->runtimeID(),
                entity->name().c_str(),
                entityPos.x, entityPos.y, entityPos.z
            );
            entitiesDetails.append(entityDetails);
            entitiesDetails.append("\n");
        }
        ImGui::SetClipboardText(entitiesDetails.c_str());
    }

    void CopyEntityDetailsToClipboard(game::Entity* entity)
    {
        auto entityPos = entity->absolutePosition();
        std::string headerString = "Entity,RuntimeID,Name,PosX,PosY,PosZ\n";
        std::string baseString = csvFriendly ? "{},{},{},{},{},{}" : "{} {} {} x={} y={} z={}";
        if (csvFriendly && includeHeaders)
            baseString = headerString.append(baseString);
        auto entityDetails = fmt::format(
            fmt::runtime(baseString),
            fmt::ptr(entity),
            entity->runtimeID(),
            entity->name().c_str(),
            entityPos.x, entityPos.y, entityPos.z
        );
        ImGui::SetClipboardText(entityDetails.c_str());
    }

	void DrawCombatDetails(game::Entity* entity)
	{
		auto combat = entity->combat();
		if (combat != nullptr) {
			auto combatProp = combat->fields._combatProperty_k__BackingField;
			auto maxHP = app::MoleMole_SafeFloat_get_Value(combatProp->fields.maxHP, nullptr);
			auto HP = app::MoleMole_SafeFloat_get_Value(combatProp->fields.HP, nullptr);
			auto isLockHp = combatProp->fields.islockHP == nullptr || app::MoleMole_FixedBoolStack_get_value(combatProp->fields.islockHP, nullptr);
			auto isInvincible = combatProp->fields.isInvincible == nullptr || app::MoleMole_FixedBoolStack_get_value(combatProp->fields.isInvincible, nullptr);
			ImGui::BeginTooltip();
			ImGui::Text(u8"Combat: %s", combat == nullptr ? u8"No" : u8"Yes");
			ImGui::Text(u8"Combat Prop: %s", combatProp == nullptr ? u8"No" : u8"Yes");
			ImGui::Text(u8"HP Curr/Max: %.01f/%.01f", HP, maxHP);
			ImGui::Text(u8"Locked HP: %s", isLockHp ? u8"Yes" : u8"No");
			ImGui::Text(u8"Invincible: %s", isInvincible ? u8"Yes" : u8"No");
			ImGui::EndTooltip();
		}
	}

	void DrawEntityActionButtons(game::Entity* entity, bool& csvFriendly)
	{
		auto& manager = game::EntityManager::instance();

		if (ImGui::SmallButton(u8"T"))
		{
			auto& mapTeleport = MapTeleport::GetInstance();
			mapTeleport.TeleportTo(entity->absolutePosition());
		};
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(u8"Teleport");

		ImGui::SameLine();
		if (ImGui::SmallButton(u8"S"))
			entity->setRelativePosition(manager.avatar()->relativePosition());
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(u8"Summon");

		ImGui::SameLine();
		if (ImGui::SmallButton(u8"B"))
			entity->setRelativePosition({ 0, 0, 0 });
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(u8"Banish");

		ImGui::SameLine();
		if (ImGui::SmallButton(u8"C"))
			CopyEntityDetailsToClipboard(entity);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(u8"Copy Details");
	}

	std::vector<game::Entity*> SortEntities(std::vector<game::Entity*> entities, Debug::EntitySortCondition condition)
	{
		switch (condition) {
		case Debug::EntitySortCondition::RuntimeID: {
			std::sort(entities.begin(),
				entities.end(),
				[](game::Entity* e1, game::Entity* e2) {
					auto s1 = e1->runtimeID();
					auto s2 = e2->runtimeID();
					return s1 < s2;
				});
			break;
		}
		case Debug::EntitySortCondition::Name: {
			std::sort(entities.begin(),
				entities.end(),
				[](game::Entity* e1, game::Entity* e2) {
					auto s1 = e1->name().c_str();
					auto s2 = e2->name().c_str();
					return s1 < s2;
				});
			break;
		}
		case Debug::EntitySortCondition::Distance: {
			std::sort(entities.begin(),
				entities.end(),
				[](game::Entity* e1, game::Entity* e2) {
					auto& manager = game::EntityManager::instance();
					return manager.avatar()->distance(e1) < manager.avatar()->distance(e2);
				});
			break;
		}
		default:
			break;
		}
		return entities;
	}

	void TeleportByCondition(std::vector<game::Entity*> entities, Debug::TeleportCondition condition)
	{
		auto& manager = game::EntityManager::instance();
		auto& mapTeleport = MapTeleport::GetInstance();

		// Opted for this instead of min_/max_element to guarantee no voodoo magic happens.
		// We'll go for min_/max_element later on if we want to implement weird sorts like
		// lowest HP/highest HP/etc. Even then, that will be in SortEntities, not here.
		// Like so: SortEntities(entities, Debug::EntitySortCondition::Health);
		auto sortedEntities = SortEntities(entities, Debug::EntitySortCondition::Distance);
		// Always have a default target!
		auto target = sortedEntities.front();

		// Keeping this as a switch statement instead of ternary. We don't know yet how
		// many cases we want to keep supporting. Ternary is cleaner, but not a big
		// performance hit if we keep a switch statement.
		switch (condition) {
		case Debug::TeleportCondition::Closest: {
			// We've already selected this.
			break;
		}
		case Debug::TeleportCondition::Farthest: {
			target = sortedEntities.back();
			break;
		}
		}

		// Separating this logic to keep it clean and consistent.
		if (target != nullptr)
		{
			auto targetDist = manager.avatar()->distance(target);
			if (targetDist > 30.0f)
				mapTeleport.TeleportTo(target->absolutePosition());
			else manager.avatar()->setRelativePosition(target->relativePosition());
		}
	}

	void SummonEntities(game::Entity* entity)
	{
		auto& manager = game::EntityManager::instance();
		entity->setRelativePosition(manager.avatar()->relativePosition());
	}

	void SummonEntities(std::vector<game::Entity*> entities)
	{
		for (auto entity : entities)
			SummonEntities(entity);
	}

	void BanishEntities(game::Entity* entity)
	{
		entity->setRelativePosition({ 0, 0, 0 });
	}

	void BanishEntities(std::vector<game::Entity*> entities)
	{
		for (auto entity : entities)
			BanishEntities(entity);
	}

	void DrawEntityGroupActionButtons(std::vector<game::Entity*> entities, bool& csvFriendly, bool& includeHeaders)
	{
		auto& manager = game::EntityManager::instance();

		if (ImGui::Button(u8"Teleport: Closest"))
			TeleportByCondition(entities, Debug::TeleportCondition::Closest);

		ImGui::SameLine();
		if (ImGui::Button(u8"Teleport: Farthest"))
			TeleportByCondition(entities, Debug::TeleportCondition::Farthest);

		ImGui::SameLine();
		if (ImGui::Button(u8"Summon All"))
			SummonEntities(entities);

		ImGui::SameLine();
		if (ImGui::Button(u8"Banish All"))
			BanishEntities(entities);

		ImGui::SameLine();
		if (ImGui::Button(u8"Copy All Details"))
			CopyEntityDetailsToClipboard(entities);

		ImGui::SameLine();
		ImGui::Checkbox(u8"CSV Friendly", &csvFriendly);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(u8"Uses comma separation and removes xyz from pos on copy.");

		if (csvFriendly) {
			ImGui::SameLine();
			ImGui::Checkbox(u8"Include Headers", &includeHeaders);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(u8"Includes headers when copying.");
		}
	}

	void DrawEntitiesTable(std::vector<game::Entity*> entities)
	{
		auto& manager = game::EntityManager::instance();
		auto clipSize = min(entities.size(), 15) + 1; // Number of rows in table as initial view. Past this is scrollbar territory.

		static ImGuiTableFlags flags =
			ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable // | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
			| ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
			| ImGuiTableFlags_ScrollY;
		if (ImGui::BeginTable(u8"EntityTable", 8, flags, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * clipSize), 0.0f))
		{
			ImGui::TableSetupColumn(u8"Commands", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0, 0);
			ImGui::TableSetupColumn(u8"ID", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
			ImGui::TableSetupColumn(u8"RuntimeID", ImGuiTableColumnFlags_WidthFixed, 0.0f, 2);
			ImGui::TableSetupColumn(u8"Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, 3);
			ImGui::TableSetupColumn(u8"Distance", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_WidthFixed, 0.0, 4);
			ImGui::TableSetupColumn(u8"Pos.x", ImGuiTableColumnFlags_WidthFixed, 0.0, 5);
			ImGui::TableSetupColumn(u8"Pos.y", ImGuiTableColumnFlags_WidthFixed, 0.0, 6);
			ImGui::TableSetupColumn(u8"Pos.z", ImGuiTableColumnFlags_WidthFixed, 0.0, 7);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(entities.size()));
			while (clipper.Step())
				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
				{
					auto entity = entities[row_n];
					auto entityPos = entity->absolutePosition();

					ImGui::PushID(entity->runtimeID());
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					DrawEntityActionButtons(entity, csvFriendly);
					ImGui::TableNextColumn();

					ImGui::Text(u8"0x%p", entity);
					if (ImGui::IsItemHovered())
						DrawCombatDetails(entity);
					ImGui::TableNextColumn();

					ImGui::Text(u8"%u", entity->runtimeID());
					ImGui::TableNextColumn();

					ImGui::Text(u8"%s", entity->name().c_str());
					ImGui::TableNextColumn();

					ImGui::Text(u8"%.3fm", manager.avatar()->distance(entity));
					ImGui::TableNextColumn();

					ImGui::Text(u8"%.04f", entityPos.x);
					ImGui::TableNextColumn();

					ImGui::Text(u8"%.04f", entityPos.y);
					ImGui::TableNextColumn();

					ImGui::Text(u8"%.04f", entityPos.z);

					ImGui::PopID();
				}
			ImGui::EndTable();
		}
	}

	static void DrawEntitiesData()
	{
		static bool typeFilters[0x63] = {};
		static bool typeFiltersInitialized = false;

		if (!typeFiltersInitialized) {
			std::fill_n(typeFilters, 0x63, true);
			typeFiltersInitialized = true;
		}

		static bool useObjectNameFilter = false;
		static char objectNameFilter[128] = {};
		static float radius = 0.0f;
		static bool useRadius = false;
		static bool groupByType = true;
		static int typeFiltersColCount = 5;

		static bool checkOnlyShells = false;
		static bool showEmptyTypes = false;
		static Debug::EntitySortCondition sortCondition = Debug::EntitySortCondition::Distance;
		static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll |
			ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_TabListPopupButton;

		auto& manager = game::EntityManager::instance();
		auto entities = manager.entities();
		auto entries = magic_enum::enum_entries<app::EntityType__Enum_1>();

		std::vector<std::pair<app::EntityType__Enum_1, std::string_view>> sortedEntries;
		sortedEntries.insert(sortedEntries.begin(), std::begin(entries), std::end(entries));
		std::sort(sortedEntries.begin(), sortedEntries.end(), [](auto a1, auto a2) {
			return a1.second < a2.second;
			});

		ImGuiContext& g = *GImGui;
		ImGuiIO& io = g.IO;

		ImGui::Text(u8"Entity Count %d", entities.size());

		ImGui::Checkbox(u8"## Enable Object Name Filter", &useObjectNameFilter); ImGui::SameLine();
		if (!useObjectNameFilter)
			ImGui::BeginDisabled();
		ImGui::InputText(u8"Entity Name Filter", objectNameFilter, 128);
		if (!useObjectNameFilter)
			ImGui::EndDisabled();


		ImGui::Checkbox(u8"Filter by Radius", &useRadius);
		if (!useRadius)
			ImGui::BeginDisabled();

		ImGui::SameLine();
		ImGui::PushItemWidth(200.0);
		ImGui::SliderFloat(u8"Radius", &radius, 0.0f, 100.0f);
		ImGui::PopItemWidth();
		if (!useRadius)
			ImGui::EndDisabled();

		if (ImGui::BeginTabBar(u8"EntityManagerTabBar", tab_bar_flags))
		{
			if (ImGui::BeginTabItem(u8"Type Filter"))
			{
				if (ImGui::Button(u8"Select All"))
					std::fill_n(typeFilters, 0x63, true);
				ImGui::SameLine();

				if (ImGui::Button(u8"Deselect All"))
					std::fill_n(typeFilters, 0x63, false);
				ImGui::SameLine();

				ImGui::PushItemWidth(100.0);
				ImGui::SliderInt(u8"No. of Columns", &typeFiltersColCount, 2, 5);
				ImGui::PopItemWidth();

				if (ImGui::BeginTable(u8"Type Filter Table", typeFiltersColCount, ImGuiTableFlags_NoBordersInBody))
				{
					for (const auto& [value, name] : sortedEntries)
					{
						ImGui::TableNextColumn();
						ImGui::Checkbox(name.data(), &typeFilters[(int)value]);
					}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(u8"Entity List"))
			{
				// Checkbox: Group by Type.
				ImGui::Checkbox(u8"Group by Type", &groupByType);
				ImGui::SameLine();

				if (groupByType) {
					ImGui::Checkbox(u8"Show Empty Types", &showEmptyTypes);
					ImGui::SameLine();
				}

				ImGui::Checkbox(u8"Show Only Oculi", &checkOnlyShells);
				ImGui::SameLine();

				bool sortConditionChanged = ComboEnum(u8"Sort Mode", &sortCondition);

				if (entities.size() > 0) {
					if (groupByType) {
						if (ImGui::BeginTabBar(u8"EntityListTabBar", tab_bar_flags))
						{
							for (const auto& [currentType, typeName] : sortedEntries) {
								if (!typeFilters[int(currentType)])
									continue;

								auto filteredEntities = manager.entities(game::SimpleFilter(currentType));
								if (!showEmptyTypes && filteredEntities.size() == 0)
									continue;

								std::vector<cheat::game::Entity*> validEntities;
								for (const auto& entity : filteredEntities)
								{
									if (entity == nullptr)
										continue;

									if (entity->type() != currentType)
										continue;

									if (checkOnlyShells && !game::filters::combined::Oculies.IsValid(entity))
										continue;

									if (useObjectNameFilter && entity->name().find(objectNameFilter) == -1)
										continue;

									if (useRadius)
									{
										auto dist = manager.avatar()->distance(entity);
										if (dist > radius)
											continue;
									}

									validEntities.push_back(entity);
								}

								if (validEntities.size() == 0 && !showEmptyTypes)
									continue;


								if (ImGui::BeginTabItem(typeName.data()))
								{
									auto sortedEntities = SortEntities(validEntities, sortCondition);
									DrawEntityGroupActionButtons(sortedEntities, csvFriendly, includeHeaders);
									DrawEntitiesTable(sortedEntities);
									ImGui::EndTabItem();
								}
							}
						}
						ImGui::EndTabBar();
					}
					else {
						std::vector<cheat::game::Entity*> validEntities;
						for (const auto& entity : entities)
						{
							if (entity == nullptr)
								continue;

							if (!typeFilters[int(entity->type())])
								continue;

							if (checkOnlyShells && !game::filters::combined::Oculies.IsValid(entity))
								continue;

							if (useObjectNameFilter && entity->name().find(objectNameFilter) == -1)
								continue;

							if (useRadius)
							{
								auto dist = manager.avatar()->distance(entity);
								if (dist > radius)
									continue;
							}

							validEntities.push_back(entity);
						}

						auto sortedEntities = SortEntities(validEntities, sortCondition);
						DrawEntityGroupActionButtons(sortedEntities, csvFriendly, includeHeaders);
						DrawEntitiesTable(sortedEntities);
						ImGui::TreePop();
					}
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

#define DRAW_UINT(owner, fieldName) ImGui::Text(u8"%s: %u", #fieldName, owner##->fields.##fieldName );
#define DRAW_FLOAT(owner, fieldName) ImGui::Text(u8"%s: %f", #fieldName, owner##->fields.##fieldName );
#define DRAW_BOOL(owner, fieldName) ImGui::Text(u8"%s: %s", #fieldName, owner##->fields.##fieldName ? u8"true" : u8"false");

	static void DrawBaseInteraction(app::BaseInterAction* inter)
	{
		ImGui::Text(u8"_type: %s", magic_enum::enum_name(inter->fields._type).data());
		DRAW_UINT(inter, _mainQuestId);
		DRAW_BOOL(inter, _isFromExternal);
		DRAW_BOOL(inter, _isStarted);
		DRAW_BOOL(inter, _isFinished);
		auto cfg = inter->fields._cfg;
		if (cfg == nullptr)
			return;

		ImGui::Text(u8"Config: u8");
		ImGui::Text(u8"_type: %s", magic_enum::enum_name(cfg->fields._type).data());
		DRAW_FLOAT(cfg, _delayTime);
		DRAW_FLOAT(cfg, _duration);
		DRAW_FLOAT(cfg, _checkNextImmediately);
	}

	static void DrawInteractionManagerInfo()
	{
		auto interactionManager = GET_SINGLETON(InteractionManager);
		if (interactionManager == nullptr)
		{
			ImGui::Text(u8"Manager not loaded.");
			return;
		}

		DRAW_UINT(interactionManager, _keyInterCnt);
		DRAW_FLOAT(interactionManager, _endFadeInTime);
		DRAW_FLOAT(interactionManager, _endFadeOutTime);
		DRAW_BOOL(interactionManager, _hasKeyPre);
		DRAW_BOOL(interactionManager, _havEndFade);
		DRAW_BOOL(interactionManager, _inEndFade);
		DRAW_BOOL(interactionManager, _inStartFade);
		DRAW_BOOL(interactionManager, _talkLoading);
		DRAW_BOOL(interactionManager, _voiceLoading);
		DRAW_BOOL(interactionManager, _isLockGameTime);
		DRAW_BOOL(interactionManager, _isInteeReadyChecking);
		DRAW_BOOL(interactionManager, _isDelayClear);
		DRAW_BOOL(interactionManager, _isFromPerformConfig);
		DRAW_BOOL(interactionManager, _edtTalkWaiting);
		DRAW_BOOL(interactionManager, _isManulAttackMode);
		DRAW_BOOL(interactionManager, _canShowAvatarEffectWhenTalkStart);


		auto keyList = TO_UNI_LINK_LIST(interactionManager->fields._keyInterList, app::InterActionGrp*);
		if (keyList != nullptr && ImGui::TreeNode(u8"KeyList"))
		{
			auto reminder = keyList->count;
			auto current = keyList->first;
			while (reminder > 0 && current != nullptr)
			{
				auto item = current->item;
				if (ImGui::TreeNode(item, u8"Key item: gid %d", item->fields.groupId))
				{
					DRAW_UINT(item, groupId);
					DRAW_UINT(item, nextGroupId);
					DRAW_BOOL(item, isKeyList);
					DRAW_BOOL(item, _isStarted);

					if (item->fields._interActionList != nullptr && ImGui::TreeNode(u8"Interactions"))
					{
						auto interactions = TO_UNI_LIST(item->fields._interActionList, app::BaseInterAction*);
						for (auto& interaction : *interactions)
						{
							if (interaction == nullptr)
								continue;

							if (ImGui::TreeNode(interaction, u8"Base interaction"))
							{
								DrawBaseInteraction(interaction);
								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				current = current->forward;
				reminder--;
			}
			ImGui::TreePop();

		}
	}

#undef DRAW_UINT
#undef DRAW_FLOAT
#undef DRAW_BOOL

	void DrawPositionInfo()
	{
		auto avatarPos = app::ActorUtils_GetAvatarPos(nullptr);
		ImGui::Text(u8"Avatar position: %s", il2cppi_to_string(avatarPos).c_str());

		auto relativePos = app::WorldShiftManager_GetRelativePosition(avatarPos, nullptr);
		ImGui::Text(u8"Relative position: %s", il2cppi_to_string(relativePos).c_str());

		auto levelPos = app::Miscs_GenLevelPos_1(avatarPos, nullptr);
		ImGui::Text(u8"Level position: %s", il2cppi_to_string(levelPos).c_str());


		static app::Vector3 teleportPos = {};
		ImGui::InputFloat3(u8"Teleport position", reinterpret_cast<float*>(&teleportPos));

		auto& teleport = MapTeleport::GetInstance();
		if (ImGui::Button(u8"Map teleport"))
			teleport.TeleportTo(app::Vector2{ teleportPos.x, teleportPos.y });

		ImGui::SameLine();

		if (ImGui::Button(u8"World teleport"))
			teleport.TeleportTo(teleportPos);

		if (ImGui::TreeNode(u8"Ground pos info"))
		{
			auto groundNormal = app::Miscs_CalcCurrentGroundNorm(avatarPos, nullptr);
			ImGui::Text(u8"Ground normal: %s", il2cppi_to_string(groundNormal).c_str());

			static app::Vector3 pos{};
			static bool fixedToPos;
			ImGui::Checkbox(u8"## Fixed to position", &fixedToPos); ImGui::SameLine();
			if (fixedToPos) {
				pos = relativePos;
				pos.y = 1000;
			}

			ImGui::DragFloat3(u8"Checked pos", (float*)&pos, 1.0f, -4000.0f, 4000.0f);

			static float length = 1000;
			ImGui::DragFloat(u8"Raycast length", &length, 1.0f, -2000.0f, 2000.0f);

			ImGui::Text(u8"All: %f", app::Miscs_CalcCurrentGroundHeight_1(avatarPos.x, avatarPos.z, avatarPos.y, length, 0xFFFFFFFF, nullptr));

			ImGui::TreePop();
		}
		if (ImGui::Button(u8"Copy Position"))
		{
			auto text = il2cppi_to_string(avatarPos);
			ImGui::SetClipboardText(text.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"Copy All Info"))
		{
			auto text = il2cppi_to_string(avatarPos) + u8"\n" + il2cppi_to_string(relativePos) + u8"\n" + il2cppi_to_string(levelPos) + u8"\n" + il2cppi_to_string(app::Miscs_CalcCurrentGroundNorm(avatarPos, nullptr));
			ImGui::SetClipboardText(text.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"Copy as json"))
		{
			std::string text = u8"\"position\":[";
			text += std::to_string(avatarPos.x) + u8",";
			text += std::to_string(avatarPos.y) + u8",";
			text += std::to_string(avatarPos.z) + u8"]";
			std::string name = u8"";
			for (int i = 0; i < 10; i++)
				name += std::to_string(rand() % 10);
			text = u8"\"name\":\"" + name + u8"\"," + text;
			text = u8"{" + text + u8"}";
			ImGui::SetClipboardText(text.c_str());
		}

	}

	void DrawMapManager()
	{
		auto mapManager = GET_SINGLETON(MoleMole_MapManager);
		if (mapManager == nullptr)
			return;

		int temp = mapManager->fields.playerSceneID;
		ImGui::InputInt(u8"Player scene id", &temp);

		temp = mapManager->fields.mapSceneID;
		ImGui::InputInt(u8"Map scene id", &temp);
	}

	void DrawImGuiFocusTest()
	{
		ImGui::Text(u8"Is any item active: %s", ImGui::IsAnyItemActive() ? u8"true" : u8"false");
		ImGui::Text(u8"Is any item focused: %s", ImGui::IsAnyItemFocused() ? u8"true" : u8"false");

		ImGui::Button(u8"Test");
		auto hk = Hotkey();
		int temp = 0;
		InputHotkey(u8"Test hotkey", &hk, false);
		ImGui::InputInt(u8"Test input", &temp);
	}

	std::map<std::string, std::string> chestNames;
	std::unordered_set<std::string> notWrittenChests;
	bool showNotWritten = false;
	void OnGameUpdate()
	{
		if (!showNotWritten)
			return;

		auto& entityManager = game::EntityManager::instance();

		notWrittenChests.clear();
		for (auto& entity : entityManager.entities(game::filters::combined::Chests))
		{
			auto& entityName = entity->name();
			if (chestNames.count(entityName) == 0)
				notWrittenChests.insert(entityName);
		}
	}

	void DrawChestPlugin()
	{
		static std::map<std::string, std::string> tempNames;

		auto& entityManager = game::EntityManager::instance();
		ImGui::Checkbox(u8"Show not written", &showNotWritten);
		for (auto& entity : entityManager.entities(game::filters::combined::Chests))
		{
			auto& entityName = entity->name();
			if (showNotWritten && chestNames.count(entityName) > 0)
				continue;

			app::LCChestPlugin* chestPlugin = entity->plugin<app::LCChestPlugin>(*app::LCChestPlugin__TypeInfo);
			if (chestPlugin == nullptr)
				continue;

			if (!ImGui::TreeNode(entity, u8"Chest 0x%p, Distance: %f", entity, entityManager.avatar()->distance(entity)))
				continue;

			auto& pluginData = chestPlugin->fields;
			auto& owner = pluginData._owner->fields;
			auto& ownerData = owner._dataItem->fields;
			app::GadgetState__Enum chestState = static_cast<app::GadgetState__Enum>(ownerData.gadgetState);
			ImGui::Text(u8"Is ability locked: %s", pluginData._isLockByAbility ? u8"true" : u8"false");
			ImGui::Text(u8"State: %s", magic_enum::enum_name(chestState).data());

			bool added = chestNames.count(entityName) > 0;

			if (tempNames.count(entityName) == 0)
				tempNames[entityName] = added ? chestNames[entityName] : std::string();

			auto& tempName = tempNames[entityName];
			ImGui::PushID(entity);

			ImGui::Text(u8"Name: %s", entityName.c_str());
			ImGui::InputText(u8"Friendly name", &tempName);

			if (ImGui::Button(added ? u8"Update" : u8"Add"))
				chestNames[entityName] = tempName;

			if (ImGui::Button(u8"Teleport"))
			{
				auto& mapTeleport = MapTeleport::GetInstance();
				mapTeleport.TeleportTo(entity->absolutePosition());
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode(u8"Chest dictionary"))
		{
			std::stringstream text;
			text << u8"{\n";
			for (auto& [rawName, friendlyName] : chestNames)
			{
				text << u8"\t\"" << friendlyName << u8"\" : \"" << rawName << u8"\",\n";
			}
			text << u8"}";
			std::string textStr = text.str();
			ImGui::InputTextMultiline(u8"Dict", &textStr);
			ImGui::TreePop();
		}

	}

	void DrawScenePropManager()
	{
		auto scenePropManager = GET_SINGLETON(MoleMole_ScenePropManager);
		if (scenePropManager == nullptr)
		{
			ImGui::Text(u8"Scene prop manager not loaded.");
			return;
		}

		auto scenePropDict = TO_UNI_DICT(scenePropManager->fields._scenePropDict, int32_t, app::Object*);
		if (scenePropDict == nullptr)
		{
			ImGui::Text(u8"Scene prop dict is nullptr.");
			return;
		}

		ImGui::Text(u8"Prop count: %d", scenePropDict->count);

		auto& manager = game::EntityManager::instance();
		for (auto& [id, propObject] : scenePropDict->pairs())
		{
			auto tree = CastTo<app::SceneTreeObject>(propObject, *app::SceneTreeObject__TypeInfo);
			if (tree == nullptr)
				continue;

			auto pos = tree->fields._.realBounds.m_Center;
			auto config = tree->fields._config->fields;

			auto pattern = config._._.scenePropPatternName;
			app::MoleMole_Config_TreeType__Enum value;
			bool result = app::MoleMole_ScenePropManager_GetTreeTypeByPattern(scenePropManager, pattern, &value, nullptr);
			if (!result)
				continue;

			ImGui::Text(u8"Tree at %s, type: %s, distance %0.3f", il2cppi_to_string(pos).c_str(), magic_enum::enum_name(value).data(),
				manager.avatar()->distance(app::WorldShiftManager_GetRelativePosition(pos, nullptr)));
		}
	}

	class ItemFilter : game::IEntityFilter
	{
	public:
		ItemFilter() : ItemFilter(app::EntityType__Enum_1::None, u8"")
		{}

		ItemFilter(app::EntityType__Enum_1 type, const std::string& name) : m_Type(type), m_Name(name)
		{

		}

		bool IsValid(game::Entity* entity) const override
		{
			return entity->type() == m_Type && entity->name() == m_Name;
		}

		app::EntityType__Enum_1 m_Type;
		std::string m_Name;
	};

	static bool filtersIsLoaded = false;
	static std::map<std::string, ItemFilter> simpleFilters;
	static std::vector<ItemFilter> removedItems;

	static const std::string filename = u8"picked_filters.json";
	static bool filterItemPickerEnabled = false;

	static ItemFilter tempFilter;
	static std::string tempName;
	static std::string tempSectionName;

	static bool addingFilter;
	static game::CacheFilterExecutor executor;

	void FilterItemPickerLoad()
	{
		filtersIsLoaded = true;

		std::ifstream fs(filename, std::ios::in);
		if (!fs.is_open())
			return;

		nlohmann::json jRoot;
		try {
			jRoot = nlohmann::json::parse(fs);
		}
		catch (nlohmann::detail::parse_error& parseError)
		{
			UNREFERENCED_PARAMETER(parseError);
			LOG_ERROR(u8"Failed to parse json");
		}

		for (auto& [key, value] : jRoot["filters"].items())
			simpleFilters[key] = ItemFilter(value["type"], value["name"]);

		for (auto& value : jRoot["excluded"])
			removedItems.push_back(ItemFilter(value["type"], value["name"]));
	}

	void FiltetItemPickerSave()
	{
		std::ofstream fs(filename, std::ios::out);
		if (!fs.is_open())
		{
			LOG_ERROR(u8"Failed to save changes.");
			return;
		}

		nlohmann::json jRoot = {};
		jRoot["filters"] = {};
		for (auto& [key, value] : simpleFilters)
		{
			jRoot["filters"][key] = {};
			jRoot["filters"][key]["name"] = value.m_Name;
			jRoot["filters"][key]["type"] = value.m_Type;
		}

		jRoot["excluded"] = {};
		for (auto& value : removedItems)
		{
			nlohmann::json item = {};
			item["name"] = value.m_Name;
			item["type"] = value.m_Type;

			jRoot["excluded"].push_back(item);
		}

		fs << jRoot.dump(4);
		fs.close();
	}

	void DrawFilterItemPicker()
	{
		ImGui::Checkbox(u8"Enable ## itemPicker", &filterItemPickerEnabled);
		if (!filterItemPickerEnabled)
			return;

		if (!filtersIsLoaded)
			FilterItemPickerLoad();

		for (auto& [key, filter] : simpleFilters)
		{
			ImGui::PushID(key.c_str());
			ImGui::PushItemWidth(250);

			std::string keyText = key;
			ImGui::InputText(u8"## Name", &keyText);
			ImGui::SameLine();

			ImGui::InputText(u8"## ItemName", &filter.m_Name);
			ImGui::SameLine();

			std::string typeName = std::string(magic_enum::enum_name(filter.m_Type));
			ImGui::InputText(u8"## ItemType", &typeName);

			ImGui::PopItemWidth();
			ImGui::PopID();
		}
	}

	void Debug::DrawExternal()
	{
		//auto draw = ImGui::GetBackgroundDrawList();

		//std::string fpsString = fmt::format(u8"{:.1f}/{:.1f}", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//draw->AddText(ImVec2(100, 100), ImColor(0, 0, 0), fpsString.c_str());

		if (!filterItemPickerEnabled)
			return;

		auto& manager = game::EntityManager::instance();

		game::Entity* selectedEntity = nullptr;
		esp::render::PrepareFrame();

		for (auto& entity : manager.entities())
		{
			bool unexplored = true;
			for (auto& [_, filter] : simpleFilters)
			{
				if (executor.ApplyFilter(entity, reinterpret_cast<game::IEntityFilter*>(&filter)))
				{
					unexplored = false;
					break;
				}
			}

			for (auto& filter : removedItems)
			{
				if (executor.ApplyFilter(entity, reinterpret_cast<game::IEntityFilter*>(&filter)))
				{
					unexplored = false;
					break;
				}
			}

			if (!unexplored)
				continue;

			bool isSelected = esp::render::DrawEntity(entity->name(), entity, ImColor(255, 0, 0, 255), ImColor(255, 0, 0, 255));
			if (isSelected && selectedEntity == nullptr)
			{
				esp::render::DrawEntity(entity->name(), entity, ImColor(0, 255, 0, 255), ImColor(255, 0, 255, 255));
				selectedEntity = entity;
			}
		}


		bool updated = false;

		if (!addingFilter)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_R, false) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			{
				if (removedItems.size() > 0)
				{
					removedItems.pop_back();
					updated = true;
				}
			}
			else if (selectedEntity != nullptr && ImGui::IsKeyPressed(ImGuiKey_R, false))
			{
				removedItems.push_back(ItemFilter(selectedEntity->type(), selectedEntity->name()));
				updated = true;
			}

			if (selectedEntity != nullptr && ImGui::IsKeyPressed(ImGuiKey_T, false))
			{
				tempFilter = ItemFilter(selectedEntity->type(), selectedEntity->name());
				addingFilter = true;
				tempName = u8"";
				renderer::SetInputLock(this, true);
			}
		}

		if (addingFilter)
		{
			ImGui::Begin(u8"Input name", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::PushItemWidth(500);
			ImGui::InputText(u8"Section", &tempSectionName);
			if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
				ImGui::SetKeyboardFocusHere(0);
			ImGui::InputText(u8"Name", &tempName);
			ImGui::PopItemWidth();
			ImGui::End();

			if (ImGui::IsKeyPressed(ImGuiKey_Enter, false))
			{
				simpleFilters[fmt::format(u8"{}::{}", tempSectionName, tempName)] = tempFilter;
				renderer::SetInputLock(this, false);
				addingFilter = false;
				updated = true;
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
			{
				renderer::SetInputLock(this, false);
				addingFilter = false;
			}
		}

		if (updated)
			FiltetItemPickerSave();
	}


	void DrawFPSGraph()
	{
		static float values[120] = { 0 };
		static int values_offset = 0;
		values[values_offset++] = ImGui::GetIO().Framerate;
		if (values_offset >= IM_ARRAYSIZE(values))
			values_offset = 0;
		ImGui::PlotLines(u8"", values, IM_ARRAYSIZE(values), values_offset, u8"", 0.0f, 100.0f, ImVec2(0, 80));
		ImGui::Text(u8"%.1f FPS", ImGui::GetIO().Framerate);
		float avg_fps = 0.0f;
		for (int i = 0; i < IM_ARRAYSIZE(values); i++)
			avg_fps += values[i];
		avg_fps /= IM_ARRAYSIZE(values);
		ImGui::Text(u8"%.1f FPS (avg)", avg_fps);
	}

	void Debug::DrawMain()
	{
		if (ImGui::CollapsingHeader(u8"Entity Manager", ImGuiTreeNodeFlags_None))
			DrawEntitiesData();

		if (ImGui::CollapsingHeader(u8"Position", ImGuiTreeNodeFlags_None))
		{
			DrawMapManager();
			DrawPositionInfo();
		}

		//if (ImGui::CollapsingHeader(u8"Filter item picker"))
		//    DrawFilterItemPicker();

		//if (ImGui::CollapsingHeader(u8"Chest plugin", ImGuiTreeNodeFlags_None))
		//	DrawChestPlugin();

		//if (ImGui::CollapsingHeader(u8"Interaction manager", ImGuiTreeNodeFlags_None))
		//	DrawInteractionManagerInfo();

		if (ImGui::CollapsingHeader(u8"Map Manager", ImGuiTreeNodeFlags_None))
			DrawManagerData();
		if (ImGui::CollapsingHeader(u8"FPS Graph", ImGuiTreeNodeFlags_None))
			DrawFPSGraph();
	}

	bool Debug::NeedInfoDraw() const
	{
		return showNotWritten && notWrittenChests.size() > 0;
	}

	void Debug::DrawInfo()
	{
		for (auto& name : notWrittenChests)
		{
			ImGui::Text(u8"%s", name.c_str());
		}
	}



}