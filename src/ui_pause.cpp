#include "ui.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>
#include <string>
#include <vector>

using namespace UiInternal;

namespace
{
    constexpr int bannerHeight = 150;

    Color PageBannerColor(PausePage page)
    {
        switch (page)
        {
            case PausePage::Items: return { 85, 0, 0, 255 };
            case PausePage::Equip: return { 85, 68, 0, 255 };
            case PausePage::Skills: return { 0, 59, 2, 255 };
            case PausePage::Quests: return { 0, 71, 85, 255 };
            case PausePage::Member: return { 10, 0, 85, 255 };
            case PausePage::Systems: return { 81, 0, 85, 255 };
            default: return Theme::Ink;
        }
    }

    const char* PageTitle(PausePage page)
    {
        switch (page)
        {
            case PausePage::Items: return "ITEM";
            case PausePage::Equip: return "EQUIPMENT";
            case PausePage::Skills: return "SKILLS";
            case PausePage::Quests: return "QUESTS";
            case PausePage::Member: return "MEMBER";
            case PausePage::Systems: return "SYSTEMS";
            default: return "";
        }
    }

    Color ItemCategoryColor(ItemCategory category)
    {
        switch (category)
        {
            case ItemCategory::Healing: return Theme::Green;
            case ItemCategory::Weapon: return Theme::Blue;
            case ItemCategory::Attack: return Theme::Red;
            case ItemCategory::Other: return Theme::Muted;
        }

        return Theme::Muted;
    }

    void DrawBar(float x, float y, float width, float height,
        float fraction, Color fill)
    {
        if (fraction < 0.0f) fraction = 0.0f;
        if (fraction > 1.0f) fraction = 1.0f;

        DrawRectangle((int)x, (int)y, (int)width, (int)height, Fade(Theme::Ink, 0.30f));
        DrawRectangle((int)x, (int)y, (int)(width * fraction), (int)height, fill);
        DrawRectangleLines((int)x, (int)y, (int)width, (int)height, Fade(Theme::Ink, 0.45f));
    }

    void DrawFace(int index, float x, float y, float size)
    {
        if (PartyBattlePoses[index].id > 0)
        {
            DrawTexturePro(PartyBattlePoses[index], { 0.0f, 0.0f, 32.0f, 32.0f },
                { x, y, size, size }, { 0.0f, 0.0f }, 0.0f, WHITE);
        }
        else
        {
            DrawRectangle((int)x, (int)y, (int)size, (int)size, PartyTints[index]);
        }
    }

    void DrawPartyList(const Party& party, int x, int y, int width, int selected,
        bool targeting = false, int targetIndex = 0, bool targetAll = false)
    {
        const int rowHeight = 96;
        const int count = static_cast<int>(party.Members().size());

        DrawRectangleLinesEx({ (float)x, (float)y, (float)width,
            (float)(rowHeight * count) }, 2.0f, Fade(WHITE, 0.85f));

        for (int i = 0; i < count; i++)
        {
            const Character& member = party.Members()[i];
            const Rectangle row = {
                (float)x,
                (float)(y + i * rowHeight),
                (float)width,
                (float)rowHeight
            };

            const bool lit = targeting && (targetAll || i == targetIndex);

            if (lit)
            {
                DrawRectangleRec(row, Fade(Theme::Gold, targetAll ? 0.18f : 0.24f));
                DrawRectangleLinesEx(row, 3.0f, Theme::Gold);
            }
            else if (i == selected)
            {
                DrawRectangleRec(row, Fade(WHITE, 0.12f));
                DrawRectangle(x, y + i * rowHeight, 5, rowHeight, Theme::Gold);
            }

            if (i > 0)
                DrawLine(x, y + i * rowHeight, x + width, y + i * rowHeight,
                    Fade(WHITE, 0.25f));

            DrawFace(i, row.x + 14.0f, row.y + 16.0f, 62.0f);

            Color nameColor = WHITE;
            if (lit) nameColor = Theme::Gold;
            else if (i == selected) nameColor = Theme::Gold;

            UiDrawText(member.name, x + 90, y + i * rowHeight + 10, 26,
                nameColor, lit || i == selected);

            UiDrawText(TextFormat("LV %d", member.level),
                x + 90, y + i * rowHeight + 42, 17, Fade(WHITE, 0.72f));

            DrawBar((float)(x + 90), (float)(y + i * rowHeight + 68), 200.0f, 9.0f,
                (member.maxHp > 0) ? (float)member.hp / member.maxHp : 0.0f,
                Theme::Green);

            UiDrawText(TextFormat("%d / %d", member.hp, member.maxHp),
                x + 300, y + i * rowHeight + 62, 15, Fade(WHITE, 0.78f));
        }
    }

    void DrawPaperPanel(Rectangle panel)
    {
        DrawRectangleRec(panel, Theme::Paper);
        DrawRectangleLinesEx(panel, 2.0f, Theme::Ink);
    }

    void DrawBanner(PausePage page, const Party& party)
    {
        const int width = GetScreenWidth();

        DrawRectangle(0, 0, width, bannerHeight, PageBannerColor(page));

        const int titleIndex = static_cast<int>(page) - 1;

        if (titleIndex >= 0 && titleIndex < 6 && MenuTitles[titleIndex].id > 0)
        {
            const Texture2D& title = MenuTitles[titleIndex];
            float titleHeight = 116.0f;
            float titleWidth = titleHeight * (float)title.width / (float)title.height;

            if (titleWidth > 980.0f)
            {
                titleWidth = 980.0f;
                titleHeight = titleWidth * (float)title.height / (float)title.width;
            }

            DrawTexturePro(title, { 0.0f, 0.0f, (float)title.width, (float)title.height },
                { (float)width - 40.0f - titleWidth,
                  (bannerHeight - titleHeight) / 2.0f, titleWidth, titleHeight },
                { 0.0f, 0.0f }, 0.0f, WHITE);
        }
        else
        {
            const char* title = PageTitle(page);
            const int titleSize = 100;
            UiDrawText(title, width - 50 - UiMeasureText(title, titleSize, true),
                8, titleSize, WHITE, true);
        }

        UiDrawText("ARCHETYPES", 40, 24, 24, Fade(WHITE, 0.75f), true);
        UiDrawText(TextFormat("MNY %d", party.GetMoney()), 40, 62, 22,
            Fade(WHITE, 0.85f), true);
    }

    void DrawHint(const char* text)
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        UiDrawText(text, width - 40 - UiMeasureText(text, 17),
            height - 38, 17, Fade(WHITE, 0.75f));
    }

    void DrawRootMenu(int menuIndex, const Party& party)
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();

        if (MenuPolygon.id > 0)
        {
            DrawTexturePro(MenuPolygon,
                { 0.0f, 0.0f, (float)MenuPolygon.width, (float)MenuPolygon.height },
                { (float)width / 2.0f - 560.0f, 20.0f, 1120.0f, 1100.0f },
                { 0.0f, 0.0f }, 0.0f, WHITE);
        }
        else
        {
            DrawTriangle({ (float)width / 2.0f, 20.0f },
                { 340.0f, (float)height }, { (float)width - 340.0f, (float)height },
                { 217, 217, 217, 255 });
        }

        if (SpritesLoaded && ElizaWalkDown[0].id > 0)
        {
            DrawTexturePro(ElizaWalkDown[0], { 0.0f, 0.0f, 32.0f, 32.0f },
                { (float)width / 2.0f - 48.0f, 86.0f, 96.0f, 96.0f },
                { 0.0f, 0.0f }, 0.0f, WHITE);
        }

        const char* labels[6] = {
            "ITEM", "EQUIP", "SKILLS", "QUESTS", "MEMBER", "SYSTEMS"
        };

        int y = 236;

        for (int i = 0; i < 6; i++)
        {
            const bool selected = (i == menuIndex);
            const int size = selected ? 68 : 60;
            const int textWidth = UiMeasureText(labels[i], size, selected);
            const int x = width / 2 - textWidth / 2;

            UiDrawText(labels[i], x, y, size, Theme::Ink, selected);

            if (selected)
            {
                const int caretY = y + (size * 2) / 3;
                DrawTriangle({ (float)(x - 34), (float)caretY },
                    { (float)(x - 12), (float)(caretY - 12) },
                    { (float)(x - 12), (float)(caretY + 12) }, Theme::Gold);
                DrawRectangle(x, y + size + 4, textWidth, 3, Theme::Gold);
            }

            y += 104;
        }

        UiDrawText(TextFormat("MNY %d", party.GetMoney()),
            40, height - 44, 20, Fade(WHITE, 0.75f), true);

        const char* hint = "W/S  SELECT      J / ENTER  CONFIRM      K / ESC  CLOSE";
        UiDrawText(hint, width / 2 - UiMeasureText(hint, 17) / 2,
            height - 44, 17, Fade(Theme::Ink, 0.85f));
    }

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

    void DrawSkillsPage(const Party& party, int memberIndex)
    {
        DrawBanner(PausePage::Skills, party);
        DrawPartyList(party, 40, 200, 430, memberIndex);

        const Character& character = party.Members()[memberIndex];

        const Rectangle panel = { 520.0f, 190.0f, 1360.0f, 700.0f };
        DrawPaperPanel(panel);

        UiDrawText(TextFormat("%s  /  SKILLS", character.name),
            (int)panel.x + 30, (int)panel.y + 24, 26, Theme::Ink, true);

        DrawLine((int)panel.x + 30, (int)panel.y + 70,
            (int)panel.x + (int)panel.width - 30, (int)panel.y + 70,
            Fade(Theme::Ink, 0.25f));

        int y = (int)panel.y + 100;

        for (int i = 0; i < static_cast<int>(character.skills.size()); i++)
        {
            const Skill& skill = *character.skills[i];
            const bool unlocked = character.level >= skill.GetLevelRequired();

            UiDrawText(TextFormat("%d", i + 1), (int)panel.x + 36, y + 2, 22,
                Theme::Gold, true);
            UiDrawText(skill.GetName(), (int)panel.x + 82, y, 24,
                unlocked ? Theme::Ink : Theme::Muted, true);
            UiDrawText(TextFormat("LV %d", skill.GetLevelRequired()),
                (int)panel.x + 330, y + 6, 16, unlocked ? Theme::Gold : Theme::Muted);
            UiDrawText(skill.GetDescription(), (int)panel.x + 430, y + 6, 17,
                unlocked ? Theme::Muted : Fade(Theme::Muted, 0.6f));

            DrawLine((int)panel.x + 30, y + 44,
                (int)panel.x + (int)panel.width - 30, y + 44,
                Fade(Theme::Ink, 0.15f));

            y += 58;
        }

        DrawHint("W/S  MEMBER      K / ESC  BACK      P  CLOSE");
    }

    void DrawQuestsPage(const Party& party)
    {
        DrawBanner(PausePage::Quests, party);

        const Rectangle listPanel = { 40.0f, 200.0f, 430.0f, 620.0f };
        DrawPaperPanel(listPanel);

        UiDrawText("ACTIVE", (int)listPanel.x + 24, (int)listPanel.y + 24, 20,
            Theme::Muted, true);

        const Rectangle questRow = {
            listPanel.x + 18.0f, listPanel.y + 64.0f, listPanel.width - 36.0f, 62.0f
        };
        DrawRectangleRec(questRow, Fade(Theme::Gold, 0.18f));
        DrawRectangleLinesEx(questRow, 1.0f, Theme::Gold);
        UiDrawText("Confront Lord Agustus", (int)questRow.x + 14, (int)questRow.y + 10,
            20, Theme::Ink, true);
        UiDrawText("MAIN QUEST", (int)questRow.x + 14, (int)questRow.y + 36, 14,
            Theme::Muted);

        const Rectangle panel = { 520.0f, 190.0f, 1360.0f, 700.0f };
        DrawPaperPanel(panel);

        UiDrawText("Confront Lord Agustus", (int)panel.x + 30, (int)panel.y + 28, 40,
            Theme::Ink, true);
        DrawLine((int)panel.x + 30, (int)panel.y + 84,
            (int)panel.x + (int)panel.width - 30, (int)panel.y + 84,
            Fade(Theme::Ink, 0.25f));

        const char* lines[] = {
            "Lord Agustus waits at the bottom of the dungeon.",
            "Descend floor by floor and gather strength for the final",
            "confrontation.",
            "",
            "A tremendous foe guards every 10th floor.",
            "Defeat it to claim the deeper path.",
            "",
            "Press on. Your party follows.",
        };

        int y = (int)panel.y + 110;

        for (const char* line : lines)
        {
            if (line[0] != '\0')
                UiDrawText(line, (int)panel.x + 36, y, 20, Theme::Ink);

            y += 34;
        }

        DrawHint("K / ESC  BACK      P  CLOSE");
    }

    void DrawMemberPage(const Party& party, int memberIndex)
    {
        DrawBanner(PausePage::Member, party);
        DrawPartyList(party, 40, 200, 430, memberIndex);

        const Character& character = party.Members()[memberIndex];
        const int width = GetScreenWidth();

        if (memberIndex == 0 && MenuPortrait.id > 0)
        {
            DrawTexturePro(MenuPortrait,
                { 0.0f, 0.0f, (float)MenuPortrait.width, (float)MenuPortrait.height },
                { (float)width - 660.0f, 180.0f, 620.0f, 620.0f },
                { 0.0f, 0.0f }, 0.0f, WHITE);
        }
        else if (PartyBattlePoses[memberIndex].id > 0)
        {
            DrawTexturePro(PartyBattlePoses[memberIndex], { 0.0f, 0.0f, 32.0f, 32.0f },
                { (float)width - 620.0f, 180.0f, 540.0f, 540.0f },
                { 0.0f, 0.0f }, 0.0f, WHITE);
        }

        const Rectangle panel = { 520.0f, 190.0f, 620.0f, 620.0f };
        DrawPaperPanel(panel);

        UiDrawText(character.name, (int)panel.x + 30, (int)panel.y + 26, 40,
            Theme::Ink, true);
        UiDrawText(TextFormat("LV %d      XP %d / %d", character.level,
            character.xp, character.xpToNext),
            (int)panel.x + 30, (int)panel.y + 82, 18, Theme::Muted);

        DrawLine((int)panel.x + 30, (int)panel.y + 120,
            (int)panel.x + (int)panel.width - 30, (int)panel.y + 120,
            Fade(Theme::Ink, 0.25f));

        UiDrawText(TextFormat("HP    %d / %d", character.hp, character.maxHp),
            (int)panel.x + 30, (int)panel.y + 146, 22, Theme::Ink);
        UiDrawText(TextFormat("ATK   %d", character.attack),
            (int)panel.x + 30, (int)panel.y + 184, 22, Theme::Ink);
        UiDrawText(TextFormat("DEF   %d", character.defense),
            (int)panel.x + 30, (int)panel.y + 222, 22, Theme::Ink);

        UiDrawText("EQUIPMENT", (int)panel.x + 30, (int)panel.y + 276, 20,
            Theme::Muted, true);
        UiDrawText(GetEquipment(character.weapon).name,
            (int)panel.x + 36, (int)panel.y + 308, 19, Theme::Ink);
        UiDrawText(GetEquipment(character.armor).name,
            (int)panel.x + 36, (int)panel.y + 338, 19, Theme::Ink);
        UiDrawText(GetEquipment(character.accessory).name,
            (int)panel.x + 36, (int)panel.y + 368, 19, Theme::Ink);

        UiDrawText("SKILLS", (int)panel.x + 30, (int)panel.y + 420, 20,
            Theme::Muted, true);

        int skillY = (int)panel.y + 452;

        for (int i = 0; i < static_cast<int>(character.skills.size()); i++)
        {
            const Skill& skill = *character.skills[i];
            const bool unlocked = character.level >= skill.GetLevelRequired();

            UiDrawText(skill.GetName(), (int)panel.x + 36, skillY, 18,
                unlocked ? Theme::Ink : Theme::Muted);

            skillY += 30;
        }

        const Rectangle nameStrip = {
            40.0f, (float)GetScreenHeight() - 118.0f, (float)width - 80.0f, 78.0f
        };
        DrawRectangleRec(nameStrip, Theme::Paper);
        DrawRectangleLinesEx(nameStrip, 1.0f, Theme::Ink);
        DrawFace(memberIndex, nameStrip.x + 10.0f, nameStrip.y + 8.0f, 62.0f);
        UiDrawText(character.name, (int)nameStrip.x + 88, (int)nameStrip.y + 18, 36,
            Theme::Ink, true);
        UiDrawText(TextFormat("LV %d", character.level),
            (int)(nameStrip.x + nameStrip.width) - 140, (int)nameStrip.y + 18, 20,
            Theme::Muted);

        const char* memberHint = "W/S  MEMBER      K / ESC  BACK      P  CLOSE";
        UiDrawText(memberHint, (int)(nameStrip.x + nameStrip.width) - 24 -
            UiMeasureText(memberHint, 14), (int)nameStrip.y + 50, 14, Theme::Muted);
    }

    void DrawSystemsPage(const Party& party, int dungeonFloor)
    {
        DrawBanner(PausePage::Systems, party);

        const Rectangle panel = { 150.0f, 200.0f, 1620.0f, 680.0f };
        DrawPaperPanel(panel);

        UiDrawText("SYSTEMS", (int)panel.x + 36, (int)panel.y + 28, 40,
            Theme::Ink, true);
        DrawLine((int)panel.x + 36, (int)panel.y + 84,
            (int)panel.x + (int)panel.width - 36, (int)panel.y + 84,
            Fade(Theme::Ink, 0.25f));

        UiDrawText("CONTROLS", (int)panel.x + 36, (int)panel.y + 110, 22,
            Theme::Muted, true);

        const char* controls[] = {
            "OVERWORLD      WASD MOVE      SHIFT RUN      SPACE / J SLASH",
            "             P / ESC / TAB MENU      I ITEMS",
            "BATTLE       W/S COMMAND      ENTER / J CONFIRM",
            "             A/D TARGET      K / ESC BACK",
            "MENU         W/S SELECT      J / ENTER CONFIRM      K / ESC BACK",
            "ITEM         A/D CATEGORY      W/S ITEM      J TARGET      K BACK",
            "EQUIP        W/S MEMBER > SLOT > ITEM      J CONFIRM      K BACK",
        };

        int y = (int)panel.y + 148;

        for (const char* line : controls)
        {
            UiDrawText(line, (int)panel.x + 36, y, 18, Theme::Ink);
            y += 30;
        }

        UiDrawText("RUN", (int)panel.x + 36, (int)panel.y + 380, 22,
            Theme::Muted, true);
        UiDrawText(TextFormat("DUNGEON FLOOR %d", dungeonFloor),
            (int)panel.x + 36, (int)panel.y + 418, 22, Theme::Ink);
        UiDrawText(TextFormat("PARTY LEVEL %d", party.Members()[0].level),
            (int)panel.x + 36, (int)panel.y + 456, 22, Theme::Ink);

        UiDrawText("RETURN TO GAME", (int)panel.x + 36, (int)panel.y + 560, 26,
            Theme::Gold, true);

        DrawHint("K / ESC  BACK      P  CLOSE");
    }
}

void UI::DrawPauseMenu(const Party& party, const Inventory& inventory,
    const PauseMenuState& menu, int dungeonFloor) const
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();

    DrawRectangle(0, 0, width, height, Fade(BLACK, 0.62f));

    int member = menu.memberIndex;

    if (member < 0 || member >= static_cast<int>(party.Members().size()))
        member = 0;

    if (menu.page == PausePage::Root)
    {
        DrawRootMenu(menu.menuIndex, party);
        return;
    }

    PauseMenuState state = menu;
    state.memberIndex = member;

    if (state.page == PausePage::Items)
        DrawItemPage(party, inventory, state);
    else if (state.page == PausePage::Equip)
        DrawEquipPage(party, inventory, state);
    else if (state.page == PausePage::Skills)
        DrawSkillsPage(party, member);
    else if (state.page == PausePage::Quests)
        DrawQuestsPage(party);
    else if (state.page == PausePage::Member)
        DrawMemberPage(party, member);
    else if (state.page == PausePage::Systems)
        DrawSystemsPage(party, dungeonFloor);
}
