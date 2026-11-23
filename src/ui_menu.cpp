#include "ui.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>

using namespace UiInternal;

namespace
{
    void HelpSection(const char* heading, int x, int& y)
    {
        UiDrawText(heading, x, y, 22, Theme::Muted, true);
        y += 38;
    }

    void HelpLine(const char* text, int x, int& y)
    {
        UiDrawText(text, x, y, 19, Theme::Ink);
        y += 31;
    }
}

int UI::DrawMainMenu(int selectedIndex, bool showHelp) const
{
    DrawBackdrop();

    const int width = GetScreenWidth();
    const int height = GetScreenHeight();

    if (showHelp)
    {
        DrawRectangle(0, 0, width, height, Fade(BLACK, 0.45f));
        DrawRectangle(0, 0, width, 140, { 81, 0, 85, 255 });

        const char* title = "HOW TO PLAY";
        UiDrawText(title, width - 50 - UiMeasureText(title, 96, true),
            6, 96, WHITE, true);

        const Rectangle panel = { 80.0f, 170.0f, (float)width - 160.0f,
            (float)height - 250.0f };
        DrawRectangleRec(panel, Theme::Paper);
        DrawRectangleLinesEx(panel, 2.0f, Theme::Ink);

        const int leftX = (int)panel.x + 50;
        const int rightX = (int)panel.x + 760;

        int y = (int)panel.y + 40;

        HelpSection("OVERWORLD", leftX, y);
        HelpLine("WASD                  MOVE", leftX, y);
        HelpLine("SHIFT                 RUN", leftX, y);
        HelpLine("SPACE / J             SLASH", leftX, y);
        HelpLine("P / ESC / TAB         MENU", leftX, y);
        HelpLine("I                     ITEMS", leftX, y);

        y += 18;
        HelpSection("BATTLE", leftX, y);
        HelpLine("W / S                 COMMAND", leftX, y);
        HelpLine("ENTER / J             CONFIRM", leftX, y);
        HelpLine("A / D                 TARGET", leftX, y);
        HelpLine("K / ESC               BACK", leftX, y);

        int ry = (int)panel.y + 40;

        HelpSection("MENU", rightX, ry);
        HelpLine("W / S                 SELECT", rightX, ry);
        HelpLine("ENTER / J             CONFIRM", rightX, ry);
        HelpLine("K / ESC               BACK / CLOSE", rightX, ry);
        HelpLine("P / TAB               CLOSE", rightX, ry);

        ry += 18;
        HelpSection("EQUIPMENT", rightX, ry);
        HelpLine("W / S                 MEMBER, SLOT, ITEM", rightX, ry);
        HelpLine("J / ENTER             CONFIRM", rightX, ry);
        HelpLine("K / ESC               BACK", rightX, ry);

        ry += 18;
        HelpSection("ITEMS", rightX, ry);
        HelpLine("A / D                 CATEGORY", rightX, ry);
        HelpLine("W / S                 ITEM", rightX, ry);
        HelpLine("J / ENTER             TARGET / USE", rightX, ry);

        ry += 18;
        HelpSection("TIPS", rightX, ry);
        HelpLine("Hit weaknesses to gain extra turns.", rightX, ry);
        HelpLine("Watch for the tremendous foe every 10 floors.", rightX, ry);

        const char* hint = "K / ESC  CLOSE";
        UiDrawText(hint, (int)panel.x + (int)panel.width - 40 -
            UiMeasureText(hint, 19), (int)panel.y + (int)panel.height - 46,
            19, Theme::Muted, true);

        return -1;
    }

    const char* title = "ARCHETYPES";
    UiDrawText(title, 88, 120, 160, Theme::Ink, true);

    UiDrawText("A PRESS-TURN ARCHETYPE RPG", 104, 330, 22, Theme::Muted);

    DrawLine(104, 386, width - 104, 386, Fade(Theme::Ink, 0.25f));

    const char* labels[3] = { "New Game", "How to Play", "Quit" };
    const int optionY[3] = { 450, 514, 578 };

    int clicked = -1;

    for (int i = 0; i < 3; i++)
    {
        const Rectangle hit = {
            80.0f,
            (float)optionY[i] - 8.0f,
            620.0f,
            54.0f
        };

        const bool selected = (selectedIndex == i);

        if (selected)
            DrawRectangle(92, optionY[i], 5, 44, Theme::Gold);

        UiDrawText(labels[i], 116, optionY[i], selected ? 40 : 38,
            selected ? Theme::Ink : Theme::Muted, selected);

        if (selected)
        {
            const int textWidth = UiMeasureText(labels[i], 40, true);
            DrawRectangle(116, optionY[i] + 46, textWidth, 2, Theme::Gold);
        }

        if (CheckCollisionPointRec(GetMousePosition(), hit) &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            clicked = i;
        }
    }

    const char* hint = "W/S OR ARROWS   SELECT      ENTER / J   CONFIRM";
    UiDrawText(hint, 104, height - 84, 18, Theme::Muted);

    const char* version = "ARCHETYPES  v1.0";
    UiDrawText(version, width - 104 - UiMeasureText(version, 18),
        height - 84, 18, Theme::Muted);

    return clicked;
}
