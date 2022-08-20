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
		NF(f_MenuKey, reinterpret_cast<const char*>(u8"��ʾ���ײ˵���λ"), reinterpret_cast<const char*>(u8"ͨ��"), Hotkey(VK_F1)),
		NF(f_HotkeysEnabled, reinterpret_cast<const char*>(u8"�ȼ�����"), reinterpret_cast<const char*>(u8"ͨ��"), true),
		
		NF(f_StatusMove, reinterpret_cast<const char*>(u8"�ƶ�״̬����"), reinterpret_cast<const char*>(u8"ͨ��::״̬����"), true),
		NF(f_StatusShow, reinterpret_cast<const char*>(u8"��ʾ״̬����"), reinterpret_cast<const char*>(u8"ͨ��::״̬����"), true),

		NF(f_InfoMove, reinterpret_cast<const char*>(u8"�ƶ���Ϣ����"), reinterpret_cast<const char*>(u8"ͨ��::��Ϣ����"), true),
		NF(f_InfoShow, reinterpret_cast<const char*>(u8"��ʾ��Ϣ����"), reinterpret_cast<const char*>(u8"ͨ��::��Ϣ����"), true),

		NF(f_FpsMove, reinterpret_cast<const char*>(u8"�ƶ�FPSָʾ��"), reinterpret_cast<const char*>(u8"ͨ��::FPS"), false),
		NF(f_FpsShow, reinterpret_cast<const char*>(u8"��ʾFPSָʾ��"), reinterpret_cast<const char*>(u8"ͨ��::FPS"), true),

		NF(f_NotificationsShow, reinterpret_cast<const char*>(u8"��ʾ֪ͨ"), reinterpret_cast<const char*>(u8"ͨ��::֪ͨ"), true),
		NF(f_NotificationsDelay, reinterpret_cast<const char*>(u8"�ӳ�֪ͨ"), reinterpret_cast<const char*>(u8"ͨ��::֪ͨ"), 500),

		NF(f_FileLogging, reinterpret_cast<const char*>(u8"�ļ���־��¼"), reinterpret_cast<const char*>(u8"ͨ��::��־��¼"), false),
		NF(f_ConsoleLogging, reinterpret_cast<const char*>(u8"����̨��־��¼"), reinterpret_cast<const char*>(u8"ͨ��::��־��¼"), true),

		NF(f_FastExitEnable, reinterpret_cast<const char*>(u8"�����˳�"), reinterpret_cast<const char*>(u8"ͨ��::�����˳�"), false),
		NF(f_HotkeyExit, reinterpret_cast<const char*>(u8"�ȼ�"), reinterpret_cast<const char*>(u8"ͨ��::�����˳�"), Hotkey(VK_F12)),

		NF(f_FontSize, reinterpret_cast<const char*>(u8"�����С"), reinterpret_cast<const char*>(u8"ͨ��"), 16.0f),
		NF(f_ShowStyleEditor, reinterpret_cast<const char*>(u8"��ʾ��ɫ����"), reinterpret_cast<const char*>(u8"ͨ��"), false),
		//NFS(f_DefaultTheme, reinterpret_cast<const char*>(u8"����"), reinterpret_cast<const char*>(u8"ͨ��::��ɫ"), reinterpret_cast<const char*>(u8"Ĭ��"),
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
		static const FeatureGUIInfo info{ "", reinterpret_cast<const char*>(u8"����"), false };
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

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"ͨ��"));
		{
			ConfigWidget(f_MenuKey, false,
				reinterpret_cast<const char*>(u8"�л����˵��ɼ��Եļ��������ǿյ�.\n"\
				"�����������������������������ļ��в鿴��������."));
			ConfigWidget(f_HotkeysEnabled, "֧���ȼ�.");
					}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"��־��¼"));
		{
			bool consoleChanged = ConfigWidget(f_ConsoleLogging,
				reinterpret_cast<const char*>(u8"������־��¼��Ϣ����̨(���Ľ���������������Ч)"));
			if (consoleChanged && !f_ConsoleLogging)
			{
				Logger::SetLevel(Logger::Level::None, Logger::LoggerType::ConsoleLogger);
			}

			bool fileLogging = ConfigWidget(f_FileLogging,
				reinterpret_cast<const char*>(u8"�����ļ���־��¼(���Ľ���������������Ч).\n" \
					"����appĿ¼�´���һ���ļ������ڼ�¼��־."));
			if (fileLogging && !f_FileLogging)
			{
				Logger::SetLevel(Logger::Level::None, Logger::LoggerType::FileLogger);
			}
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"״̬����"));
		{
			ConfigWidget(f_StatusShow);
			ConfigWidget(f_StatusMove, reinterpret_cast<const char*>(u8"�����ƶ���״̬������."));
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"��Ϣ����"));
		{
			ConfigWidget(f_InfoShow);
			ConfigWidget(f_InfoMove, reinterpret_cast<const char*>(u8"�����ƶ�����Ϣ������."));
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"FPSָʾ��"));
		{
			ConfigWidget(f_FpsShow);
			ConfigWidget(f_FpsMove, reinterpret_cast<const char*>(u8"�����ƶ�'FPSָʾ��'����."));
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"��ʾ֪ͨ"));
		{
			ConfigWidget(f_NotificationsShow, reinterpret_cast<const char*>(u8"�������½ǽ���ʾ֪ͨ."));
			ConfigWidget(f_NotificationsDelay, 1, 1, 10000, reinterpret_cast<const char*>(u8"֪ͨ������ӳ٣��Ժ���Ϊ��λ."));
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"�����˳�"));
		{
			ConfigWidget(reinterpret_cast<const char*>(u8"����"),
				f_FastExitEnable,
				reinterpret_cast<const char*>(u8"���ÿ����˳�.\n")
			);
			if (!f_FastExitEnable)
				ImGui::BeginDisabled();

			ConfigWidget(reinterpret_cast<const char*>(u8"��"), f_HotkeyExit, true,
				reinterpret_cast<const char*>(u8"�˳���Ϸ��."));

			if (!f_FastExitEnable)
				ImGui::EndDisabled();
		}
		ImGui::EndGroupPanel();

		ImGui::BeginGroupPanel(reinterpret_cast<const char*>(u8"���涨��"));
		{
			if (ConfigWidget(f_FontSize, 1, 8, 64, reinterpret_cast<const char*>(u8"�������������С.")))
			{
				f_FontSize = std::clamp(f_FontSize.value(), 8, 64);
				renderer::SetGlobalFontSize(static_cast<float>(f_FontSize));
			}
			ImGui::Spacing();

			ConfigWidget(f_ShowStyleEditor, reinterpret_cast<const char*>(u8"��ʾ��ɫ���ƴ���."));
			ImGui::Spacing();

			ImGui::Text(reinterpret_cast<const char*>(u8"�����Զ�����ɫ"));
			static std::string nameBuffer_;
			ImGui::InputText(reinterpret_cast<const char*>(u8"��ɫ����"), &nameBuffer_);
			if (ImGui::Button(reinterpret_cast<const char*>(u8"����")))
				Colors_Export(nameBuffer_);
				ImGui::SameLine();

			if (std::filesystem::exists(themesDir / (nameBuffer_ + ".json")))
			{
				if (this->f_DefaultTheme.value() != nameBuffer_)
				{
					if (ImGui::Button(reinterpret_cast<const char*>(u8"����ΪĬ��")))
					{
						f_DefaultTheme = nameBuffer_;
					}
					ImGui::SameLine();
					if (ImGui::Button(reinterpret_cast<const char*>(u8"����")))
					{
						Colors_Import(nameBuffer_);
					}
				}
			}
			else
			{
				ImGui::Text(reinterpret_cast<const char*>(u8"��ɫ������."));
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
