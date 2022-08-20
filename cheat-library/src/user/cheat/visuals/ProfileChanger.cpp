#include "pch-il2cpp.h"
#include "ProfileChanger.h"

#include <helpers.h>
#include <cheat/events.h>
#include <misc/cpp/imgui_stdlib.h>
#include <fstream>
 

namespace cheat::feature
{
    namespace GameObject {
        app::GameObject* WaterMark = nullptr;
    }
    
    namespace Components {
        app::Component_1* WaterMark = nullptr;

        app::Texture2D* CardTexture = nullptr;
        app::Texture2D* AvatarTexture = nullptr;
        app::Sprite* CardSprite = nullptr;
        app::Sprite* AvatarSprite = nullptr;
        app::Rect RectCard;
        app::Rect RectAvatar;
    }

    // Profile Page
    app::Button_1* ProfilePage(app::MonoInLevelPlayerProfilePage* __this, MethodInfo* method);

    // Edit Player Info Page 
    static void ProfileEditPage(app::MonoFriendInformationDialog* __this, app::Sprite* value, MethodInfo* method);

    ProfileChanger::ProfileChanger() : Feature(),
        NF(f_Enabled, u8"�Զ��������ļ�", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_UID, u8"UID", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_UIDWaterMarkPrefix, u8"UIDˮ��ǰ׺", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_UIDsize, u8"UID �ߴ�", u8"�Ӿ�::�����ļ��ı�", 14),
        NF(f_UIDpos_x, u8"UID  X ����", u8"�Ӿ�::�����ļ��ı�", static_cast<float>(app::Screen_get_width(nullptr)* 0.96875)),
        NF(f_UIDpos_y, u8"UID  Y ����", u8"�Ӿ�::�����ļ��ı�", 0),
        NF(f_NickName, u8"�ǳ�", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_Level, u8"����", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_Exp, u8"����", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_CurExp, u8"��ǰ����", u8"�Ӿ�::�����ļ��ı�", 1),
        NF(f_MaxExp, u8"�����", u8"�Ӿ�::�����ļ��ı�", 1),
        NF(f_ExpBar, u8"������", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_ExpBarValue, u8"��������ֵ", u8"�Ӿ�::�����ļ��ı�", 20.0f),
        NF(f_WorldLevel, u8"����ȼ�", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_Avatar, u8"ͷ��ͼƬ", u8"�Ӿ�::�����ļ��ı�", false),
        NF(f_Card, u8"��ƬͼƬ", u8"�Ӿ�::�����ļ��ı�", false),
        toBeUpdate(), nextUpdate(0)
    {
        HookManager::install(app::ProfilePage, ProfilePage);
        HookManager::install(app::ProfileEditPage, ProfileEditPage);
        events::GameUpdateEvent += MY_METHOD_HANDLER(ProfileChanger::OnGameUpdate);
    }

    const FeatureGUIInfo& ProfileChanger::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"�Զ����ɫ����", u8"�Ӿ�", true };
        return info;
    }

    void ProfileChanger::DrawMain()
    {
        ConfigWidget(f_Enabled, u8"�Զ����ɫ����.");
        ConfigWidget(f_UID, u8"����uid.");
        ConfigWidget(u8"׷�� \"UID:\" ǰ��ˮӡ", f_UIDWaterMarkPrefix);
        ConfigWidget(u8"UID �ߴ�", f_UIDsize, 0.1, 1, 500.0, u8"Set UID size");
        ConfigWidget(u8"UID Pos X", f_UIDpos_x, 1.f, 1.f, static_cast<float>(app::Screen_get_width(nullptr)), u8"Set UID position X");
        ConfigWidget(u8"UID Pos y", f_UIDpos_y, 1.f, 0, static_cast<float>(app::Screen_get_height(nullptr)), u8"Set UID position y");
        ConfigWidget(f_NickName, u8"�����ǳ�.");
        ConfigWidget(f_Level, u8"�ı�ȼ�.");
        ConfigWidget(f_Exp, u8"�ı侭��.");
        if (f_Exp) {
            ConfigWidget(u8"CurExp", f_CurExp, 1, 2, 100000, u8"Changes the ExpBar visually.");
            ConfigWidget(u8"MaxExp", f_MaxExp, 1, 2, 100000, u8"Changes the ExpBar visually.");
            ConfigWidget(f_ExpBar, u8"Changes the ExpBar visually.");
            if (f_ExpBar)
                ConfigWidget(u8"ExpBarValue", f_ExpBarValue, 1, 2, 100, u8"Changes the ExpBar visually.");
        }   
        ConfigWidget(f_WorldLevel, u8"�ı����缶��.");
        ConfigWidget(f_Avatar, u8"�ı�ͷ��ͼƬ.\n" \
            u8"Note the size of the picture must be: 256x256.\n" \
            u8"Example path: C:\\Avatars.png");

        ConfigWidget(f_Card, u8"�ı俨Ƭ.\n" \
            u8"Note the size of the picture must be: 840x400.\n" \
            u8"Example path: C:\\Avatars.png");
    }

    bool ProfileChanger::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void ProfileChanger::DrawStatus()
    {
        ImGui::Text(u8"�Զ����ɫ����");
    }

    ProfileChanger& ProfileChanger::GetInstance()
    {
        static ProfileChanger instance;
        return instance;
    }

    void ProfileChanger::OnGameUpdate()
    {
        if (!f_Enabled || !f_UID)
            return;

        auto currentTime = util::GetCurrentTimeMillisec();
        if (currentTime < nextUpdate)
            return;

        if (f_UID) {
            if (GameObject::WaterMark == nullptr)
                GameObject::WaterMark = app::GameObject_Find(string_to_il2cppi(u8"/BetaWatermarkCanvas(Clone)/Panel/TxtUID"), nullptr);
            
            if (GameObject::WaterMark != nullptr && Components::WaterMark == nullptr)
                Components::WaterMark = app::GameObject_GetComponentByName(GameObject::WaterMark, string_to_il2cppi(u8"Text"), nullptr);

            if (Components::WaterMark != nullptr)
                app::Text_set_text(reinterpret_cast<app::Text*>(Components::WaterMark), string_to_il2cppi(f_UID.value().value.empty() ? u8"" : std::string((f_UIDWaterMarkPrefix ? u8"UID: u8" : u8"") + f_UID.value().value)), nullptr);

            auto transformWatermark = app::GameObject_get_transform(GameObject::WaterMark, nullptr);
            if (transformWatermark)
            {
                app::Vector3 uidPos = { f_UIDpos_x, f_UIDpos_y, 0 };
                app::Text_set_alignment(reinterpret_cast<app::Text*>(Components::WaterMark), app::TextAnchor__Enum::LowerRight, nullptr);
                app::Text_set_horizontalOverflow(reinterpret_cast<app::Text*>(Components::WaterMark), app::HorizontalWrapMode__Enum::Overflow, nullptr);
                app::Text_set_verticalOverflow(reinterpret_cast<app::Text*>(Components::WaterMark), app::VerticalWrapMode__Enum::Overflow, nullptr);
                app::Text_set_resizeTextForBestFit(reinterpret_cast<app::Text*>(Components::WaterMark), false, nullptr);
                app::Text_set_fontSize(reinterpret_cast<app::Text*>(Components::WaterMark), f_UIDsize, nullptr);
                app::Transform_set_position(transformWatermark, uidPos, nullptr);
            }
        }
           
        nextUpdate = currentTime + (int)f_DelayUpdate;
    }

    bool ProfileChanger::CheckFile(const std::string& Filename) {
        struct stat buffer;
        return (stat(Filename.c_str(), &buffer) == 0);
    }

    app::Button_1* ProfilePage(app::MonoInLevelPlayerProfilePage* __this, MethodInfo* method)
    {
        auto& profile = ProfileChanger::GetInstance();

        if (profile.f_Enabled) {
           
            if (profile.f_UID)
                app::Text_set_text(__this->fields._playerID, string_to_il2cppi(profile.f_UID), nullptr);
                
            if (profile.f_Level)
                app::Text_set_text(__this->fields._playerLv, string_to_il2cppi(profile.f_Level), nullptr);
 
            if (profile.f_Exp) {
                std::string CurExpStr = std::to_string(profile.f_CurExp);
                std::string MaxExpStr = std::to_string(profile.f_MaxExp);
                app::Text_set_text(__this->fields._playerExp, string_to_il2cppi(CurExpStr + u8"/" + MaxExpStr), nullptr);

                if (profile.f_ExpBar) 
                {
                    app::Slider_1_set_minValue(__this->fields._playerExpSlider, 1, nullptr);
                    app::Slider_1_set_maxValue(__this->fields._playerExpSlider, 100, nullptr);
                    app::Slider_1_set_value(__this->fields._playerExpSlider, profile.f_ExpBarValue, nullptr);
                }
            }

            if (profile.f_WorldLevel)
                app::Text_set_text(__this->fields._playerWorldLv, string_to_il2cppi(profile.f_WorldLevel), nullptr);

            if (profile.f_NickName){
                auto playerModule = GET_SINGLETON(MoleMole_PlayerModule);
                if (playerModule != nullptr && playerModule->fields._accountData_k__BackingField != nullptr) {
                    auto& accountData = playerModule->fields._accountData_k__BackingField->fields;
                    accountData.nickName = string_to_il2cppi(profile.f_NickName);
                }
            }
          
            // Card Name png size 840x400
            if (profile.f_Card){
                if (profile.CheckFile(profile.f_Card)) {
                    Components::CardTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(profile.f_Card), 100, false, false, false, nullptr);           
                    // If you don't do this check, then the UI will break after teleportation, I'm just too lazy to set up Rect manually
                    if (Components::RectCard.m_Width == 0) 
                        Components::RectCard = app::Sprite_get_rect(__this->fields._nameCardPic->fields.m_Sprite, nullptr);
                    app::Vector2 Vec2 = { 100, 100 };
                    Components::CardSprite = app::Sprite_Create(Components::CardTexture, Components::RectCard, Vec2, 1, nullptr);
                    __this->fields._nameCardPic->fields.m_OverrideSprite = Components::CardSprite;  
                }
                else {
                    std::cout << u8"Card Image: \n" << u8"not found" << std::endl;
                }
            }
            // Avatar png size 256x256
            if (profile.f_Avatar) {
                if (profile.CheckFile(profile.f_Avatar)) {
                    Components::AvatarTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(profile.f_Avatar), 100, false, false, false, nullptr);
                    // If you don't do this check, then the UI will break after teleportation, I'm just too lazy to set up Rect manually
                    if (Components::RectAvatar.m_Width == 0)
                        Components::RectAvatar = app::Sprite_get_rect(__this->fields.playerIconImage->fields.m_Sprite, nullptr);
                    app::Vector2 Vec2Avatar = { 128, 128 };
                    Components::AvatarSprite = app::Sprite_Create(Components::AvatarTexture, Components::RectAvatar, Vec2Avatar, 1, nullptr);
                    __this->fields.playerIconImage->fields.m_OverrideSprite = Components::AvatarSprite;
                }
                else {
                    std::cout << u8"Avatar Image: \n" << u8"not found" << std::endl;
                }
            }
        }
        return CALL_ORIGIN(ProfilePage, __this, method);
    }

    static void ProfileEditPage(app::MonoFriendInformationDialog* __this, app::Sprite* value, MethodInfo* method) {
        auto& profile = ProfileChanger::GetInstance();
        if (profile.f_Enabled) {
            if (profile.f_UID)
                __this->fields._playerUID->fields.m_Text = string_to_il2cppi(profile.f_UID);
 
            if (profile.f_Level)
                __this->fields._playerLevel->fields.m_Text = string_to_il2cppi(profile.f_Level);

            if (profile.f_WorldLevel)
                __this->fields._worldLevel->fields.m_Text = string_to_il2cppi(profile.f_WorldLevel);

            // Card Name png size 840x400

            if (profile.f_Card) {
                if (profile.CheckFile(profile.f_Card)) {
                    Components::CardTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(profile.f_Card), 100, false, false, false, nullptr);
                    // If you don't do this check, then the UI will break after teleportation, I'm just too lazy to set up Rect manually
                    if (Components::RectCard.m_Width == 0)
                        Components::RectCard = app::Sprite_get_rect(__this->fields._cardImg->fields.m_Sprite, nullptr);
                    app::Vector2 Vec2 = { 100, 100 };
                    Components::CardSprite = app::Sprite_Create(Components::CardTexture, Components::RectCard, Vec2, 1, nullptr);
                    __this->fields._cardImg->fields.m_OverrideSprite = Components::CardSprite;
                }
                else {
                    std::cout << u8"Card Image: \n" << u8"not found" << std::endl;
                }
            }

            // Avatar png size 256x256
            if (profile.f_Avatar) {
                if (profile.CheckFile(profile.f_Avatar)) {
                    Components::AvatarTexture = app::NativeGallery_LoadImageAtPath(string_to_il2cppi(profile.f_Avatar), 100, false, false, false, nullptr);
                    if (Components::RectAvatar.m_Width == 0)
                        Components::RectAvatar = app::Sprite_get_rect(__this->fields._icon->fields.m_Sprite, nullptr);
                    app::Vector2 Vec2Avatar = { 128, 128 };
                    Components::AvatarSprite = app::Sprite_Create(Components::AvatarTexture, Components::RectAvatar, Vec2Avatar, 1, nullptr);
                    __this->fields._icon->fields.m_OverrideSprite = Components::AvatarSprite;
                }
                else {
                    std::cout << u8"Card Image: \n" << u8"not found" << std::endl;
                }
            }      
        }

        return CALL_ORIGIN(ProfileEditPage, __this, value, method);
    }
}