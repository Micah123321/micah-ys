#include "pch-il2cpp.h"
#include "CustomTeleports.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include "MapTeleport.h"
#include <cheat/game/util.h>
#include <misc/cpp/imgui_stdlib.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <imgui_internal.h>
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

namespace cheat::feature
{
	CustomTeleports::CustomTeleports() : Feature(),
		NF(f_Enabled, u8"�Զ��崫��", u8"�Զ��崫��", false),
		NF(f_Next, u8"������һ��", u8"�Զ��崫��", Hotkey(VK_OEM_6)),
		NF(f_Previous, u8"����ǰһ��", u8"�Զ��崫��", Hotkey(VK_OEM_4)),
		NF(f_Interpolate, u8"�Զ��崫��", u8"�Զ��崫��", false),
		NF(f_Speed, u8"˲���ٶ�", u8"�Զ��崫��", 10.0f),
		dir(util::GetCurrentPath() / u8"teleports")
	{
		f_Next.value().PressedEvent += MY_METHOD_HANDLER(CustomTeleports::OnNext);
		f_Previous.value().PressedEvent += MY_METHOD_HANDLER(CustomTeleports::OnPrevious);
	}

	const FeatureGUIInfo& CustomTeleports::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"�Զ��崫��", u8"����", true };
		return info;
	}

	void CustomTeleports::CheckFolder()
	{
		if (!std::filesystem::exists(dir))
			std::filesystem::create_directory(dir);
		else return;
	}

	bool CustomTeleports::ValidateTeleport(std::string name)
	{
		for (auto& Teleport : Teleports)
			if (Teleport.name == name)
				return false;
		if (name.find_first_of(u8"\\/:*?\"<>|") != std::string::npos)
			return false;
		return true;
	}

	Teleport CustomTeleports::Teleport_(std::string name, app::Vector3 position, std::string description)
	{
		Teleport t(name, position, description);
		return t;
	}

	void CustomTeleports::SerializeTeleport(Teleport t)
	{
		Teleports.push_back(t);
		LOG_INFO(u8"Teleport '%s' Loaded", t.name.c_str());
		CheckFolder();
		std::ofstream ofs(dir / (t.name + u8".json"));
		nlohmann::json j;
		try
		{
			j["name"] = t.name;
			j["position"] = { t.position.x, t.position.y, t.position.z };
			j["description"] = t.description;
			ofs << j;
			ofs.close();
			LOG_INFO(u8"Teleport '%s' Serialized.", t.name.c_str());
		}
		catch (std::exception e)
		{
			ofs.close();
			LOG_ERROR(u8"Failed to serialize teleport: %s: %s", t.name.c_str(), e.what());
		}
	}

	Teleport CustomTeleports::SerializeFromJson(std::string json, bool fromfile)
	{
		nlohmann::json j;
		try { j = nlohmann::json::parse(json); }
		catch (nlohmann::json::parse_error& e)
		{
			LOG_ERROR(u8"Invalid JSON Format");
			LOG_ERROR(u8"Failed to parse JSON: %s", e.what());
		}
		std::string teleportName;
		teleportName = j["name"];
		if (j["name"].is_null() && fromfile)
		{
			LOG_ERROR(u8"No name found! Using File Name");
			teleportName = std::filesystem::path(json).stem().filename().string();
		}
		std::string description;
		if (j["description"].is_null()) description = u8"";
		else description = j["description"];
		return Teleport_(teleportName, { j["position"][0], j["position"][1], j["position"][2] }, description);
	}

	void CustomTeleports::ReloadTeleports()
	{
		auto result = std::filesystem::directory_iterator(dir);
		Teleports.clear();

		for (auto& file : result)
		{
			if (file.path().extension() == u8".json")
			{
				std::ifstream ifs(file.path());
				std::string json;
				std::getline(ifs, json);
				SerializeTeleport(SerializeFromJson(json, true));
			}
		}
	}

	float PositionDistance(app::Vector3 a, app::Vector3 b)
	{
		return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
	}

	void CustomTeleports::TeleportTo(app::Vector3 position, bool interpolate)
	{
		auto &manager = game::EntityManager::instance();
		auto avatar = manager.avatar();
		if (avatar->moveComponent() == nullptr)
		{
			LOG_ERROR(u8"Avatar has no move component, Is scene loaded?");
			return;
		}
		if (interpolate)
		{
			float speed = this->f_Speed;
			auto avatarPos = manager.avatar()->absolutePosition();
			auto endPos = position;
			std::thread interpolate([avatarPos, endPos, &manager, speed]()
									{
                            float t = 0.0f;
							app::Vector3 zero = {0,0,0};
							auto newPos = zero;
                            while (t < 1.0f) {
                                newPos = app::Vector3_Lerp(avatarPos, endPos, t, nullptr);
                                manager.avatar()->setAbsolutePosition(newPos);
								t += speed / 100.0f;
                                Sleep(10); 
                            } });
			interpolate.detach();
		}
		else
		{
			if (PositionDistance(position, app::ActorUtils_GetAvatarPos(nullptr)) > 60.0f)
				MapTeleport::GetInstance().TeleportTo(position);
			else
				manager.avatar()->setAbsolutePosition(position);
		}
	}

	void CustomTeleports::OnTeleportKeyPressed(bool next)
	{
		if (!f_Enabled || selectedIndex < 0)
			return;

		auto& mapTeleport = MapTeleport::GetInstance();
		app::Vector3 position;

		if (selectedByClick)
		{
			position = Teleports.at(selectedIndex).position;
			selectedByClick = false;
		}
		else
		{
			std::vector list(checkedIndices.begin(), checkedIndices.end());
			if (next ?  selectedIndex == list.back() : selectedIndex == list.front())
				return;

			auto index = std::distance(list.begin(), std::find(list.begin(), list.end(), selectedIndex));
			selectedIndex = list.at(index + (next ? 1 : -1));
			position = Teleports.at(selectedIndex).position;
		}
		TeleportTo(position, this->f_Interpolate);
		UpdateIndexName();
	}

	void CustomTeleports::OnPrevious()
	{
		OnTeleportKeyPressed(false);
	}
	void CustomTeleports::OnNext()
	{
		OnTeleportKeyPressed(true);
	}

	void itr(std::regex exp, std::string name, std::string s)
	{
		std::sregex_iterator itr(name.begin(), name.end(), exp);
		while (itr != std::sregex_iterator())
		{
			for (unsigned i = 0; i < itr->size(); i++)
				s.append((*itr)[i]);
			itr++;
		}
	}

	void CustomTeleports::UpdateIndexName()
	{
		// abbreviate teleport names that are too long
		std::string name(selectedIndex == -1 || checkedIndices.empty() ? u8"" : Teleports.at(selectedIndex).name);
		if (name.length() > 15)
		{
			std::string shortened;
			std::regex numsExp(u8"[\\d]+");
			std::regex firstCharsExp(u8"\\b[A-Za-z]");
			itr(firstCharsExp, name, shortened);
			itr(numsExp, name, shortened);
			name = shortened;
		}
		selectedIndexName = name;
	}

	void CustomTeleports::DrawMain()
	{
		// Buffers
		static std::string nameBuffer_;
		static std::string searchBuffer_;
		static std::string JSONBuffer_;
		static std::string descriptionBuffer_;

		ImGui::InputText(u8"����", &nameBuffer_);
		ImGui::InputText(u8"��ע", &descriptionBuffer_);
		if (ImGui::Button(u8"���Ӵ��͵�"))
		{
			selectedIndex = -1;
			UpdateIndexName();
			SerializeTeleport(Teleport_(nameBuffer_, app::ActorUtils_GetAvatarPos(nullptr), descriptionBuffer_));
			nameBuffer_ = u8"";
			descriptionBuffer_ = u8"";
		}
		ImGui::SameLine();

		if (ImGui::Button(u8"���¼���"))
		{
			selectedIndex = -1;
			UpdateIndexName();
			checkedIndices.clear();
			ReloadTeleports();
		}

		ImGui::SameLine();
		if (ImGui::Button(u8"���ļ���"))
		{
			CheckFolder();
			ShellExecuteA(NULL, u8"��", dir.string().c_str(), NULL, NULL, SW_SHOW);
		}

		ImGui::SameLine();
		if (ImGui::Button(u8"��JSON����"))
		{
			selectedIndex = -1;
			UpdateIndexName();
			SerializeTeleport(SerializeFromJson(JSONBuffer_, false));
			JSONBuffer_ = u8"";
		}
		ImGui::InputTextMultiline(u8"JSON����", &JSONBuffer_, ImVec2(0, 50), ImGuiInputTextFlags_AllowTabInput);

		ConfigWidget(u8"������һ", f_Next, true, u8"���´���ѡ������һ��");
		ConfigWidget(u8"����֮ǰ", f_Previous, true, u8"������ѡ�е�ǰһ��");
		ConfigWidget(u8"����", f_Enabled,
					 u8"���ô���ͨ���б�����\n"
					 u8"ʹ��:\n"
					 u8"1. ��Checkmark�����봫��ʹ���ȼ�\n"
					 u8"2. ��������(���и�ѡ���)��ѡ������Ҫ��ʼ��λ��\n"
					 u8"3. �����ڿ��԰�����һ������һ���ȼ�����ͨ������\n"
					 u8"�������������ҵ���������ѡ��\n"
					 u8"ע��:˫���򵥻���ͷ�򿪴��͵���ϸ��Ϣ");
		ConfigWidget(u8"˲��", f_Interpolate, u8"ʹ�ü���ʱ�����ô���֮��Ĳ�ֵ");
		ConfigWidget(u8"˲���ٶ�", f_Speed, 0.1f, 0.1f, 99.0f,
					 u8"˲���ٶ�.\n �������õ��ڻ����0.1.");

		if (ImGui::Button(u8"ɾ�� ���"))
		{
			if (!Teleports.empty())
			{
				if (checkedIndices.empty())
				{
					LOG_INFO(u8"No teleports selected");
					return;
				}
				std::vector<std::string> teleportNames;
				for (auto& Teleport : Teleports)
					teleportNames.push_back(Teleport.name);
				for (auto& index : checkedIndices)
				{
					std::filesystem::remove(dir / (teleportNames[index] + u8".json"));
					LOG_INFO(u8"Deleted teleport %s", teleportNames[index].c_str());
				}
				checkedIndices.clear();
				UpdateIndexName();
				ReloadTeleports();
			}
			else { LOG_INFO(u8"No teleports to delete"); }
		}
		ImGui::SameLine();
		HelpMarker(u8"����:�⽫ɾ�����ļ���Ŀ¼��\n \
		���б����Ƴ����͡�������Զʧȥ.");

		if (ImGui::TreeNode(u8"Teleports"))
		{
			std::sort(Teleports.begin(), Teleports.end(), [](const auto &a, const auto &b)
					  { return StrCmpLogicalW(std::wstring(a.name.begin(), a.name.end()).c_str(), std::wstring(b.name.begin(), b.name.end()).c_str()) < 0; });
			bool allSearchChecked = std::includes(checkedIndices.begin(), checkedIndices.end(), searchIndices.begin(), searchIndices.end()) && !searchIndices.empty();
			bool allChecked = (checkedIndices.size() == Teleports.size() && !Teleports.empty()) || allSearchChecked;
			ImGui::Checkbox(u8"All", &allChecked);
			if (ImGui::IsItemClicked())
			{
				if (!Teleports.empty())
				{
					if (allChecked)
					{
						selectedIndex = -1;
						if (!searchIndices.empty())
							for (const auto& i : searchIndices)
								checkedIndices.erase(i);
						else
							checkedIndices.clear();
					}
					else if (!searchIndices.empty())
						checkedIndices.insert(searchIndices.begin(), searchIndices.end());
					else
						for (int i = 0; i < Teleports.size(); i++)
							checkedIndices.insert(i);
					UpdateIndexName();
				}
			}
			ImGui::SameLine();
			ImGui::InputText(u8"����", &searchBuffer_);
			unsigned int index = 0;
			searchIndices.clear();

			unsigned int maxNameLength = 0;
			for (auto& Teleport : Teleports)
				if (Teleport.name.length() > maxNameLength)
					maxNameLength = Teleport.name.length();
			ImGui::BeginTable(u8"����", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings);
			ImGui::TableSetupColumn(u8"#", ImGuiTableColumnFlags_WidthFixed, 20);
			ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, maxNameLength * 8 + 10);
			ImGui::TableSetupColumn(u8"�ص�");
			ImGui::TableHeadersRow();

			for (const auto& [name, position, description] : Teleports)
			{
				if (searchBuffer_.empty() || std::search(name.begin(), name.end(), searchBuffer_.begin(), searchBuffer_.end(), [](char a, char b)
					{ return std::tolower(a) == std::tolower(b); }) != name.end())
				{
					if (!searchBuffer_.empty())
						searchIndices.insert(index);
					bool checked = std::any_of(checkedIndices.begin(), checkedIndices.end(), [&index](const auto& i)
						{ return i == index; });
					bool selected = index == selectedIndex;
					std::string stringIndex = std::to_string(index);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text(u8"%d", index);
					ImGui::TableNextColumn();
					ImGui::Checkbox((u8"##�±�" + stringIndex).c_str(), &checked);
					if (ImGui::IsItemClicked(0))
					{
						if (checked)
						{
							if (selected)
								selectedIndex = -1;
							checkedIndices.erase(index);
						}
						else
							checkedIndices.insert(index);
						UpdateIndexName();
					}

					ImGui::SameLine();
					if (ImGui::Button((u8"����##��ť" + stringIndex).c_str()))
					{
						TeleportTo(position, false);
					}

					ImGui::SameLine();
					if (ImGui::Button((u8"ƽ��##��ť" + stringIndex).c_str()))
					{
						TeleportTo(position, true);
					}
					ImGui::SameLine();

					if (ImGui::Button((u8"ѡ��##��ť" + stringIndex).c_str()))
					{
						selectedIndex = index;
						selectedByClick = true;
						UpdateIndexName();
					}
					ImGui::TableNextColumn();

					ImGui::PushStyleColor(ImGuiCol_Text, selected ? IM_COL32(40, 90, 175, 255) : ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
					ImGui::Text(u8"%s", name.c_str());
					ImGui::PopStyleColor();
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(u8"%s", description.c_str());
						ImGui::Text(u8"����: %.2f", PositionDistance(position, app::ActorUtils_GetAvatarPos(nullptr)));
						ImGui::EndTooltip();
					}
					ImGui::TableNextColumn();
					ImGui::Text(u8"%f, %f, %f", position.x, position.y, position.z);
				}
				index++;
			}
			ImGui::EndTable();
			ImGui::TreePop();
		}

		if (selectedIndex != -1)
			ImGui::Text(u8"ѡ��: [%d] %s", selectedIndex, Teleports[selectedIndex].name.c_str());
	}

	bool CustomTeleports::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void CustomTeleports::DrawStatus()
	{
		ImGui::Text(u8"�Զ��崫��\n[%s]", selectedIndexName);
	}

	CustomTeleports& CustomTeleports::GetInstance()
	{
		static CustomTeleports instance;
		return instance;
	}
}