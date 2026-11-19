#include "ui.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>

using namespace UiInternal;
int UI::DrawMainMenu(int selectedIndex, bool showHelp) const
{
    DrawBackdrop();

    const int width = GetScreenWidth();
    const int height = GetScreenHeight();

    if (showHelp)
    {
        DrawRectangle(0, 0, width, height, Fade(Theme::Ink, 0.35f));

        const Rectangle panel = {
            static_cast<float>(width) / 2.0f - 460.0f,
            static_cast<float>(height) / 2.0f - 310.0f,
            920.0f,
            620.0f
        };

        DrawRectangleRec(panel, Theme::Paper);
        DrawRectangleLinesEx(panel, 2.0f, Theme::Ink);
        DrawRectangle((int)panel.x, (int)panel.y, 6, (int)panel.height, Theme::Red);

        UiDrawText("How to Play", (int)panel.x + 30, (int)panel.y + 24, 40, Theme::Ink, true);
        DrawLine((int)panel.x + 30, (int)panel.y + 82,
            (int)panel.x + (int)panel.width - 30, (int)panel.y + 82, Fade(Theme::Ink, 0.3f));

        const char* lines[] = {
            "MOVE                  WASD",
            "RUN                   SHIFT",
            "ATTACK / DASH         SPACE or J",
            "PAUSE / STATUS        P or ESC",
            "INVENTORY             I",
            "DUNGEON PROGRESS      STEP ON THE STAIRS",
            "",
            "BATTLE",
            "  W / S               COMMAND MENU",
            "  ENTER / J           CONFIRM",
            "  A / D               SELECT TARGET (AFTER ACTION)",
            "  K / ESC             BACK",
            "",
            "PAUSE: 1 / 2 / 3 CYCLES WEAPON / ARMOR / ACCESSORY"
        };

        int y = (int)panel.y + 108;

        for (const char* line : lines)
        {
            if (line[0] != '\0')
                UiDrawText(line, (int)panel.x + 40, y, 20, Theme::Ink);

            y += 32;
        }

        UiDrawText("ENTER / J   CONFIRM      K / ESC   BACK", (int)panel.x + 40, (int)panel.y + 560, 18, Theme::Muted);
        return -1;
    }

    const char* title = "ARCHETYPES";
    UiDrawText(title, width / 2 - UiMeasureText(title, 150, true) / 2, 150, 150, Theme::Ink, true);

    const char* subtitle = "A PRESS-TURN ARCHETYPE RPG";
    UiDrawText(subtitle, width / 2 - UiMeasureText(subtitle, 24) / 2, 330, 24, Theme::Red);

    DrawLine(80, 392, width - 80, 392, Fade(Theme::Ink, 0.35f));

    const float optionY[3] = { 470.0f, 550.0f, 630.0f };
    const char* labels[3] = { "Start Game", "How to Play", "Quit" };

    int clicked = -1;

    for (int i = 0; i < 3; i++)
    {
        const Rectangle hit = {
            static_cast<float>(width) / 2.0f - 260.0f,
            optionY[i] - 10.0f,
            520.0f,
            54.0f
        };

        const bool selected = (selectedIndex == i);
        const Color color = selected ? Theme::Red : Theme::Ink;

        UiDrawText(labels[i],
            width / 2 - UiMeasureText(labels[i], 40, selected) / 2,
            (int)optionY[i], 40, color, selected);

        if (selected)
        {
            const int textWidth = UiMeasureText(labels[i], 40, true);
            DrawRectangle(width / 2 - textWidth / 2, (int)optionY[i] + 48, textWidth, 2, Theme::Gold);
        }

        DrawLine(80, (int)optionY[i] + 66, width - 80, (int)optionY[i] + 66, Fade(Theme::Ink, 0.15f));

        if (CheckCollisionPointRec(GetMousePosition(), hit) &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            clicked = i;
        }
    }

    const char* hint = "W/S OR ARROWS   SELECT      ENTER   CONFIRM";
    UiDrawText(hint, width / 2 - UiMeasureText(hint, 18) / 2, height - 84, 18, Theme::Muted);

    return clicked;
}


