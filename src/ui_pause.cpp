#include "ui.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>
#include <string>
#include <vector>

using namespace UiInternal;
void UI::DrawPauseMenu(const Party& party, const Inventory& inventory,
    int tab, int memberIndex, int itemIndex, const std::string& message) const
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();

    DrawRectangle(0, 0, width, height, Fade(Theme::Ink, 0.40f));

    const Rectangle panel = {
        static_cast<float>(width) / 2.0f - 700.0f,
        static_cast<float>(height) / 2.0f - 360.0f,
        1400.0f,
        720.0f
    };

    DrawRectangleRec(panel, Theme::Parchment);
    DrawRectangleLinesEx(panel, 2.0f, Theme::Ink);
    DrawRectangle((int)panel.x, (int)panel.y, 6, (int)panel.height, Theme::Red);

    UiDrawText("PAUSE", (int)panel.x + 30, (int)panel.y + 20, 34, Theme::Ink, true);

    const char* moneyText = TextFormat("MNY %d", party.GetMoney());
    UiDrawText(moneyText,
        (int)(panel.x + panel.width) - 30 - UiMeasureText(moneyText, 20, true),
        (int)panel.y + 26, 20, Theme::Gold, true);

    const Rectangle partyTab = { panel.x + 30.0f, panel.y + 70.0f, 180.0f, 38.0f };
    const Rectangle itemsTab = { panel.x + 220.0f, panel.y + 70.0f, 180.0f, 38.0f };

    UiDrawText("PARTY", (int)partyTab.x + 54, (int)partyTab.y + 6, 22,
        tab == 0 ? Theme::Gold : Theme::Muted);
    UiDrawText("ITEMS", (int)itemsTab.x + 54, (int)itemsTab.y + 6, 22,
        tab == 1 ? Theme::Gold : Theme::Muted);

    DrawRectangle((int)((tab == 0) ? partyTab.x : itemsTab.x),
        (int)partyTab.y + 32, 180, 2, Theme::Gold);

    DrawLine((int)panel.x + 30, (int)panel.y + 112,
        (int)panel.x + (int)panel.width - 30, (int)panel.y + 112, Fade(Theme::Ink, 0.3f));

    int member = memberIndex;
    if (member < 0 || member >= static_cast<int>(party.Members().size()))
        member = 0;

    if (tab == 0)
    {
        int listY = (int)panel.y + 140;

        for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
        {
            const Character& character = party.Members()[i];
            const Rectangle row = {
                panel.x + 30.0f,
                static_cast<float>(listY),
                430.0f,
                64.0f
            };

            if (i == member)
            {
                DrawRectangleRec(row, Fade(Theme::Gold, 0.16f));
                DrawRectangleLinesEx(row, 2.0f, Theme::Gold);
            }

            UiDrawText(character.name, (int)row.x + 20, listY + 8, 24, Theme::Ink);
            UiDrawText(TextFormat("LV %d", character.level),
                (int)row.x + 270, listY + 10, 18, Theme::Muted);
            UiDrawText(TextFormat("HP %d / %d", character.hp, character.maxHp),
                (int)row.x + 20, listY + 38, 16, Theme::Muted);

            listY += 76;
        }

        const Character& character = party.Members()[member];
        const int x = (int)panel.x + 520;
        int y = (int)panel.y + 140;

        UiDrawText(character.name, x, y, 32, Theme::Ink);
        UiDrawText(TextFormat("LEVEL %d      XP %d / %d",
            character.level, character.xp, character.xpToNext), x, y + 44, 18, Theme::Muted);

        UiDrawText(TextFormat("HP    %d / %d", character.hp, character.maxHp), x, y + 82, 22, Theme::Ink);
        UiDrawText(TextFormat("ATK   %d", character.attack), x, y + 114, 22, Theme::Ink);
        UiDrawText(TextFormat("DEF   %d", character.defense), x, y + 146, 22, Theme::Ink);

        UiDrawText("EQUIPMENT", x, y + 192, 20, Theme::Gold);
        UiDrawText(TextFormat("%s      %s", EquipSlotName(EquipSlot::Weapon),
            GetEquipment(character.weapon).name), x, y + 224, 18, Theme::Ink);
        UiDrawText(TextFormat("%s       %s", EquipSlotName(EquipSlot::Armor),
            GetEquipment(character.armor).name), x, y + 252, 18, Theme::Ink);
        UiDrawText(TextFormat("%s   %s", EquipSlotName(EquipSlot::Accessory),
            GetEquipment(character.accessory).name), x, y + 280, 18, Theme::Ink);

        UiDrawText("SKILLS", x, y + 326, 20, Theme::Gold);

        int skillY = y + 358;

        for (int i = 0; i < static_cast<int>(character.skills.size()); i++)
        {
            const Skill& skill = *character.skills[i];
            const bool unlocked = character.level >= skill.GetLevelRequired();

            UiDrawText(TextFormat("%d   %s", i + 1, skill.GetName()), x, skillY, 18,
                unlocked ? Theme::Ink : Theme::Muted);
            UiDrawText(TextFormat("LV %d", skill.GetLevelRequired()), x + 340, skillY, 16,
                unlocked ? Theme::Gold : Theme::Muted);

            skillY += 28;
        }

        UiDrawText("W/S  MEMBER      1/2/3  CYCLE WEAPON / ARMOR / ACCESSORY      A/D  TAB      K / ESC  CLOSE",
            (int)panel.x + 30, (int)panel.y + 660, 16, Theme::Muted);
    }
    else
    {
        const std::vector<ItemStack>& stacks = inventory.Stacks();
        int selected = itemIndex;

        if (stacks.empty())
            selected = 0;
        else if (selected < 0 || selected >= static_cast<int>(stacks.size()))
            selected = 0;

        UiDrawText("CONSUMABLES", (int)panel.x + 30, (int)panel.y + 140, 20, Theme::Gold);

        int y = (int)panel.y + 180;

        if (stacks.empty())
        {
            UiDrawText("EMPTY", (int)panel.x + 60, y, 22, Theme::Muted);
        }
        else
        {
            for (int i = 0; i < static_cast<int>(stacks.size()); i++)
            {
                const ItemStack& stack = stacks[i];
                const Rectangle row = {
                    panel.x + 30.0f,
                    static_cast<float>(y) - 6.0f,
                    620.0f,
                    46.0f
                };

                if (i == selected)
                {
                    DrawRectangleRec(row, Fade(Theme::Gold, 0.18f));
                    DrawRectangleLinesEx(row, 2.0f, Theme::Gold);
                    UiDrawText(">", (int)panel.x + 40, y + 6, 20, Theme::Gold);
                }

                UiDrawText(ItemName(stack.type), (int)panel.x + 68, y + 4, 22, Theme::Ink);
                UiDrawText(TextFormat("x%d", stack.count), (int)panel.x + 300, y + 6, 20, Theme::Gold);
                UiDrawText(TextFormat("HEALS %d", ItemHeal(stack.type)),
                    (int)panel.x + 400, y + 8, 16, Theme::Muted);

                y += 52;
            }

            const ItemStack& selectedStack = stacks[selected];
            const std::string description = std::string(ItemName(selectedStack.type))
                + " restores " + std::to_string(ItemHeal(selectedStack.type))
                + " HP to the weakest living ally.";

            UiDrawText(description.c_str(), (int)panel.x + 40, (int)panel.y + 380, 18, Theme::Ink);
        }

        UiDrawText("EQUIPMENT IN BAG", (int)panel.x + 720, (int)panel.y + 140, 20, Theme::Gold);

        int equipmentY = (int)panel.y + 180;
        const std::vector<EquipmentStack>& equipment = inventory.EquipmentStacks();

        if (equipment.empty())
        {
            UiDrawText("EMPTY", (int)panel.x + 750, equipmentY, 22, Theme::Muted);
        }
        else
        {
            for (const EquipmentStack& stack : equipment)
            {
                const EquipmentDef& def = GetEquipment(stack.id);

                UiDrawText(TextFormat("%s   x%d", def.name, stack.count),
                    (int)panel.x + 740, equipmentY, 18, Theme::Ink);

                std::string bonus;
                if (def.hp != 0) bonus += "HP+" + std::to_string(def.hp) + "  ";
                if (def.attack != 0) bonus += "ATK+" + std::to_string(def.attack) + "  ";
                if (def.defense != 0) bonus += "DEF+" + std::to_string(def.defense);

                UiDrawText(bonus.c_str(), (int)panel.x + 740, equipmentY + 22, 14, Theme::Muted);

                equipmentY += 52;
            }
        }

        std::string partyLine;

        for (const Character& character : party.Members())
        {
            partyLine += std::string(character.name) + " " + std::to_string(character.hp)
                + "/" + std::to_string(character.maxHp) + "   ";
        }

        UiDrawText(partyLine.c_str(), (int)panel.x + 30, (int)panel.y + 572, 16, Theme::Muted);
        UiDrawText("W/S  SELECT      SPACE / J  USE      A/D  TAB      K / ESC  CLOSE",
            (int)panel.x + 30, (int)panel.y + 660, 16, Theme::Muted);
    }

    UiDrawText(message.c_str(), (int)panel.x + 30, (int)panel.y + 612, 18, Theme::Gold);
}


