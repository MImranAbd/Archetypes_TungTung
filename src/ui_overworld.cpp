#include "ui.h"
#include "ui_internal.h"
#include "theme.h"
#include <raylib.h>

using namespace UiInternal;
AttackPlan MakeAttackPlan(const Ball& ball, const Enemy& enemy, const GameMap& map)
{
    const int dash = ball.IsRunning() ? 2 : (ball.IsMoving() ? 1 : 0);
    const int maxReach = dash + 1;

    int effectiveReach = 0;
    int hitDistance = 0;

    for (int d = 1; d <= maxReach; d++)
    {
        const int checkX = ball.GetGridX() + ball.GetFacingX() * d;
        const int checkY = ball.GetGridY() + ball.GetFacingY() * d;

        if (!map.IsInside(checkX, checkY)) break;
        if (map.IsWall(checkX, checkY)) break;

        effectiveReach = d;

        if (enemy.IsActive() &&
            enemy.GetGridX() == checkX &&
            enemy.GetGridY() == checkY)
        {
            hitDistance = d;
            break;
        }
    }

    AttackPlan plan;
    plan.reach = effectiveReach;
    plan.willHit = hitDistance > 0;

    if (hitDistance > 0)
    {
        plan.dashCells = hitDistance - 1;
        if (plan.dashCells > dash) plan.dashCells = dash;

        plan.targetX = enemy.GetGridX();
        plan.targetY = enemy.GetGridY();
    }
    else
    {
        plan.dashCells = dash;
        if (plan.dashCells > effectiveReach - 1)
            plan.dashCells = (effectiveReach > 0) ? effectiveReach - 1 : 0;

        plan.targetX = ball.GetGridX() + ball.GetFacingX() * effectiveReach;
        plan.targetY = ball.GetGridY() + ball.GetFacingY() * effectiveReach;
    }

    return plan;
}

void UI::DrawOverworld(const Ball& ball, const Enemy& enemy, const Party& party,
    const GameMap& map, bool pendingBattle, int dungeonFloor,
    bool showHud, const Camera2D& camera) const
{
    DrawBackdrop();
    BeginMode2D(camera);
    map.Draw();

    if (!pendingBattle)
        DrawAttackIndicator(ball, enemy, map);

    if (SpritesLoaded)
    {
        const float bx = ball.GetPixelX();
        const float by = ball.GetPixelY();
        const bool moving = ball.IsMoving();
        const int frame = moving
            ? static_cast<int>(GetTime() * 8.0f) % 4
            : 0;

        Texture2D texture = ElizaStand;
        bool flip = false;

        if (ball.GetFacingY() < 0)
        {
            texture = ElizaWalkUp[frame];
        }
        else if (ball.GetFacingY() == 0)
        {
            texture = ElizaWalkSide[frame];
            flip = (ball.GetFacingX() < 0);
        }
        else if (moving)
        {
            texture = ElizaWalkDown[frame];
        }

        Rectangle source = { 0.0f, 0.0f, 32.0f, 32.0f };

        if (flip)
        {
            source.x = 32.0f;
            source.width = -32.0f;
        }

        DrawTexturePro(texture, source,
            { bx - 25.0f, by - 25.0f, 50.0f, 50.0f },
            { 0.0f, 0.0f }, 0.0f, WHITE);

        if (enemy.IsActive())
        {
            const float ex = enemy.GetPixelX();
            const float ey = enemy.GetPixelY();

            DrawTexturePro(Slime, { 0.0f, 0.0f, 32.0f, 32.0f },
                { ex - 21.0f, ey - 21.0f, 42.0f, 42.0f },
                { 0.0f, 0.0f }, 0.0f, WHITE);
        }
    }
    else
    {
        ball.Draw();
        enemy.Draw();
    }

    DrawSlash();
    EndMode2D();

    if (!showHud)
        return;

    const int width = GetScreenWidth();
    const int height = GetScreenHeight();

    DrawRectangle(0, 0, width, 78, Fade(Theme::Parchment, 0.96f));
    DrawRectangle(0, 77, width, 1, Fade(Theme::Ink, 0.4f));

    DrawRectangle(0, height - 60, width, 60, Fade(Theme::Parchment, 0.96f));
    DrawRectangle(0, height - 60, width, 1, Fade(Theme::Ink, 0.4f));

    const char* floorText = TextFormat("DUNGEON FLOOR %d", dungeonFloor);
    DrawCentered(floorText, width / 2, 30, 20, Theme::Muted);

    UiDrawText(TextFormat("PARTY  %d / %d", party.GetTotalHp(), party.GetTotalMaxHp()),
        24, 30, 20, Theme::Ink);

    const Rectangle partyBar = { 24.0f, 58.0f, 220.0f, 10.0f };
    DrawHpBar(partyBar, party.GetTotalHp(), party.GetTotalMaxHp(), Theme::Forest);

    if (enemy.IsActive())
    {
        UiDrawText(TextFormat("FOE  %d / %d", enemy.GetHp(), enemy.GetMaxHp()),
            width - 244, 30, 20, Theme::Ink);

        const Rectangle enemyBar = { static_cast<float>(width) - 244.0f, 58.0f, 220.0f, 10.0f };
        DrawHpBar(enemyBar, enemy.GetHp(), enemy.GetMaxHp(), Theme::Burgundy);
    }
    else
    {
        UiDrawText("CLEARED", width - 244, 30, 20, Theme::Muted);
    }

    UiDrawText("WASD  MOVE     SHIFT  RUN     SPACE  SLASH     P / TAB  MENU     I  ITEMS",
        24, height - 42, 18, Theme::Muted);

    const int reach = ball.IsRunning() ? 3 : (ball.IsMoving() ? 2 : 1);
    const char* reachText = TextFormat("REACH  %d", reach);
    UiDrawText(reachText, width - UiMeasureText(reachText, 18) - 24, height - 42, 18, Theme::Muted);
}

void UI::DrawAttackIndicator(const Ball& ball, const Enemy& enemy, const GameMap& map) const
{
    const AttackPlan plan = MakeAttackPlan(ball, enemy, map);
    constexpr int cell = Ball::cellSize;

    if (plan.reach <= 0) return;

    for (int d = 1; d <= plan.reach; d++)
    {
        const int cellX = ball.GetGridX() + ball.GetFacingX() * d;
        const int cellY = ball.GetGridY() + ball.GetFacingY() * d;

        DrawRectangleLines(cellX * cell, cellY * cell, cell, cell, Fade(Theme::Ink, 0.18f));
    }

    for (int d = 1; d <= plan.dashCells; d++)
    {
        const int cellX = ball.GetGridX() + ball.GetFacingX() * d;
        const int cellY = ball.GetGridY() + ball.GetFacingY() * d;

        DrawRectangle(cellX * cell, cellY * cell, cell, cell, Fade(Theme::Gold, 0.14f));
    }

    const int px = plan.targetX * cell;
    const int py = plan.targetY * cell;
    const Rectangle targetRect = {
        static_cast<float>(px),
        static_cast<float>(py),
        static_cast<float>(cell),
        static_cast<float>(cell)
    };

    DrawRectangleRec(targetRect, Fade(Theme::Gold, 0.18f));
    DrawRectangleLinesEx(targetRect, 2.0f, Theme::Ink);

    if (plan.willHit)
    {
        DrawLine(px + 10, py + 10, px + cell - 10, py + cell - 10, Theme::Burgundy);
        DrawLine(px + cell - 10, py + 10, px + 10, py + cell - 10, Theme::Burgundy);
    }
}

void UI::DrawSlash() const
{
    if (slashTimer <= 0.0f) return;

    const float progress = 1.0f - slashTimer / slashDuration;
    const float alpha = slashTimer / slashDuration;
    constexpr int cell = Ball::cellSize;

    const float cx = slashCellX * cell + cell / 2.0f;
    const float cy = slashCellY * cell + cell / 2.0f;
    const float offset = (progress - 0.5f) * 12.0f;

    const Vector2 from1 = { cx - 20.0f, cy - 20.0f + offset };
    const Vector2 to1 = { cx + 20.0f, cy + 20.0f + offset };
    const Vector2 from2 = { cx - 14.0f, cy - 24.0f + offset };
    const Vector2 to2 = { cx + 24.0f, cy + 14.0f + offset };

    DrawLineEx(from1, to1, 6.0f, Fade(Theme::Ink, alpha));
    DrawLineEx(from2, to2, 2.0f, Fade(Theme::Gold, alpha));
}


