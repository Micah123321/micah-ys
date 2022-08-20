#include "pch-il2cpp.h"
#include "TextureChanger.h"

#include <helpers.h>
#include <cheat/events.h>
#include <misc/cpp/imgui_stdlib.h>
#include <fstream>

namespace cheat::feature
{
	namespace GameObject {
		app::GameObject* AvatarRoot = nullptr;
	}

	TextureChanger::TextureChanger() : Feature(),
		NF(f_Enabled, u8"Ƥ���ı�", u8"�Ӿ�::Ƥ���ı�", false),
		NF(f_HeadPath, u8"ͷ", u8"�Ӿ�::Ƥ���ı�", false),
		NF(f_BodyPath, u8"����", u8"�Ӿ�::Ƥ���ı�", false),
		NF(f_DressPath, u8"�·�", u8"�Ӿ�::Ƥ���ı�", false),
		toBeUpdate(), nextUpdate(0)
	{
		events::GameUpdateEvent += MY_METHOD_HANDLER(TextureChanger::OnGameUpdate);
	}

	const FeatureGUIInfo& TextureChanger::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"Ƥ���ı�", u8"�Ӿ�", true };
		return info;
	}

	void TextureChanger::DrawMain()
	{
		ConfigWidget(f_Enabled, u8"Ƥ���ı�.");
		ImGui::Text(u8"��ǰ��Ӣ��: %s", ActiveHero.c_str());

		ConfigWidget(f_HeadPath, u8"ͷ�ṹ.\n" \
			u8"ʾ��·��: C:\\Head.png");

		ConfigWidget(f_BodyPath, u8"���� Texture.\n" \
			u8"Example path: C:\\Body.png");

		ConfigWidget(f_DressPath, u8"�·� Texture.\n" \
			u8"Example path: C:\\Dress.png");

		if (ImGui::Button(u8"Ӧ��"))
			ApplyTexture = true;
	}

	bool TextureChanger::NeedStatusDraw() const
	{
		return f_Enabled;
	}

	void TextureChanger::DrawStatus()
	{
		ImGui::Text(u8"Ƥ���ı�");
	}

	TextureChanger& TextureChanger::GetInstance()
	{
		static TextureChanger instance;
		return instance;
	}

	void TextureChanger::OnGameUpdate()
	{
		if (!f_Enabled)
			return;

		auto currentTime = util::GetCurrentTimeMillisec();
		if (currentTime < nextUpdate)
			return;

		if (ApplyTexture)
		{
			GameObject::AvatarRoot = app::GameObject_Find(string_to_il2cppi(u8"/EntityRoot/AvatarRoot"), nullptr);

			if (GameObject::AvatarRoot != nullptr)
			{
				auto Transform = app::GameObject_GetComponentByName(GameObject::AvatarRoot, string_to_il2cppi(u8"Transform"), nullptr);
				auto HeroCount = app::Transform_get_childCount(reinterpret_cast<app::Transform*>(Transform), nullptr);

				for (int i = 0; i <= HeroCount - 1; i++)
				{
					auto HeroComponent = app::Transform_GetChild(reinterpret_cast<app::Transform*>(Transform), i, nullptr);
					auto HeroGameObject = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(HeroComponent), nullptr);
					auto isActiveHero = app::GameObject_get_active(HeroGameObject, nullptr);

					if (isActiveHero)
					{
						auto GameObjectName = app::Object_1_get_name(reinterpret_cast<app::Object_1*>(HeroGameObject), nullptr);
						ActiveHero = il2cppi_to_string(GameObjectName);
						auto OffsetDummy = app::GameObject_Find(string_to_il2cppi(u8"/EntityRoot/AvatarRoot/" + il2cppi_to_string(GameObjectName) + u8"/OffsetDummy"), nullptr);
						auto TransformOffsetDummy = app::GameObject_GetComponentByName(OffsetDummy, string_to_il2cppi(u8"Transform"), nullptr);
						auto TransformChildOffsetDummy = app::Transform_GetChild(reinterpret_cast<app::Transform*>(TransformOffsetDummy), 0, nullptr);
						auto OffsetGameObject = app::Component_1_get_gameObject(reinterpret_cast<app::Component_1*>(TransformChildOffsetDummy), nullptr);
						auto OffsetGameObjectName = app::Object_1_get_name(reinterpret_cast<app::Object_1*>(OffsetGameObject), nullptr);
						auto GameObjectBody = app::GameObject_Find(string_to_il2cppi(u8"/EntityRoot/AvatarRoot/" + il2cppi_to_string(GameObjectName) + u8"/OffsetDummy/" + il2cppi_to_string(OffsetGameObjectName) + u8"/Body"), nullptr);
						auto SkinnedMeshRenderer = app::GameObject_GetComponentByName(reinterpret_cast<app::GameObject*>(GameObjectBody), string_to_il2cppi(u8"SkinnedMeshRenderer"), nullptr);
						auto Material = app::Renderer_GetMaterialArray(reinterpret_cast<app::Renderer*>(SkinnedMeshRenderer), nullptr);
						// 0 - Hair, 1 - Body, 2 - Dress
						if (f_HeadPath && CheckFile(f_HeadPath)) {
							auto HeadTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(f_HeadPath), 100, false, false, false, nullptr);
							app::Material_set_mainTexture(Material->vector[0], reinterpret_cast<app::Texture*>(HeadTexture), nullptr);
						}
						if (f_BodyPath && CheckFile(f_BodyPath)) {
							auto BodyTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(f_BodyPath), 100, false, false, false, nullptr);
							app::Material_set_mainTexture(Material->vector[1], reinterpret_cast<app::Texture*>(BodyTexture), nullptr);
						}
						if (f_DressPath && CheckFile(f_DressPath)) {
							auto DressTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(f_DressPath), 100, false, false, false, nullptr);

							if (Material->vector[2] != nullptr)
								app::Material_set_mainTexture(Material->vector[2], reinterpret_cast<app::Texture*>(DressTexture), nullptr);
						}
						ApplyTexture = false;
					}
				}
			}
		}
		nextUpdate = currentTime + (int)f_DelayUpdate;
	}

	bool TextureChanger::CheckFile(const std::string& Filename) {
		struct stat buffer;
		return (stat(Filename.c_str(), &buffer) == 0);
	}
}