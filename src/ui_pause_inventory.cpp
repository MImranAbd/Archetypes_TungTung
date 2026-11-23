#include "ui_pause_internal.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>
#include <string>
#include <vector>

using namespace UiInternal;

namespace PauseUi
{
    void DrawItemPage(const Party& party, const Inventory& inventory,
        const PauseMenuState& menu)
    {
        DrawBanner(PausePage::Items, party);

        const ItemCategory category = static_cast<ItemCategory>(menu.itemCategory);

        if (menu.itemTargeting)
        {
            std::vector<const ItemStack*> items = ItemStacksInCategory(inventory, category);
            const ItemStack* selectedStack = nullptr;

            if (!items.empty() && menu.itemIndex >= 0 &&
                menu.itemIndex < static_cast<int>(items.size()))
                selectedStack = items[menu.itemIndex];

            const bool allTarget = selectedStack != nullptr &&
                ItemTargetsAll(selectedStack->type);

            DrawPartyList(party, 40, 200, 430, -1, true,
                menu.targetIndex, allTarget);

            const Rectangle panel = { 520.0f, 190.0f, 1360.0f, 700.0f };
            DrawPaperPanel(panel);

            UiDrawText("CHOOSE AN ALLY", (int)panel.x + 40, (int)panel.y + 34, 36,
                Theme::Ink, true);

            if (selectedStack != nullptr)
            {
                UiDrawText(ItemName(selectedStack->type),
                    (int)panel.x + 40, (int)panel.y + 96, 26, Theme::Ink, true);
                UiDrawText(TextFormat("Restores %d HP", ItemHeal(selectedStack->type)),
                    (int)panel.x + 40, (int)panel.y + 132, 19, Theme::Muted);

                if (allTarget)
                {
                    UiDrawText("ALL ALLIES", (int)panel.x + 40, (int)panel.y + 210,
                        48, Theme::Gold, true);
                }
                else
                {
                    const int target = menu.targetIndex;
                    const Character& member = party.Members()[target];

                    UiDrawText(member.name, (int)panel.x + 40, (int)panel.y + 200, 48,
                        Theme::Gold, true);
                    UiDrawText(TextFormat("HP  %d / %d", member.hp, member.maxHp),
                        (int)panel.x + 40, (int)panel.y + 270, 24, Theme::Ink);

                    DrawBar(panel.x + 40.0f, panel.y + 310.0f, 400.0f, 12.0f,
                        (member.maxHp > 0) ? (float)member.hp / member.maxHp : 0.0f,
                        Theme::Green);
                }
            }

            UiDrawText(menu.message.c_str(), (int)panel.x + 40, (int)panel.y + 640,
                19, Theme::Gold, true);

            DrawHint("W/S  ALLY      J / ENTER  USE      K / ESC  CANCEL");
            return;
        }

        DrawPartyList(party, 40, 200, 430, -1);

        const Rectangle panel = { 520.0f, 190.0f, 1360.0f, 700.0f };
        DrawPaperPanel(panel);

        const char* categoryNames[4] = { "HEALING", "WEAPON", "ATTACK", "OTHER" };
        int tabX = (int)panel.x + 30;

        for (int i = 0; i < 4; i++)
        {
            const ItemCategory tabCategory = static_cast<ItemCategory>(i);
            const bool selected = (i == menu.itemCategory);
            const Rectangle tab = { (float)tabX, panel.y + 20.0f, 190.0f, 44.0f };
            const Color color = ItemCategoryColor(tabCategory);

            if (selected)
            {
                DrawRectangleRec(tab, color);
                UiDrawText(categoryNames[i], tabX + 16, (int)tab.y + 9, 20,
                    WHITE, true);
            }
            else
            {
                DrawRectangleLinesEx(tab, 1.0f, Fade(color, 0.75f));
                UiDrawText(categoryNames[i], tabX + 16, (int)tab.y + 9, 20,
                    color, false);
            }

            tabX += 202;
        }

        DrawLine((int)panel.x + 30, (int)panel.y + 82,
            (int)panel.x + (int)panel.width - 30, (int)panel.y + 82,
            Fade(Theme::Ink, 0.25f));

        std::vector<const ItemStack*> items = ItemStacksInCategory(inventory, category);
        int selected = menu.itemIndex;

        if (items.empty())
            selected = 0;
        else if (selected < 0 || selected >= static_cast<int>(items.size()))
            selected = 0;

        int y = (int)panel.y + 106;

        if (items.empty())
        {
            UiDrawText("EMPTY", (int)panel.x + 40, y + 10, 22, Theme::Muted);
        }
        else
        {
            for (int i = 0; i < static_cast<int>(items.size()); i++)
            {
                const ItemStack& stack = *items[i];
                const Rectangle row = {
                    panel.x + 26.0f,
                    (float)y - 6.0f,
                    panel.width - 52.0f,
                    52.0f
                };

                if (i == selected)
                {
                    DrawRectangleRec(row, Fade(Theme::Gold, 0.20f));
                    DrawRectangleLinesEx(row, 1.0f, Theme::Gold);
                }

                DrawRectangle((int)row.x + 10, (int)row.y + 14, 12, 24,
                    ItemCategoryColor(ItemCategoryOf(stack.type)));

                UiDrawText(ItemName(stack.type), (int)row.x + 44, y + 4, 24,
                    Theme::Ink, true);
                UiDrawText(TextFormat("x%d", stack.count), (int)row.x + 330, y + 8, 22,
                    Theme::Gold, true);
                UiDrawText(TextFormat("Restores %d HP", ItemHeal(stack.type)),
                    (int)row.x + 440, y + 10, 17, Theme::Muted);

                y += 58;
            }
        }

        if (!menu.message.empty())
            UiDrawText(menu.message.c_str(), (int)panel.x + 30, (int)panel.y + 640,
                19, Theme::Gold, true);

        DrawHint("A/D  CATEGORY      W/S  ITEM      J / ENTER  CHOOSE      K / ESC  BACK");
    }

    void DrawEquipPage(const Party& party, const Inventory& inventory,
        const PauseMenuState& menu)
    {
        DrawBanner(PausePage::Equip, party);

        const int memberIndex = menu.memberIndex;
        const Character& character = party.Members()[memberIndex];

        DrawPartyList(party, 40, 200, 430, memberIndex);

        const Rectangle panel = { 520.0f, 190.0f, 1360.0f, 700.0f };
        DrawPaperPanel(panel);

        const int step = menu.equipStep;
        const char* stepNames[3] = { "1  CHARACTER", "2  SLOT", "3  EQUIPMENT" };
        int stepX = (int)panel.x + 30;

        for (int i = 0; i < 3; i++)
        {
            const bool active = (i == step);
            const bool done = (i < step);
            const Color color = active ? Theme::Ink : (done ? Theme::Gold : Theme::Muted);
            UiDrawText(stepNames[i], stepX, (int)panel.y + 20, 18,
                color, active);
            stepX += 200;
        }

        DrawLine((int)panel.x + 30, (int)panel.y + 56,
            (int)panel.x + (int)panel.width - 30, (int)panel.y + 56,
            Fade(Theme::Ink, 0.25f));

        if (step == 0)
        {
            UiDrawText("CHOOSE A CHARACTER", (int)panel.x + 30, (int)panel.y + 88, 32,
                Theme::Ink, true);
            UiDrawText(character.name, (int)panel.x + 30, (int)panel.y + 146, 42,
                Theme::Gold, true);
            UiDrawText(TextFormat("LV %d      HP %d / %d", character.level,
                character.hp, character.maxHp),
                (int)panel.x + 30, (int)panel.y + 208, 20, Theme::Muted);

            UiDrawText("CURRENT EQUIPMENT", (int)panel.x + 30, (int)panel.y + 280, 20,
                Theme::Muted, true);
            UiDrawText(TextFormat("WEAPON       %s",
                GetEquipment(character.weapon).name),
                (int)panel.x + 36, (int)panel.y + 316, 20, Theme::Ink);
            UiDrawText(TextFormat("ARMOR        %s",
                GetEquipment(character.armor).name),
                (int)panel.x + 36, (int)panel.y + 348, 20, Theme::Ink);
            UiDrawText(TextFormat("ACCESSORY    %s",
                GetEquipment(character.accessory).name),
                (int)panel.x + 36, (int)panel.y + 380, 20, Theme::Ink);
        }
        else if (step == 1)
        {
            const char* slotNames[3] = { "WEAPON", "ARMOR", "ACCESSORY" };
            const EquipmentId ids[3] = {
                character.weapon, character.armor, character.accessory
            };

            UiDrawText("CHOOSE A SLOT", (int)panel.x + 30, (int)panel.y + 88, 32,
                Theme::Ink, true);

            int y = (int)panel.y + 150;

            for (int i = 0; i < 3; i++)
            {
                const Rectangle row = {
                    panel.x + 26.0f, (float)y, panel.width - 52.0f, 62.0f
                };

                if (i == menu.equipSlot)
                {
                    DrawRectangleRec(row, Fade(Theme::Gold, 0.20f));
                    DrawRectangleLinesEx(row, 2.0f, Theme::Gold);
                }

                UiDrawText(TextFormat("%d", i + 1), (int)row.x + 16, y + 16, 24,
                    Theme::Gold, true);
                UiDrawText(slotNames[i], (int)row.x + 60, y + 16, 24, Theme::Ink, true);
                UiDrawText(GetEquipment(ids[i]).name, (int)row.x + 330, y + 18, 22,
                    Theme::Muted);

                y += 76;
            }
        }
        else
        {
            const EquipSlot slot = static_cast<EquipSlot>(menu.equipSlot);
            std::vector<EquipmentId> options = EquipmentOptions(character, slot,
                inventory);

            EquipmentId current = EquipmentId::None;
            if (slot == EquipSlot::Weapon) current = character.weapon;
            else if (slot == EquipSlot::Armor) current = character.armor;
            else current = character.accessory;

            UiDrawText("CHOOSE EQUIPMENT", (int)panel.x + 30, (int)panel.y + 88, 32,
                Theme::Ink, true);
            UiDrawText(EquipSlotName(slot), (int)panel.x + 30, (int)panel.y + 130, 20,
                Theme::Muted, true);

            int y = (int)panel.y + 168;

            for (int i = 0; i < static_cast<int>(options.size()); i++)
            {
                const EquipmentId id = options[i];
                const EquipmentDef& def = GetEquipment(id);
                const Rectangle row = {
                    panel.x + 26.0f, (float)y - 4.0f, panel.width - 52.0f, 52.0f
                };

                if (i == menu.equipChoice)
                {
                    DrawRectangleRec(row, Fade(Theme::Gold, 0.20f));
                    DrawRectangleLinesEx(row, 1.0f, Theme::Gold);
                }

                UiDrawText(def.name, (int)row.x + 20, y + 2, 22, Theme::Ink, true);

                if (id == EquipmentId::None)
                {
                    UiDrawText("REMOVE", (int)row.x + 440, y + 8, 16, Theme::Muted);
                }
                else
                {
                    std::string bonus;
                    if (def.hp != 0) bonus += "HP+" + std::to_string(def.hp) + "  ";
                    if (def.attack != 0) bonus += "ATK+" + std::to_string(def.attack) + "  ";
                    if (def.defense != 0) bonus += "DEF+" + std::to_string(def.defense);

                    UiDrawText(bonus.c_str(), (int)row.x + 440, y + 8, 16, Theme::Muted);
                }

                if (id == current)
                    UiDrawText("EQUIPPED", (int)row.x + 900, y + 8, 15, Theme::Gold, true);

                y += 56;
            }
        }

        if (!menu.message.empty())
            UiDrawText(menu.message.c_str(), (int)panel.x + 30, (int)panel.y + 640,
                19, Theme::Gold, true);

        const char* hint = (step == 0)
            ? "W/S  CHARACTER      J / ENTER  CHOOSE      K / ESC  BACK"
            : ((step == 1)
                ? "W/S  SLOT      J / ENTER  CHOOSE      K / ESC  BACK"
                : "W/S  EQUIPMENT      J / ENTER  CONFIRM      K / ESC  BACK");

        DrawHint(hint);
    }
}

