#include <pch.h>
#include "Settings.h"

#include <cheat-base/cheat/CheatManagerBase.h>
#include <cheat-base/render/renderer.h>
#include <cheat-base/render/gui-util.h>
#include <misc/cpp/imgui_stdlib.h>
#include <cheat-base/util.h>

#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

namespace cheat::feature
{
	Settings::Settings() : Feature(),
		NF(f_MenuKey, reinterpret_cast<const char*>(u8"显示作弊菜单键位"), reinterpret_cast<const char*>(u8"通用"), Hotkey(VK_F1)),
		NF(f_HotkeysEnabled, reinterpret_cast<const char*>(u8"热键启用"), reinterpret_cast<const char*>(u8"通用"), true),
		
		NF(f_StatusMove, reinterpret_cast<const char*>(u8"移动状态窗口"), reinterpret_cast<const char*>(u8"通用::状态窗口"), true),
		NF(f_StatusShow, reinterpret_cast<const char*>(u8"显示状态窗口"), reinterpret_cast<const char*>(u8"通用::状态窗口"), true),

		NF(f_InfoMove, reinterpret_cast<const char*>(u8"移动信息窗口"), reinterpret_cast<const char*>(u8"通用::信息窗口"), true),
		NF(f_InfoShow, reinterpret_cast<const char*>(u8"显示信息窗口"), reinterpret_cast<const char*>(u8"通用::信息窗口"), true),

		NF(f_FpsMove, reinterpret_cast<const char*>(u8"移动FPS指示器"), reinterpret_cast<const char*>(u8"通用::FPS"), false),
		NF(f_FpsShow, reinterpret_cast<const char*>(u8"显示FPS指示器"), reinterpret_cast<const char*>(u8"通用::FPS"), true),

		NF(f_NotificationsShow, reinterpret_cast<const char*>(u8"显示通知"), reinterpret_cast<const char*>(u8"通用::通知"), true),
		NF(f_NotificationsDelay, reinterpret_cast<const char*>(u8"延迟通知"), reinterpret_cast<const char*>(u8"通用::通知"), 500),

		NF(f_FileLogging, reinterpret_cast<const char*>(u8"文件日志记录"), reinterpret_cast<const char*>(u8"通用::日志记录"), false),
		NF(f_ConsoleLogging, reinterpret_cast<const char*>(u8"控制台日志记录"), reinterpret_cast<const char*>(u8"通用::日志记录"), true),

		NF(f_FastExitEnable, reinterpret_cast<const char*>(u8"快速退出"), reinterpret_cast<const char*>(u8"通用::快速退出"), false),
		NF(f_HotkeyExit, reinterpret_cast<const char*>(u8"热键"), reinterpret_cast<const char*>(u8"通用::快速退出"), Hotkey(VK_F12)),

		NF(f_FontSize, reinterpret_cast<const char*>(u8"字体大小"), reinterpret_cast<const char*>(u8"通用"), 16.0f),
		NF(f_ShowStyleEditor, reinterpret_cast<const char*>(u8"显示颜色定制"), reinterpret_cast<const char*>(u8"通用"), false),
		//NFS(f_DefaultTheme, reinterpret_cast<const char*>(u8"主题"), reinterpret_cast<const char*>(u8"通用::颜色"), reinterpret_cast<const char*>(u8"默认"),
		//themesDir(util::GetCurrentPath() / "themes"))

		NFS(f_DefaultTheme, "Theme", "General::Colors", "Default"),
		themesDir(util::GetCurrentPath() / "themes")

	{
		renderer::SetGlobalFontSize(static_cast<float>(f_FontSize));
		f_HotkeyExit.value().PressedEvent += MY_METHOD_HANDLER(Settings::OnExitKeyPressed);
		if (!std::filesystem::exists(themesDir))
			std::filesystem::create_directory(themesDir);

	}

	bool inited = false;
	void Settings::Init() {
		if (this->f_DefaultTheme.value() != "Default" && !inited)
		{
			LOG_INFO("Loading theme: %s", themesDir / (f_DefaultTheme.value() + ".json").c_str());
			if (!std::filesystem::exists(themesDir / (f_DefaultTheme.value() + ".json")))
				f_DefaultTheme = "Default";
			else Colors_Import(f_DefaultTheme.value());
			inited = true;
		}
	}

	const FeatureGUIInfo& Settings::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ "", reinterpret_cast<const char*>(u8"设置"), false };
		return info;
	}

	void Settings::Colors_Export(std::string name)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		auto colors = style.Colors;

		nlohmann::json json;
		for (int i = 0; i < ImGuiCol_COUNT; i++)
			json[ImGui::GetStyleColorName((ImGuiCol)i)] = { colors[i].x, colors[i].y, colors[i].z, colors[i].w };
		std::ofstream file(themesDir / (name + ".json"));
		file << std::setw(4) << json << std::endl;
	}

	void Settings::Colors_Import(std::string name)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		auto colors = style.Colors;
		nlohmann::json json;
		std::ifstream file(themesDir / (name + ".json"));
		file >> json;
		for (int i = 0; i < ImGuiCol_COUNT; i++)
		{
			auto color = json[ImGui::GetStyleColorName((ImGuiCol)i)];
			colors[i].x = color[0];
			colors[i].y = color[1];
			colors[i].z = color[2];
			colors[i].w = color[3];
		}
	}

	void Settings::DrawMain()
	{

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"通用"));
		{
			ConfigWidget(f_MenuKey, false,
				reinterpret_cast<const char*>(u8"切换主菜单可见性的键。不能是空的.\n"\
				"如果您忘记了这个键，可以在配置文件中查看或设置它."));
			ConfigWidget(f_HotkeysEnabled, "支持热键.");
					}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"日志记录"));
		{
			bool consoleChanged = ConfigWidget(f_ConsoleLogging,
				reinterpret_cast<const char*>(u8"启用日志记录信息控制台(更改将在重新启动后生效)"));
			if (consoleChanged && !f_ConsoleLogging)
			{
				Logger::SetLevel(Logger::Level::None, Logger::LoggerType::ConsoleLogger);
			}

			bool fileLogging = ConfigWidget(f_FileLogging,
				reinterpret_cast<const char*>(u8"启用文件日志记录(更改将在重新启动后生效).\n" \
					"将在app目录下创建一个文件夹用于记录日志."));
			if (fileLogging && !f_FileLogging)
			{
				Logger::SetLevel(Logger::Level::None, Logger::LoggerType::FileLogger);
			}
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"状态窗口"));
		{
			ConfigWidget(f_StatusShow);
			ConfigWidget(f_StatusMove, reinterpret_cast<const char*>(u8"允许移动“状态”窗口."));
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"信息窗口"));
		{
			ConfigWidget(f_InfoShow);
			ConfigWidget(f_InfoMove, reinterpret_cast<const char*>(u8"允许移动“信息”窗口."));
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"FPS指示器"));
		{
			ConfigWidget(f_FpsShow);
			ConfigWidget(f_FpsMove, reinterpret_cast<const char*>(u8"允许移动'FPS指示器'窗口."));
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"显示通知"));
		{
			ConfigWidget(f_NotificationsShow, reinterpret_cast<const char*>(u8"窗口右下角将显示通知."));
			ConfigWidget(f_NotificationsDelay, 1, 1, 10000, reinterpret_cast<const char*>(u8"通知间隔的延迟，以毫秒为单位."));
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"快速退出"));
		{
			ConfigWidget(reinterpret_cast<const char*>(u8"启用"),
				f_FastExitEnable,
				reinterpret_cast<const char*>(u8"启用快速退出.\n")
			);
			if (!f_FastExitEnable)
				ImGui::BeginDisabled();

			ConfigWidget(reinterpret_cast<const char*>(u8"键"), f_HotkeyExit, true,
				reinterpret_cast<const char*>(u8"退出游戏键."));

			if (!f_FastExitEnable)
				ImGui::EndDisabled();
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"界面定制"));
		{
			if (ConfigWidget(f_FontSize, 1, 8, 64, reinterpret_cast<const char*>(u8"调整界面字体大小.")))
			{
				f_FontSize = std::clamp(f_FontSize.value(), 8, 64);
				renderer::SetGlobalFontSize(static_cast<float>(f_FontSize));
			}
			ImGui::Spacing();

			ConfigWidget(f_ShowStyleEditor, reinterpret_cast<const char*>(u8"显示颜色定制窗口."));
			ImGui::Spacing();

			ImGui::Text(reinterpret_cast<const char*>(u8"保存自定义颜色"));
			static std::string nameBuffer_;
			ImGui::InputText(reinterpret_cast<const char*>(u8"颜色名称"), &nameBuffer_);
			if (ImGui::Button(reinterpret_cast<const char*>(u8"保存")))
				Colors_Export(nameBuffer_);
				ImGui::SameLine();

			if (std::filesystem::exists(themesDir / (nameBuffer_ + ".json")))
			{
				if (this->f_DefaultTheme.value() != nameBuffer_)
				{
					if (ImGui::Button(reinterpret_cast<const char*>(u8"设置为默认")))
					{
						f_DefaultTheme = nameBuffer_;
					}
					ImGui::SameLine();
					if (ImGui::Button(reinterpret_cast<const char*>(u8"加载")))
					{
						Colors_Import(nameBuffer_);
					}
				}
			}
			else
			{
				ImGui::Text(reinterpret_cast<const char*>(u8"颜色不存在."));
			}
		}
		ImGui::EndGroupPanel();
	}

	Settings& Settings::GetInstance()
	{
		static Settings instance;
		return instance;
	}

	void Settings::OnExitKeyPressed()
	{
		if (!f_FastExitEnable || CheatManagerBase::IsMenuShowed())
			return;

		ExitProcess(0);
	}
}
