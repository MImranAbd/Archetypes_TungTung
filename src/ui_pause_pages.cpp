#include "ui_pause_internal.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>
#include <string>

using namespace UiInternal;

namespace PauseUi
{
    void DrawSkillsPage(const Party& party, int memberIndex)
    {
        DrawBanner(PausePage::Skills, party);
        DrawPartyList(party, 40, 200, 430, memberIndex);

        const Character& character = party.Members()[memberIndex];

        const Rectangle panel = { 520.0f, 190.0f, 1360.0f, 700.0f };
        DrawPaperPanel(panel);

        UiDrawText(TextFormat("%s  /  %s  /  SKILLS", character.name,
            RoleName(character.role)),
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

        UiDrawText(RoleName(character.role), (int)panel.x + 30,
            (int)panel.y + 106, 20, Theme::Gold, true);
        UiDrawText(RoleDescription(character.role), (int)panel.x + 30,
            (int)panel.y + 130, 14, Theme::Muted);

        DrawLine((int)panel.x + 30, (int)panel.y + 150,
            (int)panel.x + (int)panel.width - 30, (int)panel.y + 150,
            Fade(Theme::Ink, 0.25f));

        UiDrawText(TextFormat("HP    %d / %d", character.hp, character.maxHp),
            (int)panel.x + 30, (int)panel.y + 172, 22, Theme::Ink);
        UiDrawText(TextFormat("ATK   %d", character.attack),
            (int)panel.x + 30, (int)panel.y + 206, 22, Theme::Ink);
        UiDrawText(TextFormat("DEF   %d", character.defense),
            (int)panel.x + 30, (int)panel.y + 240, 22, Theme::Ink);

        UiDrawText("PASSIVES", (int)panel.x + 30, (int)panel.y + 278, 20,
            Theme::Muted, true);

        int passiveY = (int)panel.y + 304;

        for (int i = 0; i < RolePassiveCount(character.role); i++)
        {
            const Passive& passive = RolePassive(character.role, i);

            UiDrawText(passive.name, (int)panel.x + 36, passiveY, 18,
                Theme::Ink, true);
            UiDrawText(passive.description, (int)panel.x + 36, passiveY + 18, 14,
                Theme::Muted);

            passiveY += 42;
        }

        UiDrawText("EQUIPMENT", (int)panel.x + 30, (int)panel.y + 394, 20,
            Theme::Muted, true);
        UiDrawText(GetEquipment(character.weapon).name,
            (int)panel.x + 36, (int)panel.y + 422, 19, Theme::Ink);
        UiDrawText(GetEquipment(character.armor).name,
            (int)panel.x + 36, (int)panel.y + 450, 19, Theme::Ink);
        UiDrawText(GetEquipment(character.accessory).name,
            (int)panel.x + 36, (int)panel.y + 478, 19, Theme::Ink);

        UiDrawText("SKILLS", (int)panel.x + 30, (int)panel.y + 500, 20,
            Theme::Muted, true);

        for (int i = 0; i < static_cast<int>(character.skills.size()); i++)
        {
            const Skill& skill = *character.skills[i];
            const bool unlocked = character.level >= skill.GetLevelRequired();
            const int column = (i < 5) ? 0 : 1;
            const int row = (i < 5) ? i : (i - 5);
            const int skillY = (int)panel.y + 518 + row * 20;

            UiDrawText(skill.GetName(), (int)panel.x + 36 + column * 270,
                skillY, 16, unlocked ? Theme::Ink : Theme::Muted);
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
            "DEBUG        F9 TELEPORT TO FLOOR 50",
        };

        int y = (int)panel.y + 148;

        for (const char* line : controls)
        {
            UiDrawText(line, (int)panel.x + 36, y, 18, Theme::Ink);
            y += 28;
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

