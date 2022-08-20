#include "pch-il2cpp.h"
#include "MobVacuum.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/util.h>

namespace cheat::feature 
{
    MobVacuum::MobVacuum() : Feature(),
        NF(f_Enabled,        u8"ԭ��ģʽ", u8"ԭ��ģʽ", false),
        NF(f_IncludeMonsters, u8"��������", u8"ԭ��ģʽ", true),
        NF(f_MonsterCommon, u8"������", u8"ԭ��ģʽ", true),
        NF(f_MonsterElites, u8"��Ӣ", u8"ԭ��ģʽ", true),
        NF(f_MonsterBosses, u8"Boss", u8"ԭ��ģʽ", true),
        NF(f_IncludeAnimals, u8"��������", u8"ԭ��ģʽ", true),
        NF(f_AnimalDrop, u8"������", u8"ԭ��ģʽ", true),
        NF(f_AnimalPickUp, u8"��ȡ", u8"ԭ��ģʽ", true),
        NF(f_AnimalNPC, u8"NPCs", u8"ԭ��ģʽ", true),
        NF(f_Speed,      u8"�ٶ�",         u8"ԭ��ģʽ", 2.5f),
        NF(f_Distance,   u8"����",      u8"ԭ��ģʽ", 1.5f),
        NF(f_Radius,     u8"�뾶",        u8"ԭ��ģʽ", 10.0f),
        NF(f_OnlyTarget, u8"ֻ��Ŀ��", u8"ԭ��ģʽ", true),
        NF(f_Instantly,  u8"����",     u8"ԭ��ģʽ", false)
    {
        events::GameUpdateEvent += MY_METHOD_HANDLER(MobVacuum::OnGameUpdate);
        events::MoveSyncEvent += MY_METHOD_HANDLER(MobVacuum::OnMoveSync);
    }

    const FeatureGUIInfo& MobVacuum::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ u8"ԭ��ģʽ", u8"����", true };
        return info;
    }

    void MobVacuum::DrawMain()
    {
        ConfigWidget(u8"����", f_Enabled, u8"����ԭ��ģʽ.\n" \
            u8"��ָ���뾶�ڵ����ｫ�ƶ������ǰ��ָ���ľ���.");

        bool filtersChanged = false;
        ImGui::BeginGroupPanel(u8"����");
        {
            filtersChanged |= ConfigWidget(f_IncludeMonsters, u8"������м������.");
            filtersChanged |= ConfigWidget(f_MonsterCommon, u8"��ͨ�ĵ���."); ImGui::SameLine();
            filtersChanged |= ConfigWidget(f_MonsterElites, u8"��Ӣ��."); ImGui::SameLine();
            filtersChanged |= ConfigWidget(f_MonsterBosses, u8"����boss.");
        }
        ImGui::EndGroupPanel();
        
        ImGui::BeginGroupPanel(u8"����");
        {
            filtersChanged |= ConfigWidget(f_IncludeAnimals, u8"�Ѷ�����������.");
            filtersChanged |= ConfigWidget(f_AnimalDrop, u8"�ռ�ǰ��Ҫɱ���Ķ���."); ImGui::SameLine();
            filtersChanged |= ConfigWidget(f_AnimalPickUp, u8"����������ռ�����."); ImGui::SameLine();
            filtersChanged |= ConfigWidget(f_AnimalNPC, u8"����û����ѧ.");
        }
        ImGui::EndGroupPanel();

        if (filtersChanged)
            UpdateFilters();

    	ConfigWidget(u8"��ʱ���", f_Instantly, u8"���ʵ������.");
        ConfigWidget(u8"ֻ�е���/������Ϊ", f_OnlyTarget, u8"������ã����ֻ��Ӱ������ΪĿ��Ĺ������Ӱ�춯��.");
        ConfigWidget(u8"�ٶ�", f_Speed, 0.1f, 1.0f, 15.0f, u8"�������ʱ��ա�û�б���飬��ͽ����ָ�����ٶ����.");
        ConfigWidget(u8"�뾶 (m)", f_Radius, 0.1f, 5.0f, 150.0f, u8"��հ뾶.");
        ConfigWidget(u8"���� (m)", f_Distance, 0.1f, 0.5f, 10.0f, u8"��������֮��ľ���.");
    }

    bool MobVacuum::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void MobVacuum::DrawStatus() 
    { 
        ImGui::Text(u8"ԭ��ģʽ [%s]\n[%s|%.01fm|%.01fm|%s]", 
            f_IncludeMonsters && f_IncludeAnimals ? u8"����" : f_IncludeMonsters ? u8"��" : f_IncludeAnimals ? u8"����" : u8"��",
            f_Instantly ? u8"����" : fmt::format(u8"����|{:.1f}", f_Speed.value()).c_str(),
            f_Radius.value(),
            f_Distance.value(),
            f_OnlyTarget ? u8"�ж�" : u8"����"
        );
    }

    MobVacuum& MobVacuum::GetInstance()
    {
        static MobVacuum instance;
        return instance;
    }

    // Combines selected vacuum filters.
    void MobVacuum::UpdateFilters() {
        
        m_Filters.clear();

        if (f_IncludeMonsters) {
            if (f_MonsterCommon) m_Filters.push_back(&game::filters::combined::MonsterCommon);
            if (f_MonsterElites) m_Filters.push_back(&game::filters::combined::MonsterElites);
            if (f_MonsterBosses) m_Filters.push_back(&game::filters::combined::MonsterBosses);
        }

        if (f_IncludeAnimals) {
            if (f_AnimalDrop) m_Filters.push_back(&game::filters::combined::AnimalDrop);
            if (f_AnimalPickUp) m_Filters.push_back(&game::filters::combined::AnimalPickUp);
            if (f_AnimalNPC) m_Filters.push_back(&game::filters::combined::AnimalNPC);
        }
    }

    // Check if entity valid for mob vacuum.
    bool MobVacuum::IsEntityForVac(game::Entity* entity)
    {
        if (m_Filters.empty())
            return false;

        bool entityValid = std::any_of(m_Filters.cbegin(), m_Filters.cend(), 
            [entity](const game::IEntityFilter* filter) { return filter->IsValid(entity); });
        if (!entityValid)
            return false;

        auto& manager = game::EntityManager::instance();
        if (f_OnlyTarget && game::filters::combined::Monsters.IsValid(entity))
        {
            auto monsterCombat = entity->combat();
            if (monsterCombat == nullptr || monsterCombat->fields._attackTarget.runtimeID != manager.avatar()->runtimeID())
                return false;
        }

		auto distance = manager.avatar()->distance(entity);
        return distance <= f_Radius;
    }

    // Calculate mob vacuum target position.
    app::Vector3 MobVacuum::CalcMobVacTargetPos()
    {
        auto& manager = game::EntityManager::instance();
        auto avatarEntity = manager.avatar();
        if (avatarEntity == nullptr)
            return {};

        return avatarEntity->relativePosition() + avatarEntity->forward() * f_Distance;
    }

    // Mob vacuum update function.
    // Changes position of monster, if mob vacuum enabled.
    void MobVacuum::OnGameUpdate()
    {
        static auto positions = new std::map<uint32_t, app::Vector3>();

        if (!f_Enabled)
            return;

        app::Vector3 targetPos = CalcMobVacTargetPos();
        if (IsVectorZero(targetPos))
            return;

        UpdateFilters();
        if (!f_IncludeMonsters && !f_IncludeAnimals)
            return;

        if (m_Filters.empty())
            return;

        auto& manager = game::EntityManager::instance();
        auto newPositions = new std::map<uint32_t, app::Vector3>();
        for (const auto& entity : manager.entities())
        {
            if (!IsEntityForVac(entity))
                continue;

            if (f_Instantly)
            {
                entity->setRelativePosition(targetPos);
                continue;
            }

            uint32_t entityId = entity->runtimeID();
            app::Vector3 entityRelPos = positions->count(entityId) ? (*positions)[entityId] : entity->relativePosition();
            app::Vector3 newPosition = {};
            if (app::Vector3_Distance(entityRelPos, targetPos, nullptr) < 0.1)
            {
                newPosition = targetPos;
            }
            else
            {
                app::Vector3 dir = GetVectorDirection(entityRelPos, targetPos);
                float deltaTime = app::Time_get_deltaTime(nullptr);
                newPosition = entityRelPos + dir * f_Speed * deltaTime;
            }

            (*newPositions)[entityId] = newPosition;
            entity->setRelativePosition(newPosition);
        }

        delete positions;
        positions = newPositions;
    }

    // Mob vacuum sync packet replace.
    // Replacing move sync speed and motion state.
    //   Callow: I think it is more safe method, 
    //           because for server monster don't change position instantly.
    void MobVacuum::OnMoveSync(uint32_t entityId, app::MotionInfo* syncInfo)
    {
        if (!f_Enabled || f_Instantly)
            return;

        auto& manager = game::EntityManager::instance();
        auto entity = manager.entity(entityId);
        if (!IsEntityForVac(entity))
            return;

        app::Vector3 targetPos = CalcMobVacTargetPos();
        app::Vector3 entityPos = entity->relativePosition();
        if (app::Vector3_Distance(targetPos, entityPos, nullptr) < 0.2)
            return;

        app::Vector3 dir = GetVectorDirection(targetPos, entityPos);
        app::Vector3 scaledDir = dir * f_Speed;

        syncInfo->fields.speed_->fields.x = scaledDir.x;
        syncInfo->fields.speed_->fields.y = scaledDir.y;
        syncInfo->fields.speed_->fields.z = scaledDir.z;

        switch (syncInfo->fields.motionState)
        {
        case app::MotionState__Enum::MotionStandby:
        case app::MotionState__Enum::MotionStandbyMove:
        case app::MotionState__Enum::MotionWalk:
        case app::MotionState__Enum::MotionDangerDash:
            syncInfo->fields.motionState = app::MotionState__Enum::MotionRun;
        }
    }
}

