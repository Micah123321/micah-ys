#include "pch-il2cpp.h"
#include "AnimationChanger.h"

#include <helpers.h>
#include <cheat/events.h>
#include <misc/cpp/imgui_stdlib.h>
#include <cheat/game/EntityManager.h>

namespace cheat::feature
{
    static std::string animations[] = {
    //  All characters
    u8"SlipFaceWall",
    u8"SlipBackWall",
    u8"DropDown",
    u8"JumpOffWall",
    u8"Jump",
    u8"JumpForRun",
    u8"JumpForWalk",
    u8"Fly",
    u8"FlyStart",
    u8"JumpForSprint",
    u8"SwimIdle",
    u8"SwimMove",
    u8"SwimDash",
    u8"ClimbMove1",
    u8"ClimbIdle",
    u8"ClimbJump",
    u8"ClimbMove0",
    u8"FallToGroundRun",
    u8"FallOnGroundLit",
    u8"FallOnGround",
    u8"FallToGroundRunHard",
    u8"FallToGroundSprint",
    u8"Walk",
    u8"Run",
    u8"Standby",
    u8"RunToIdle",
    u8"RunToWalk",
    u8"WalkToIdle",
    u8"WalkToRun",
    u8"Sprint",
    u8"SprintToIdle",
    u8"SprintToRun",
    u8"ClimbDownToGround",
    u8"SprintBS",
    u8"ShowUp",
    u8"CrouchToStandby",
    u8"CrouchIdle",
    u8"CrouchRoll",
    u8"CrouchMove",
    u8"SkiffNormal",
    u8"Upstairs",
    u8"JumpUpWallForStandby",
    u8"JumpUpWallReady",
    u8"Standby2ClimbA",
    u8"SwimJump",
    u8"SwimJumpDrop",
    u8"SwimJumpToWater",
    u8"Standby2ClimbB",
    u8"CrouchDrop",
    u8"TurnDir",
    u8"StandbyWeapon",
    u8"StandbyPutaway",
    u8"StandbyPutawayOver",
    u8"Icespine_Out",
    u8"Icespine",
    u8"LiquidStrike_MoveStandby",
    u8"LiquidStrike_AS",
    u8"LiquidStrike_BS",
    u8"LiquidStrike_BS1",
    u8"LiquidStrike_Move",
    u8"LiquidStrike_Strike",
    u8"LiquidStrike_FatalStandby",
    u8"LiquidStrike_FatalMove",
    u8"LiquidStrike_AS_OnWater",
    u8"LiquidStrike_BS_0",
    u8"FrozenWindmill",
    u8"FrozenWindmill_AS",
    u8"Attack03",
    u8"Attack04",
    u8"Attack05",
    u8"Attack01",
    u8"Attack02",
    u8"ExtraAttack",
    u8"ExtraAttack_AS",
    u8"FallingAnthem_Loop",
    u8"FallingAnthem_AS_2",
    u8"FallingAnthem_BS_1",
    u8"FallingAnthem_BS_2",
    u8"FallingAnthem_AS_1",
    u8"FallingAnthem_Loop_Low",
    u8"SitBDown",
    u8"SitBLoop",
    u8"SitBUp",
    u8"SitDown",
    u8"SitLoop",
    u8"SitUp",
    u8"StandbyShow_01",
    u8"StandbyShow_02",
    u8"StandbyVoice",
    u8"Think01BS",
    u8"Think01Loop",
    u8"Think01AS",
    u8"Akimbo02BS",
    u8"Akimbo02Loop",
    u8"Akimbo02AS",
    u8"ChannelBS",
    u8"ChannelLoop",
    u8"ChannelAS",
    u8"PlayMusic_Lyre_AS",
    u8"PlayMusic_Lyre_BS",
    u8"PlayMusic_Lyre_Loop",
    u8"PlayMusic_Qin_BS",
    u8"PlayMusic_Qin_AS",
    u8"PlayMusic_Qin_Loop",
    u8"ActivitySkill_ElectricCoreFly",
    u8"Hit_H",
    u8"Hit_L",
    u8"Hit_Throw",
    u8"Hit_Throw_Ground",
    u8"Hit_ThrowAir",
    u8"Struggle",
    u8"NormalDie",
    u8"SwimDie",
    u8"HitGroundDie",
    u8"FallDie_AS",
    u8"FallDie",
    //  Main Character only
    u8"UziExplode_AS",
    u8"UziExplode_BS",
    u8"UziExplode_Charge_01",
    u8"UziExplode_Strike_02",
    u8"UziExplode_Charge_02",
    u8"UziExplode_Strike_01",
    u8"UziExplode_BS_1",
    u8"WindBreathe_AS",
    u8"WindBreathe",
    u8"Hogyoku_AS",
    u8"Hogyoku_BS",
    u8"Hogyoku",
    u8"Hogyoku_Charge",
    u8"Hogyoku_Charge_AS",
    u8"Hogyoku_Charge_2",
    u8"RockTide_AS",
    u8"RockTide",
    u8"CrouchThrowBS",
    u8"CrouchThrowLoop",
    u8"CrouchThrowAS",
    u8"FindCatThrowBS",
    u8"FindCatThrowLoop",
    u8"FindCatThrowAS",
    u8"Player_Electric_ElementalArt",
    u8"Player_Electric_ElementalArt_AS",
    u8"Player_Electric_ElementalBurst",
    u8"Player_Electric_ElementalBurst_AS",
    u8"PutHand01BS",
    u8"PutHand01Loop",
    u8"PutHand01AS",
    u8"Akimbo01BS",
    u8"Backrake01BS",
    u8"Forerake01BS",
    u8"StrikeChest01BS",
    u8"Akimbo01Loop",
    u8"Akimbo01AS",
    u8"Backrake01Loop",
    u8"Backrake01AS",
    u8"Forerake01Loop",
    u8"Forerake01AS",
    u8"StrikeChest01Loop",
    u8"StrikeChest01AS",
    u8"HoldHead01BS",
    u8"HoldHead01Loop",
    u8"HoldHead01AS",
    u8"Clap01",
    u8"Turn01_90LBS",
    u8"Turn01_90RBS",
    u8"Turn01_90LAS",
    u8"Turn01_90RAS",
    u8"Alert01BS",
    u8"Alert01Loop",
    u8"Alert01AS",
    u8"Fishing01_BS",
    u8"Fishing01Loop",
    u8"Fishing01AS",
    u8"Think01_BS",
    u8"Think01_Loop",
    u8"Think01_AS",
    u8"Channel01BS",
    u8"Channel01Loop",
    u8"Channel01AS",
    u8"Fishing_Battle_BS",
    u8"Fishing_Cast_AS",
    u8"Fishing_Cast_BS",
    u8"Fishing_Cast_Loop",
    u8"Fishing_Choose",
    u8"Fishing_Choose_Loop",
    u8"Fishing_End",
    u8"Fishing_Pull_01",
    u8"Fishing_Pull_02",
    u8"Fishing_Wait",
    u8"Fishing_Pull_Fail",
    u8"Bartender_MixingStandby",
    u8"Bartender_MixingStart",
    u8"Bartender_MixingToPour",
    u8"Bartender_Pour",
    u8"Bartender_PourFinish",
    u8"Bartender_PourStandby",
    u8"Bartender_AddLoop",
    u8"Bartender_PrepareStart",
    u8"Bartender_Standby",
    u8"Bartender_AddStandby",
    u8"Bartender_PrepareToStandby",
    u8"Bartender_StandbyFinish",
    u8"Blocking_BS",
    u8"Blocking_Loop",
    u8"Blocking_Back",
    u8"Blocking_Bounce",
    u8"Blocking_Hit",
    u8"Blocking_AS"
    };

    AnimationChanger::AnimationChanger() : Feature(),
        NF(f_Enabled, u8"动画改变", u8"视觉::动画改变", false),
        NF(f_Animation, u8"动画", u8"视觉::动画改变", u8"ExtraAttack"),
        NF(f_ApplyKey, u8"应用动画", u8"视觉::动画改变", Hotkey('Y')),
        NF(f_ResetKey, u8"重置动画", u8"视觉::动画改变", Hotkey('R'))
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(AnimationChanger::OnGameUpdate);
    }

    const FeatureGUIInfo& AnimationChanger::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"动画改变", u8"视觉", false };
        return info;
    }

    void AnimationChanger::DrawMain()
    {
        ImGui::BeginGroupPanel(u8"动画改变");
        {
            ConfigWidget(f_Enabled, u8"改变活动角色的动画。并不是所有的动画都适用于除主要角色之外的每个角色.");
            if (f_Enabled)
            {
                if (ImGui::BeginCombo(u8"动画", f_Animation.value().c_str()))
                {
                    for (auto &animation : animations)
                    {
                        bool is_selected = (f_Animation.value().c_str() == animation);
                        if (ImGui::Selectable(animation.c_str(), is_selected))
                            f_Animation.value() = animation;

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ConfigWidget(u8"应用按钮", f_ApplyKey, true);
                ConfigWidget(u8"复位", f_ResetKey, true);
            }
        }
        ImGui::EndGroupPanel();
    }

    bool AnimationChanger::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void AnimationChanger::DrawStatus()
    {
        ImGui::Text(u8"动画改变");
    }

    AnimationChanger& AnimationChanger::GetInstance()
    {
        static AnimationChanger instance;
        return instance;
    }

    void AnimationChanger::OnGameUpdate()
    {
        if (!f_Enabled)
            return;

        // Taiga#5555: Maybe need to add separate option to change delay value if user feels like it's too fast or slow.
        UPDATE_DELAY(400);

        auto& manager = game::EntityManager::instance();
        auto avatar = manager.avatar();
        if (avatar->animator() == nullptr)
            return;

        if (f_ApplyKey.value().IsPressed())
            app::Animator_Play(avatar->animator(), string_to_il2cppi(f_Animation.value().c_str()), 0, 0, nullptr);

        if (f_ResetKey.value().IsPressed())
            app::Animator_Rebind(avatar->animator(), nullptr);
    }
}
