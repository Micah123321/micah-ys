#include "pch-il2cpp.h"
#include "VacuumLoot.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>

namespace cheat::feature
{
	VacuumLoot::VacuumLoot() : Feature(),
		NF(f_Enabled, u8"吸战利品", u8"吸战利品", false),
		NF(f_DelayTime, u8"延迟时间 (毫秒)", u8"吸战利品", 1000),
		NF(f_Distance, u8"距离", u8"吸战利品", 1.5f),
		NF(f_MobDropRadius, u8"怪物物品掉落半径", u8"吸战利品", 20.0f),
		NF(f_Radius, u8"半径", u8"吸战利品", 20.0f),
		nextTime(0)
	{
		InstallFilters();
		events::GameUpdateEvent += MY_METHOD_HANDLER(VacuumLoot::OnGameUpdate);
	}

	const FeatureGUIInfo& VacuumLoot::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"吸战利品", u8"世界", true };
		return info;
	}

	void VacuumLoot::DrawMain()
	{

			ConfigWidget(u8"开启", f_Enabled, u8"吸战利品"); ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f);
			ConfigWidget(u8"延迟 (ms)", f_DelayTime, 1, 0, 1000, u8"吸战利品之间的延迟(毫秒).");
			ConfigWidget(u8"半径 (m)", f_Radius, 0.1f, 5.0f, 100.0f, u8"普通战利品真空的半径.");
			ConfigWidget(u8"怪物物品掉落半径(m", f_MobDropRadius, 0.1f, 5.0f, 100.0f, u8"暴徒掉落真空半径.\n"
			u8"(掉落物品和装备)");
			ConfigWidget(u8"距离 (m)", f_Distance, 0.1f, 1.0f, 10.0f, u8"玩家与战利品之间的距离.\n"
				u8"低于1.5的数值可能太过突兀.");
			if (ImGui::TreeNode(u8"掠夺类型"))
			{
				for (auto& [section, filters] : m_Sections)
				{
					ImGui::PushID(section.c_str());
					DrawSection(section, filters);
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
	}

	bool VacuumLoot::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void VacuumLoot::DrawStatus()
	{
		ImGui::Text(u8"吸战利品\n[%dms|%.01fm|%.01fm|%.01fm]",
			f_DelayTime.value(),
			f_Radius.value(),
			f_MobDropRadius.value(),
			f_Distance.value()
		);
	}

	VacuumLoot& VacuumLoot::GetInstance()
	{
		static VacuumLoot instance;
		return instance;
	}

	bool VacuumLoot::IsEntityForVac(game::Entity* entity)
	{
		// Go through all sections. For each section, go through all filters.
		// If a filter matches the given entity and that filter is enabled, return true.

		bool entityValid = std::any_of(m_Sections.begin(), m_Sections.end(),
			[entity](std::pair<std::string, Filters> const& section) {
				return std::any_of(section.second.begin(), section.second.end(), [entity](const FilterInfo& filterInfo) {
					return filterInfo.second->IsValid(entity) && filterInfo.first; });
			});

		if (!entityValid) return false;

		bool isMobDrop = std::any_of(m_MobDropFilter.begin(), m_MobDropFilter.end(),
			[entity](const game::IEntityFilter* filter) { return filter->IsValid(entity); });

		auto& manager = game::EntityManager::instance();
		auto distance = manager.avatar()->distance(entity);

		return distance <= (isMobDrop ? f_MobDropRadius : f_Radius);
	}

	void VacuumLoot::OnGameUpdate()
	{
		if (!f_Enabled)
			return;

		auto currentTime = util::GetCurrentTimeMillisec();
		if (currentTime < nextTime)
			return;

		auto& manager = game::EntityManager::instance();
		auto avatarEntity = manager.avatar();
		for (const auto& entity : manager.entities())
		{
			if (!IsEntityForVac(entity))
				continue;

			entity->setRelativePosition(avatarEntity->relativePosition() + avatarEntity->forward() * f_Distance);
		}
		nextTime = currentTime + f_DelayTime.value();
	}

	void VacuumLoot::DrawSection(const std::string& section, const Filters& filters)
	{
		bool checked = std::all_of(filters.begin(), filters.end(), [](const FilterInfo& filter) {  return filter.first; });
		bool changed = false;

		if (ImGui::BeginSelectableGroupPanel(section.c_str(), checked, changed, true))
		{
			// TODO : Get Max Container Width and Calculate Max Item Width of Checkbox + Text / or specify same width for all columns
			// then divide MaxWidth by ItemWidth/ColumnWidth and asign a floor result >= 1 to columns.
			// Though this is also just fine IMO.

			int columns = 3;

			if (ImGui::BeginTable(section.c_str(), columns == 0 ? 1 : columns )) {
				int i = 0;
				for (std::pair<config::Field<bool>, game::IEntityFilter*> filter : filters) {

					if (i % (columns == 0 ? 1 : columns) == 0)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
					}
					else
						ImGui::TableNextColumn();

					ImGui::PushID(&filter);
					ConfigWidget(filter.first);
					ImGui::PopID();
					i++;
				}
				ImGui::EndTable();
			}
		}
		ImGui::EndSelectableGroupPanel();

		if (changed)
		{
			for (const auto& info : filters)
			{
				info.first.value() = checked;
				info.first.FireChanged();
			}
		}

	}

	void VacuumLoot::AddFilter(const std::string& section, const std::string& name, game::IEntityFilter* filter)
	{
		if (m_Sections.count(section) == 0)
			m_Sections[section] = {};

		auto& filters = m_Sections[section];
		bool newItem(filter);
		filters.push_back({ config::CreateField<bool>(name,name,fmt::format(u8"VacuumLoot::Filters::{}", section),false, newItem) , filter });
	}

#define ADD_FILTER_FIELD(section, name) AddFilter(util::MakeCapital(#section), util::SplitWords(#name), &game::filters::##section##::##name##)
	void VacuumLoot::InstallFilters()
	{
		ADD_FILTER_FIELD(featured, ItemDrops);

		ADD_FILTER_FIELD(equipment, Artifacts);
		ADD_FILTER_FIELD(equipment, Bow);
		ADD_FILTER_FIELD(equipment, Catalyst);
		ADD_FILTER_FIELD(equipment, Claymore);
		ADD_FILTER_FIELD(equipment, Sword);
		ADD_FILTER_FIELD(equipment, Pole);

		ADD_FILTER_FIELD(mineral, AmethystLumpDrop);
		ADD_FILTER_FIELD(mineral, CrystalChunkDrop);
		ADD_FILTER_FIELD(mineral, ElectroCrystalDrop);
		ADD_FILTER_FIELD(mineral, IronChunkDrop);
		ADD_FILTER_FIELD(mineral, NoctilucousJadeDrop);
		ADD_FILTER_FIELD(mineral, MagicalCrystalChunkDrop);
		ADD_FILTER_FIELD(mineral, ScarletQuartzDrop);
		ADD_FILTER_FIELD(mineral, StarsilverDrop);
		ADD_FILTER_FIELD(mineral, WhiteIronChunkDrop);

		ADD_FILTER_FIELD(plant, Apple);
		ADD_FILTER_FIELD(plant, Cabbage);
		ADD_FILTER_FIELD(plant, CarrotDrop);
		ADD_FILTER_FIELD(plant, Potato);
		ADD_FILTER_FIELD(plant, RadishDrop);
		ADD_FILTER_FIELD(plant, Sunsettia);
		ADD_FILTER_FIELD(plant, Wheat);

		ADD_FILTER_FIELD(living, CrystalCore);
		ADD_FILTER_FIELD(living, Meat);
		ADD_FILTER_FIELD(living, Crab);
		ADD_FILTER_FIELD(living, Eel);
		ADD_FILTER_FIELD(living, LizardTail);
		ADD_FILTER_FIELD(living, Fish);
	}
#undef ADD_FILTER_FIELD
}
