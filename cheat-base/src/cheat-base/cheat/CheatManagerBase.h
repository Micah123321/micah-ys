#pragma once
#include <cheat-base/cheat/Feature.h>
#include <cheat-base/config/Config.h>
#include <cheat-base/events/event.hpp>
#include <cheat-base/render/renderer.h>

#include <vector>
#include <map>
#include <string>

#include <Windows.h>

namespace cheat 
{
	class CheatManagerBase
	{
	public:
		
		static bool IsMenuShowed();

		//static CheatManagerBase& GetInstance();
		CheatManagerBase(CheatManagerBase const&) = delete;
		void operator=(CheatManagerBase const&) = delete;

		void AddFeature(Feature* feature);
		void AddFeatures(std::vector<Feature*> features);

		void SetModuleOrder(std::vector<std::string> moduleOrder);

		void OnKeyUp(short key, bool& cancelled);
		void OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& cancelled);
		void OnRender();
		//��ʼ������d3d11
		void Init(LPBYTE pFontData, DWORD dFontDataSize, renderer::DXVersion dxVersion = renderer::DXVersion::D3D11);

		virtual void CursorSetVisibility(bool visibility) = 0;
		virtual bool CursorGetVisibility() = 0;

	protected:
		config::Field<size_t> m_SelectedSection;

		std::vector<Feature*> m_Features;
		std::vector<std::string> m_ModuleOrder;
		std::map<std::string, std::map<std::string, std::vector<Feature*>>> m_FeatureMap;

		inline static bool s_IsMenuShowed = false;
		bool m_IsBlockingInput;
		bool m_IsPrevCursorActive;
		bool m_IsProfileConfigurationShowed;

		explicit CheatManagerBase();

		void DrawExternal() const;

		void DrawMenu();
		void DrawMenuSection(const std::string& sectionName, const std::vector<Feature*>& features) const;

		virtual void DrawProfileGlobalActivities();
		virtual void DrawProfileEntryActivities(const std::string&profileName);
		virtual void DrawProfileEntry(const std::string& profileName);
		virtual void DrawProfileTableHeader();
		virtual int GetProfileTableColumnCount();
		virtual void DrawProfileConfiguration();
		virtual void DrawProfileLine();

		virtual void DrawStatus() const;
		virtual void DrawInfo();
		void DrawFps();
		static void DrawNotifications();
		void PushFeature(Feature* feature);
		void CheckToggles(short key) const;

		void ToggleMenuShow();
	};
}


