#include "ui_battle_internal.h"
#include "ui_internal.h"
#include "theme.h"
#include "battle.h"
#include "party.h"

#include <raylib.h>
#include <cmath>
#include <string>
#include <vector>

using namespace UiInternal;

namespace BattleUi
{const Vector2 PartyPos[4] = {
        { 180.0f, 700.0f },
        { 310.0f, 620.0f },
        { 440.0f, 540.0f },
        { 570.0f, 460.0f }
    };

    const Vector2 EnemyPos[5] = {
        { 1120.0f, 610.0f },
        { 1300.0f, 480.0f },
        { 1500.0f, 620.0f },
        { 1680.0f, 500.0f },
        { 1820.0f, 690.0f }
    };

    const float EnemySizes[5] = { 100.0f, 90.0f, 105.0f, 95.0f, 100.0f };


    const char* CommandLabel(int index)
    {
        static const char* labels[Battle::commandCount] = {
            "Attack", "Skill", "Defense", "Item", "Pass", "Escape"
        };

        if (index < 0 || index >= Battle::commandCount) return "";
        return labels[index];
    }

    const char* CommandDescription(int index)
    {
        static const char* descriptions[Battle::commandCount] = {
            "Strike the selected enemy with a basic attack.",
            "Open the skill list and choose a learned skill.",
            "Brace the party, reducing enemy damage this turn.",
            "Use a consumable item from the bag.",
            "Pass the turn to the next ally. Costs half an icon.",
            "Attempt to flee the battle. No rewards."
        };

        if (index < 0 || index >= Battle::commandCount) return "";
        return descriptions[index];
    }

    const char* ItemDescription(ItemType type)
    {
        return (type == ItemType::Herb)
            ? "Restores 5 HP to the weakest living ally."
            : "Restores 10 HP to the weakest living ally.";
    }

    void DrawBar(int x, int y, int width, int height, float ratio, Color fill)
    {
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;

        DrawRectangle(x, y, width, height, Theme::PaperDark);
        DrawRectangleLines(x, y, width, height, Theme::Ink);

        const int innerWidth = static_cast<int>((width - 2) * ratio);

        if (innerWidth > 0)
            DrawRectangle(x + 1, y + 1, innerWidth, height - 2, fill);
    }

    void DrawWrappedText(const char* text, int x, int y, int maxWidth, int fontSize,
        Color color, bool bold)
    {
        std::string line;
        std::string word;
        int lineY = y;

        for (const char* p = text;; p++)
        {
            const char c = *p;

            if (c == ' ' || c == '\0')
            {
                const std::string candidate = line.empty() ? word : line + " " + word;

                if (UiMeasureText(candidate.c_str(), fontSize, bold) > maxWidth &&
                    !line.empty())
                {
                    UiDrawText(line.c_str(), x, lineY, fontSize, color, bold);
                    lineY += fontSize + 6;
                    line = word;
                }
                else
                {
                    line = candidate;
                }

                word.clear();

                if (c == '\0') break;
            }
            else
            {
                word += c;
            }
        }

        if (!line.empty())
            UiDrawText(line.c_str(), x, lineY, fontSize, color, bold);
    }

    void DrawTurnSquares(int points, int maxIcons, int x, int y, Color fill)
    {
        const int size = 48;
        const int gap = 15;

        for (int i = 0; i < maxIcons; i++)
        {
            const Rectangle rect = {
                static_cast<float>(x + i * (size + gap)),
                static_cast<float>(y),
                static_cast<float>(size),
                static_cast<float>(size)
            };

            DrawRectangleRec(rect, Theme::Paper);
            DrawRectangleLinesEx(rect, 2.0f, Theme::Ink);

            const int value = points - i * 2;

            if (value >= 2)
                DrawRectangle(x + i * (size + gap) + 4, y + 4, size - 8, size - 8, fill);
            else if (value == 1)
                DrawRectangle(x + i * (size + gap) + 4, y + 4, (size - 8) / 2, size - 8, fill);
        }
    }

    void DrawDiamond(float x, float y, float radius, Color fill)
    {
        DrawPoly({ x, y }, 4, radius, 0.0f, fill);
        DrawPolyLinesEx({ x, y }, 4, radius, 0.0f, 2.0f, Theme::Ink);
    }

    void DrawTargetCaret(float x, float y, Color color)
    {
        DrawLineEx({ x - 12.0f, y - 10.0f }, { x, y }, 2.5f, color);
        DrawLineEx({ x + 12.0f, y - 10.0f }, { x, y }, 2.5f, color);
    }

    void DrawStarShape(int cx, int cy, float outer)
    {
        Vector2 points[10];
        points[0] = { static_cast<float>(cx), static_cast<float>(cy) };

        for (int i = 0; i < 9; i++)
        {
            const float angle = -PI / 2.0f + i * PI / 4.0f;
            const float radius = (i % 2 == 0) ? outer : outer * 0.22f;

            points[i + 1] = {
                static_cast<float>(cx) + std::cos(angle) * radius,
                static_cast<float>(cy) + std::sin(angle) * radius
            };
        }

        DrawTriangleFan(points, 10, Fade(Theme::Muted, 0.18f));
    }

    void DrawHitFlash(int x, int y, float scale, float alpha)
    {
        if (alpha <= 0.0f) return;

        const float progress = 1.0f - alpha;
        const float radius = (28.0f + progress * 60.0f) * scale;

        DrawCircle(x, y, radius, Fade(WHITE, alpha * 0.12f));
        DrawCircleLines(x, y, radius, Fade(Theme::Gold, alpha));
        DrawCircleLines(x, y, radius * 0.6f, Fade(WHITE, alpha * 0.7f));

        for (int i = 0; i < 8; i++)
        {
            const float angle = i * PI / 4.0f;
            const float inner = radius * 0.55f;
            const float outer = radius * 1.05f;

            DrawLineEx(
                { static_cast<float>(x) + std::cos(angle) * inner,
                  static_cast<float>(y) + std::sin(angle) * inner },
                { static_cast<float>(x) + std::cos(angle) * outer,
                  static_cast<float>(y) + std::sin(angle) * outer },
                2.0f, Fade(WHITE, alpha));
        }
    }

    void DrawPartyActor(int index, Vector2 pos, int activeIndex, int attackingIndex, const Party& party)
    {
        const Character& member = party.Members()[index];
        const bool active = (index == activeIndex);
        const bool attacking = (index == attackingIndex);
        const float scale = 130.0f;
        const Texture2D texture = attacking ? PartyAttack : PartyIdle;

        if (member.hp <= 0)
        {
            if (SpritesLoaded)
            {
                DrawTexturePro(texture, { 0.0f, 0.0f, 32.0f, 32.0f },
                    { pos.x - scale / 2.0f, pos.y - scale / 2.0f, scale, scale },
                    { 0.0f, 0.0f }, 0.0f, Fade(WHITE, 0.22f));
            }

            DrawLineEx({ pos.x - 34.0f, pos.y - 34.0f }, { pos.x + 34.0f, pos.y + 34.0f },
                3.0f, Theme::Muted);
            DrawLineEx({ pos.x - 34.0f, pos.y + 34.0f }, { pos.x + 34.0f, pos.y - 34.0f },
                3.0f, Theme::Muted);
            return;
        }

        if (SpritesLoaded)
        {
            DrawTexturePro(texture, { 0.0f, 0.0f, 32.0f, 32.0f },
                { pos.x - scale / 2.0f, pos.y - scale / 2.0f, scale, scale },
                { 0.0f, 0.0f }, 0.0f, PartyTints[index]);
        }
        else
        {
            DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 34.0f, PartyTints[index]);
        }

        if (active)
        {
            DrawPoly({ pos.x, pos.y - 82.0f }, 3, 12.0f, 90.0f, Theme::Muted);
            DrawPolyLinesEx({ pos.x, pos.y - 82.0f }, 3, 12.0f, 90.0f, 2.0f, Theme::Ink);
        }
    }

    void DrawEnemyActor(int index, Vector2 pos, bool selected, const Enemy& enemy)
    {
        const float size = EnemySizes[index];

        if (enemy.IsAlive())
        {
            if (SpritesLoaded)
            {
                DrawTexturePro(Slime, { 0.0f, 0.0f, 32.0f, 32.0f },
                    { pos.x - size / 2.0f, pos.y - size / 2.0f, size, size },
                    { 0.0f, 0.0f }, 0.0f, WHITE);
            }
            else
            {
                DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), size / 2.0f,
                    Theme::Blue);
            }
        }
        else
        {
            DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), size / 2.0f,
                Theme::Muted);
            DrawLineEx({ pos.x - size / 3.0f, pos.y - size / 3.0f },
                { pos.x + size / 3.0f, pos.y + size / 3.0f }, 2.0f, Theme::Muted);
            DrawLineEx({ pos.x - size / 3.0f, pos.y + size / 3.0f },
                { pos.x + size / 3.0f, pos.y - size / 3.0f }, 2.0f, Theme::Muted);
        }

        if (enemy.IsAlive())
        {
            const bool lowHealth = (enemy.GetHp() * 4 <= enemy.GetMaxHp());

            DrawDiamond(pos.x, pos.y - size / 2.0f - 28.0f, 12.0f,
                lowHealth ? Theme::Red : Theme::Green);

            if (selected)
                DrawTargetCaret(pos.x, pos.y - size / 2.0f - 48.0f, Theme::Gold);
        }

        const std::string label = "E" + std::to_string(index + 1);
        DrawCentered(label.c_str(), static_cast<int>(pos.x),
            static_cast<int>(pos.y + size / 2.0f + 8.0f), 14,
            enemy.IsAlive() ? Theme::Muted : Fade(Theme::Muted, 0.5f));
    }

    void DrawPartyCards(const Battle& battle, int x, int y, int width)
    {
        const Party& party = battle.GetParty();
        const int activeIndex = party.GetActiveIndex();
        const int gap = 12;
        const int cardWidth = (width - gap) / 2;
        const int cardHeight = 119;

        for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
        {
            const Character& member = party.Members()[i];
            const int cardX = x + (i % 2) * (cardWidth + gap);
            const int cardY = y + (i / 2) * (cardHeight + gap);
            const bool active = (i == activeIndex);
            const bool smirk = member.smirk;
            const bool anxious = member.anxious;
            const Color background = smirk ? Theme::SmirkBg
                : (anxious ? Theme::AnxiousBg : Theme::Paper);

            const Rectangle card = {
                static_cast<float>(cardX),
                static_cast<float>(cardY),
                static_cast<float>(cardWidth),
                static_cast<float>(cardHeight)
            };

            DrawRectangleRec(card, background);
            DrawRectangleLinesEx(card, active ? 2.0f : 1.0f,
                active ? Theme::Gold : Theme::Ink);

            if (SpritesLoaded)
            {
                DrawTexturePro(PartyPortrait, { 0.0f, 0.0f, 32.0f, 32.0f },
                    { static_cast<float>(cardX + 10), static_cast<float>(cardY + 12), 54.0f, 54.0f },
                    { 0.0f, 0.0f }, 0.0f, PartyTints[i]);
            }
            else
            {
                DrawRectangle(cardX + 10, cardY + 12, 54, 54, PartyTints[i]);
            }

            UiDrawText(member.name, cardX + 76, cardY + 10, 22, Theme::Ink, true);
            UiDrawText(TextFormat("LV %d", member.level), cardX + cardWidth - 64,
                cardY + 12, 14, Theme::Muted);

            DrawBar(cardX + 76, cardY + 42, cardWidth - 100, 12,
                (member.maxHp > 0) ? static_cast<float>(member.hp) / member.maxHp : 0.0f,
                Theme::Green);
            DrawBar(cardX + 76, cardY + 59, cardWidth - 100, 5,
                (member.xpToNext > 0) ? static_cast<float>(member.xp) / member.xpToNext : 0.0f,
                Theme::Gold);

            UiDrawText(TextFormat("HP %d / %d", member.hp, member.maxHp),
                cardX + 76, cardY + 70, 14, Theme::Muted);

            if (smirk)
                UiDrawText("Smirk", cardX + cardWidth - 80, cardY + 8, 16, Theme::Green, true);
            else if (anxious)
                UiDrawText("Anxious", cardX + cardWidth - 90, cardY + 8, 16, Theme::Purple, true);

            if (member.hp <= 0)
            {
                DrawLineEx({ static_cast<float>(cardX + 14), static_cast<float>(cardY + 16) },
                    { static_cast<float>(cardX + 60), static_cast<float>(cardY + 62) },
                    2.0f, Theme::Muted);
                DrawLineEx({ static_cast<float>(cardX + 14), static_cast<float>(cardY + 62) },
                    { static_cast<float>(cardX + 60), static_cast<float>(cardY + 16) },
                    2.0f, Theme::Muted);
            }
        }
    }

    void DrawBattleVictory(const Battle& battle)
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        const Party& party = battle.GetParty();

        DrawBackdrop();

        DrawStarShape(500, 520, 330);

        UiDrawText("VICTORY", 90, 430, 150, Theme::Ink, true);

        DrawLine(1120, 60, 1120, height - 80, Fade(Theme::Ink, 0.35f));

        UiDrawText("Battle Result", 1180, 70, 28, Theme::Ink, true);

        int y = 150;

        for (const Character& member : party.Members())
        {
            if (SpritesLoaded)
            {
                DrawTexturePro(PartyPortrait, { 0.0f, 0.0f, 32.0f, 32.0f },
                    { 1180.0f, static_cast<float>(y), 46.0f, 46.0f },
                    { 0.0f, 0.0f }, 0.0f, WHITE);
            }

            UiDrawText(member.name, 1240, y + 4, 24, Theme::Ink, true);
            UiDrawText(TextFormat("LV %d", member.level), 1600, y + 8, 20, Theme::Muted);
            DrawLine(1240, y + 42, 1730, y + 42, Fade(Theme::Ink, 0.2f));

            y += 70;
        }

        UiDrawText(TextFormat("%d", battle.GetXpReward()), 1780, 140, 52, Theme::Ink, true);
        UiDrawText("EXP", 1780, 196, 20, Theme::Muted);
        UiDrawText(TextFormat("%d", battle.GetMoneyReward()), 1780, 270, 52, Theme::Ink, true);
        UiDrawText("MNY", 1780, 326, 20, Theme::Muted);

        int itemCount = battle.GetItemReward();
        if (battle.HasEquipmentReward()) itemCount++;

        UiDrawText(TextFormat("%d", itemCount), 1780, 400, 52, Theme::Ink, true);
        UiDrawText("ITEM", 1780, 456, 20, Theme::Muted);

        DrawLine(1160, 600, 1860, 600, Fade(Theme::Ink, 0.35f));
        UiDrawText(TextFormat("Item Gathered (x%d)", itemCount), 1180, 630, 24, Theme::Ink);

        if (battle.HasEquipmentReward())
        {
            UiDrawText(TextFormat("Equipment: %s",
                GetEquipment(battle.GetEquipmentReward()).name),
                1180, 665, 18, Theme::Gold);
        }

        const char* continueText = "Continue";
        UiDrawText(continueText, width - 90 - UiMeasureText(continueText, 38, true),
            height - 120, 38, Theme::Ink, true);
        UiDrawText("SPACE / ENTER", width - 90 - UiMeasureText("SPACE / ENTER", 16),
            height - 74, 16, Theme::Muted);
    }

    void DrawBattleDefeat()
    {
        DrawBackdrop();

        const int width = GetScreenWidth();
        const int height = GetScreenHeight();

        DrawRectangle(0, 0, width, height, Fade(Theme::Paper, 0.92f));
        DrawCentered("DEFEAT", width / 2, height / 2 - 120, 140, Theme::Red, true);
        DrawCentered("SPACE / ENTER   RETURN", width / 2, height / 2 + 50, 24, Theme::Ink);
    }
}

