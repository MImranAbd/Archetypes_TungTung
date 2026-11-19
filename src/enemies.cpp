#include "enemies.h"
#include "map.h"
#include "theme.h"
#include <raylib.h>
#include <cmath>
#include <cstdlib>

Enemy::Enemy(int startGridX, int startGridY, int maxHp)
    : gridX(startGridX)
    , gridY(startGridY)
    , x(0)
    , y(0)
    , moveTimer(0.0f)
    , radius(15)
    , hp(maxHp)
    , maxHp(maxHp)
    , active(true)
    , weakness(static_cast<Element>(GetRandomValue(0, 2)))
{
    x = gridX * cellSize + cellSize / 2.0f;
    y = gridY * cellSize + cellSize / 2.0f;
}

void Enemy::Update(int targetGridX, int targetGridY, const GameMap& map)
{
    if (!active) return;

    moveTimer -= GetFrameTime();

    if (moveTimer <= 0.0f)
    {
        const int dx = targetGridX - gridX;
        const int dy = targetGridY - gridY;

        auto tryStep = [&](int nextX, int nextY) -> bool
        {
            if (!map.IsInside(nextX, nextY)) return false;
            if (map.IsWall(nextX, nextY)) return false;

            gridX = nextX;
            gridY = nextY;
            return true;
        };

        if (std::abs(dx) >= std::abs(dy) && dx != 0)
        {
            if (!tryStep(gridX + ((dx > 0) ? 1 : -1), gridY) && dy != 0)
                tryStep(gridX, gridY + ((dy > 0) ? 1 : -1));
        }
        else if (dy != 0)
        {
            if (!tryStep(gridX, gridY + ((dy > 0) ? 1 : -1)) && dx != 0)
                tryStep(gridX + ((dx > 0) ? 1 : -1), gridY);
        }

        moveTimer = moveDelay;
    }

    const float targetX = gridX * cellSize + cellSize / 2.0f;
    const float targetY = gridY * cellSize + cellSize / 2.0f;

    const float t = 1.0f - std::exp(-12.0f * GetFrameTime());
    x += (targetX - x) * t;
    y += (targetY - y) * t;
}

void Enemy::Draw() const
{
    if (!active) return;

    DrawCircle(x, y, radius, Theme::Burgundy);
    DrawCircle(x, y, radius - 4, Theme::Parchment);
    DrawCircle(x, y, 3, Theme::Burgundy);
}

int Enemy::GetGridX() const
{
    return gridX;
}

int Enemy::GetGridY() const
{
    return gridY;
}

float Enemy::GetPixelX() const
{
    return x;
}

float Enemy::GetPixelY() const
{
    return y;
}

int Enemy::GetHp() const
{
    return hp;
}

int Enemy::GetMaxHp() const
{
    return maxHp;
}

bool Enemy::IsAlive() const
{
    return hp > 0;
}

bool Enemy::IsActive() const
{
    return active;
}

void Enemy::SetActive(bool value)
{
    active = value;
}

void Enemy::TakeDamage(int amount)
{
    hp -= amount;

    if (hp <= 0)
    {
        hp = 0;
        active = false;
    }
}

void Enemy::Reset(int startGridX, int startGridY, int newMaxHp)
{
    gridX = startGridX;
    gridY = startGridY;
    x = gridX * cellSize + cellSize / 2.0f;
    y = gridY * cellSize + cellSize / 2.0f;
    moveTimer = 0.0f;
    hp = newMaxHp;
    maxHp = newMaxHp;
    active = true;
    weakness = static_cast<Element>(GetRandomValue(0, 2));
}

Element Enemy::GetWeakness() const
{
    return weakness;
}
