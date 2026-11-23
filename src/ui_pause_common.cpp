#include "ui_pause_internal.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>
#include <string>
#include <vector>

using namespace UiInternal;

namespace PauseUi
{
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
        bool targeting, int targetIndex, bool targetAll)
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
}

