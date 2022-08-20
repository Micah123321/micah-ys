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
		NF(f_Enabled, u8"自定义传送", u8"自定义传送", false),
		NF(f_Next, u8"传送下一个", u8"自定义传送", Hotkey(VK_OEM_6)),
		NF(f_Previous, u8"传送前一个", u8"自定义传送", Hotkey(VK_OEM_4)),
		NF(f_Interpolate, u8"自定义传送", u8"自定义传送", false),
		NF(f_Speed, u8"瞬移速度", u8"自定义传送", 10.0f),
		dir(util::GetCurrentPath() / u8"teleports")
	{
		f_Next.value().PressedEvent += MY_METHOD_HANDLER(CustomTeleports::OnNext);
		f_Previous.value().PressedEvent += MY_METHOD_HANDLER(CustomTeleports::OnPrevious);
	}

	const FeatureGUIInfo& CustomTeleports::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"自定义传送", u8"传送", true };
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

		ImGui::InputText(u8"名称", &nameBuffer_);
		ImGui::InputText(u8"备注", &descriptionBuffer_);
		if (ImGui::Button(u8"添加传送点"))
		{
			selectedIndex = -1;
			UpdateIndexName();
			SerializeTeleport(Teleport_(nameBuffer_, app::ActorUtils_GetAvatarPos(nullptr), descriptionBuffer_));
			nameBuffer_ = u8"";
			descriptionBuffer_ = u8"";
		}
		ImGui::SameLine();

		if (ImGui::Button(u8"重新加载"))
		{
			selectedIndex = -1;
			UpdateIndexName();
			checkedIndices.clear();
			ReloadTeleports();
		}

		ImGui::SameLine();
		if (ImGui::Button(u8"打开文件夹"))
		{
			CheckFolder();
			ShellExecuteA(NULL, u8"打开", dir.string().c_str(), NULL, NULL, SW_SHOW);
		}

		ImGui::SameLine();
		if (ImGui::Button(u8"从JSON加载"))
		{
			selectedIndex = -1;
			UpdateIndexName();
			SerializeTeleport(SerializeFromJson(JSONBuffer_, false));
			JSONBuffer_ = u8"";
		}
		ImGui::InputTextMultiline(u8"JSON输入", &JSONBuffer_, ImVec2(0, 50), ImGuiInputTextFlags_AllowTabInput);

		ConfigWidget(u8"传送下一", f_Next, true, u8"按下传送选定的下一个");
		ConfigWidget(u8"传送之前", f_Previous, true, u8"按传送选中的前一个");
		ConfigWidget(u8"启用", f_Enabled,
					 u8"启用传送通过列表功能\n"
					 u8"使用:\n"
					 u8"1. 把Checkmark到你想传送使用热键\n"
					 u8"2. 单击传送(带有复选标记)以选择您想要开始的位置\n"
					 u8"3. 你现在可以按下下一步或上一步热键传送通过检查表\n"
					 u8"最初它将传送玩家到所做出的选择\n"
					 u8"注意:双击或单击箭头打开传送的详细信息");
		ConfigWidget(u8"瞬移", f_Interpolate, u8"使用键绑定时，启用传送之间的插值");
		ConfigWidget(u8"瞬移速度", f_Speed, 0.1f, 0.1f, 99.0f,
					 u8"瞬移速度.\n 建议设置低于或等于0.1.");

		if (ImGui::Button(u8"删除 检查"))
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
		HelpMarker(u8"警告:这将删除该文件从目录和\n \
		从列表中移除传送。它将永远失去.");

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
			ImGui::InputText(u8"搜索", &searchBuffer_);
			unsigned int index = 0;
			searchIndices.clear();

			unsigned int maxNameLength = 0;
			for (auto& Teleport : Teleports)
				if (Teleport.name.length() > maxNameLength)
					maxNameLength = Teleport.name.length();
			ImGui::BeginTable(u8"传送", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings);
			ImGui::TableSetupColumn(u8"#", ImGuiTableColumnFlags_WidthFixed, 20);
			ImGui::TableSetupColumn(u8"命令", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn(u8"名称", ImGuiTableColumnFlags_WidthFixed, maxNameLength * 8 + 10);
			ImGui::TableSetupColumn(u8"地点");
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
					ImGui::Checkbox((u8"##下标" + stringIndex).c_str(), &checked);
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
					if (ImGui::Button((u8"传送##按钮" + stringIndex).c_str()))
					{
						TeleportTo(position, false);
					}

					ImGui::SameLine();
					if (ImGui::Button((u8"平滑##按钮" + stringIndex).c_str()))
					{
						TeleportTo(position, true);
					}
					ImGui::SameLine();

					if (ImGui::Button((u8"选择##按钮" + stringIndex).c_str()))
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
						ImGui::Text(u8"距离: %.2f", PositionDistance(position, app::ActorUtils_GetAvatarPos(nullptr)));
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
			ImGui::Text(u8"选择: [%d] %s", selectedIndex, Teleports[selectedIndex].name.c_str());
	}

	bool CustomTeleports::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void CustomTeleports::DrawStatus()
	{
		ImGui::Text(u8"自定义传送\n[%s]", selectedIndexName);
	}

	CustomTeleports& CustomTeleports::GetInstance()
	{
		static CustomTeleports instance;
		return instance;
	}
}
