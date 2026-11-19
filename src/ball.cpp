#include "ball.h"
#include "map.h"
#include "theme.h"
#include <raylib.h>
#include <cmath>

Ball::Ball()
    : gridX(2)
    , gridY(2)
    , x(0)
    , y(0)
    , repeatTimer(0.0f)
    , facingX(1)
    , facingY(0)
    , moving(false)
    , running(false)
    , exitDirX(0)
    , exitDirY(0)
    , radius(15)
{
    x = gridX * cellSize + cellSize / 2.0f;
    y = gridY * cellSize + cellSize / 2.0f;
}

void Ball::Update(const GameMap& map, int blockedX, int blockedY)
{
    int inputX = 0;
    int inputY = 0;

    if (IsKeyDown(KEY_D)) inputX = 1;
    if (IsKeyDown(KEY_A)) inputX = -1;
    if (IsKeyDown(KEY_S)) inputY = 1;
    if (IsKeyDown(KEY_W)) inputY = -1;

    const bool pressingHorizontal = inputX != 0;
    const bool pressingVertical = inputY != 0;

    moving = false;

    if (pressingHorizontal && pressingVertical)
    {
        moving = true;

        if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_A))
        {
            facingX = inputX;
            facingY = 0;
        }
        else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_W))
        {
            facingX = 0;
            facingY = inputY;
        }
    }
    else if (pressingHorizontal)
    {
        moving = true;
        facingX = inputX;
        facingY = 0;
    }
    else if (pressingVertical)
    {
        moving = true;
        facingX = 0;
        facingY = inputY;
    }

    running = moving && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));

    const int step = running ? 2 : 1;

    if (moving)
    {
        repeatTimer -= GetFrameTime();

        if (repeatTimer <= 0.0f)
        {
            int remaining = step;

            while (remaining > 0)
            {
                const int nextX = gridX + facingX;
                const int nextY = gridY + facingY;

                if (!map.IsInside(nextX, nextY))
                {
                    exitDirX = facingX;
                    exitDirY = facingY;
                    break;
                }

                if (map.IsWall(nextX, nextY))
                    break;

                if (nextX == blockedX && nextY == blockedY)
                    break;

                gridX = nextX;
                gridY = nextY;
                remaining--;
            }

            repeatTimer = repeatDelay;
        }
    }
    else
    {
        repeatTimer = 0.0f;
    }

    UpdateVisual();
}

void Ball::UpdateVisual()
{
    const float targetX = gridX * cellSize + cellSize / 2.0f;
    const float targetY = gridY * cellSize + cellSize / 2.0f;

    const float t = 1.0f - std::exp(-12.0f * GetFrameTime());
    x += (targetX - x) * t;
    y += (targetY - y) * t;
}

void Ball::Draw() const
{
    DrawCircle(x, y, radius, Theme::Ink);
}

void Ball::Dash(int cells, const GameMap& map)
{
    for (int i = 0; i < cells; i++)
    {
        const int nextX = gridX + facingX;
        const int nextY = gridY + facingY;

        if (!map.IsInside(nextX, nextY)) break;
        if (map.IsWall(nextX, nextY)) break;

        gridX = nextX;
        gridY = nextY;
    }
}

void Ball::SetGridPosition(int newGridX, int newGridY)
{
    gridX = newGridX;
    gridY = newGridY;
    x = gridX * cellSize + cellSize / 2.0f;
    y = gridY * cellSize + cellSize / 2.0f;
    repeatTimer = 0.0f;
    moving = false;
    exitDirX = 0;
    exitDirY = 0;
}

bool Ball::ConsumeExit(int& dirX, int& dirY)
{
    if (exitDirX == 0 && exitDirY == 0)
        return false;

    dirX = exitDirX;
    dirY = exitDirY;
    exitDirX = 0;
    exitDirY = 0;
    return true;
}

int Ball::GetGridX() const
{
    return gridX;
}

int Ball::GetGridY() const
{
    return gridY;
}

float Ball::GetPixelX() const
{
    return x;
}

float Ball::GetPixelY() const
{
    return y;
}

int Ball::GetFacingX() const
{
    return facingX;
}

int Ball::GetFacingY() const
{
    return facingY;
}

bool Ball::IsMoving() const
{
    return moving;
}

bool Ball::IsRunning() const
{
    return running;
}
