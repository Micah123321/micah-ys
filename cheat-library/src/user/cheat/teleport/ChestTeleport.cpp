#include "pch-il2cpp.h"
#include "ChestTeleport.h"

#include <helpers.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/Chest.h>
#include <cheat/game/util.h>
#include <cheat/teleport/MapTeleport.h>

namespace cheat::feature
{

	ChestTeleport::ChestTeleport() : ItemTeleportBase(u8"���䴫��", u8"����"),
		NF(f_FilterChestLocked, u8"������", u8"���䴫��", true),
		NF(f_FilterChestInRock, u8"����ʯ�е�", u8"���䴫��", true),
		NF(f_FilterChestFrozen, u8"�����", u8"���䴫��", true),
		NF(f_FilterChestBramble, u8"������", u8"���䴫��", true),
		NF(f_FilterChestTrap, u8"����", u8"���䴫��", true),

		NF(f_FilterChestCommon, u8"������", u8"���䴫��", true),
		NF(f_FilterChestExquisite, u8"���µ�", u8"���䴫��", true),
		NF(f_FilterChestPrecious, u8"����", u8"���䴫��", true),
		NF(f_FilterChestLuxurious, u8"������", u8"���䴫��", true),
		NF(f_FilterChestRemarkable, u8"�Ƿ���", u8"���䴫��", true),

		NF(f_FilterChest, u8"����", u8"���䴫��", true),
		NF(f_FilterInvestigates, u8"����", u8"���䴫��", false),
		NF(f_FilterBookPage, u8"��ҳ", u8"���䴫��", false),
		NF(f_FilterBGM, u8"��Ƭ", u8"���䴫��", false),
		NF(f_FilterQuestInt, u8"̽��", u8"���䴫��", false),
		NF(f_FilterFloraChest, u8"������", u8"���䴫��", false),

		NF(f_FilterUnknown, u8"δ֪", u8"���䴫��", true)
	{ }


	void cheat::feature::ChestTeleport::DrawFilterOptions()
	{
		ConfigWidget(f_ShowInfo, u8"��ʾ����Ϣ������������ӵļ����Ϣ.");

		if (ImGui::TreeNode(u8"����"))
		{

			ImGui::Text(u8"�����͹�����");

			ConfigWidget(f_FilterChest, u8"�������Ӽ��͹�������");
			ConfigWidget(f_FilterInvestigates);
			ConfigWidget(f_FilterBookPage);
			ConfigWidget(f_FilterBGM);
			ConfigWidget(f_FilterQuestInt);
			ConfigWidget(f_FilterFloraChest);
			ConfigWidget(f_FilterUnknown, u8"����δ֪���⡣�����������濴����Щ��Ŀ��������Ǵ��ڵĻ�.");

			ImGui::Spacing();

			if (!f_FilterChest)
				ImGui::BeginDisabled();

			if (ImGui::BeginTable(u8"���ӹ���", 2, ImGuiTableFlags_NoBordersInBody))
			{
				ImGui::TableNextColumn();
				ImGui::Text(u8"���������");

				ImGui::TableNextColumn();
				ImGui::Text(u8"״̬������");

				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestCommon);

				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestLocked);


				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestExquisite);

				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestInRock);


				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestPrecious);

				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestFrozen);


				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestLuxurious);

				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestBramble);


				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestRemarkable);

				ImGui::TableNextColumn();
				ConfigWidget(f_FilterChestTrap);

				ImGui::EndTable();
			}

			if (!f_FilterChest)
				ImGui::EndDisabled();

			ImGui::TreePop();
		}
	}

	const FeatureGUIInfo& ChestTeleport::GetGUIInfo() const
	{
		static const FeatureGUIInfo info{ u8"���䴫��", u8"����", true };
		return info;
	}

	ChestTeleport& ChestTeleport::GetInstance()
	{
		static ChestTeleport instance;
		return instance;
	}

	bool ChestTeleport::IsValid(game::Entity* entity) const
	{
		if (!entity->isChest())
			return false;

		auto chest = reinterpret_cast<game::Chest*>(entity);
		auto filterResult = FilterChest(chest);
		return filterResult == ChestTeleport::FilterStatus::Valid ||
			(filterResult == ChestTeleport::FilterStatus::Unknown && f_FilterUnknown);
	}

	cheat::feature::ChestTeleport::FilterStatus ChestTeleport::FilterChest(game::Chest* entity) const
	{
		auto itemType = entity->itemType();
		switch (itemType)
		{
		case game::Chest::ItemType::Chest:
		{
			if (!f_FilterChest)
				return FilterStatus::Invalid;

			auto chestRarity = entity->chestRarity();
			if (chestRarity == game::Chest::ChestRarity::Unknown)
				return FilterStatus::Unknown;

			bool rarityValid = (chestRarity == game::Chest::ChestRarity::Common && f_FilterChestCommon) ||
				(chestRarity == game::Chest::ChestRarity::Exquisite && f_FilterChestExquisite) ||
				(chestRarity == game::Chest::ChestRarity::Precious && f_FilterChestPrecious) ||
				(chestRarity == game::Chest::ChestRarity::Luxurious && f_FilterChestLuxurious) ||
				(chestRarity == game::Chest::ChestRarity::Remarkable && f_FilterChestRemarkable);

			if (!rarityValid)
				return FilterStatus::Invalid;

			auto chestState = entity->chestState();
			if (chestState == game::Chest::ChestState::Invalid)
				return FilterStatus::Invalid;

			bool chestStateValid = chestState == game::Chest::ChestState::None ||
				(chestState == game::Chest::ChestState::Locked && f_FilterChestLocked) ||
				(chestState == game::Chest::ChestState::InRock && f_FilterChestInRock) ||
				(chestState == game::Chest::ChestState::Frozen && f_FilterChestFrozen) ||
				(chestState == game::Chest::ChestState::Bramble && f_FilterChestBramble) ||
				(chestState == game::Chest::ChestState::Trap && f_FilterChestTrap);

			if (!chestStateValid)
				return FilterStatus::Invalid;

			return FilterStatus::Valid;
		}
		case game::Chest::ItemType::Investigate:
			return f_FilterInvestigates ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::BookPage:
			return f_FilterBookPage ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::BGM:
			return f_FilterBGM ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::QuestInteract:
			return f_FilterQuestInt ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::Flora:
			return f_FilterFloraChest ? FilterStatus::Valid : FilterStatus::Invalid;
		case game::Chest::ItemType::None:
		default:
			return FilterStatus::Unknown;
		}

		return FilterStatus::Unknown;
	}

	void ChestTeleport::DrawItems()
	{
		DrawUnknowns();
		DrawChests();
	}

	bool ChestTeleport::NeedInfoDraw() const
	{
		return true;
	}

	void ChestTeleport::DrawInfo()
	{
		auto entity = game::FindNearestEntity(*this);
		auto chest = reinterpret_cast<game::Chest*>(entity);

		DrawEntityInfo(entity);
		if (entity == nullptr)
			return;
		ImGui::SameLine();

		ImGui::TextColored(chest->chestColor(), u8"%s", chest->minName().c_str());
	}

	void ChestTeleport::DrawChests()
	{
		if (!ImGui::TreeNode(u8"Items"))
			return;

		auto& manager = game::EntityManager::instance();
		auto entities = manager.entities(*this);

		ImGui::BeginTable(u8"�����б�", 2);
		for (auto& entity : entities)
		{
			ImGui::PushID(entity);
			auto chest = reinterpret_cast<game::Chest*>(entity);

			ImGui::TableNextColumn();
			if (chest->itemType() == game::Chest::ItemType::Chest)
			{
				ImGui::TextColored(chest->chestColor(), u8"%s [%s] [%s] �� %0.3fm",
					magic_enum::enum_name(chest->itemType()).data(),
					magic_enum::enum_name(chest->chestRarity()).data(),
					magic_enum::enum_name(chest->chestState()).data(),
					manager.avatar()->distance(entity));
			}
			else
			{
				ImGui::TextColored(chest->chestColor(), u8"%s �� %0.3fm", magic_enum::enum_name(chest->itemType()).data(),
					manager.avatar()->distance(entity));
			}

			ImGui::TableNextColumn();

			if (ImGui::Button(u8"����"))
			{
				auto& mapTeleport = MapTeleport::GetInstance();
				mapTeleport.TeleportTo(chest->absolutePosition());
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
		ImGui::TreePop();
	}

	static bool ChestUnknownFilter(game::Entity* entity)
	{
		if (!entity->isChest())
			return false;

		auto chest = reinterpret_cast<game::Chest*>(entity);
		auto& chestTp = ChestTeleport::GetInstance();
		return chestTp.FilterChest(chest) == ChestTeleport::FilterStatus::Unknown;
	}

	void ChestTeleport::DrawUnknowns()
	{
		auto& manager = game::EntityManager::instance();
		auto unknowns = manager.entities(ChestUnknownFilter);
		if (unknowns.empty())
			return;

		ImGui::TextColored(ImColor(255, 165, 0, 255), u8"�ۣ����ҵ��˲�֪���ı��䡣����ζ�����ֱ��仹û�й���.");
		TextURL(u8"Please contribute to issue on GitHub", u8"https://github.com/CallowBlack/genshin-cheat/issues/48", false, false);

		if (ImGui::Button(u8"Copy to clipboard"))
		{
			ImGui::LogToClipboard();

			ImGui::LogText(u8"Unknown Names:\n");

			for (auto& entity : unknowns)
				ImGui::LogText(u8"%s; Position: %s; Scene: %u\n", entity->name().c_str(),
					il2cppi_to_string(entity->relativePosition()).c_str(), game::GetCurrentPlayerSceneID());

			ImGui::LogFinish();
		}

		if (!ImGui::TreeNode(u8"Unknown Items"))
			return;

		ImGui::BeginTable(u8"UnknownsTable", 2);

		for (auto& entity : unknowns)
		{
			ImGui::PushID(entity);

			ImGui::TableNextColumn();
			ImGui::Text(u8"%s. Dist %0.3f", entity->name().c_str(), manager.avatar()->distance(entity));

			ImGui::TableNextColumn();
			if (ImGui::Button(u8"TP"))
			{
				auto& mapTeleport = MapTeleport::GetInstance();
				mapTeleport.TeleportTo(entity->absolutePosition());
			}

			ImGui::PopID();
		}
		ImGui::EndTable();
		ImGui::TreePop();
	}



}

