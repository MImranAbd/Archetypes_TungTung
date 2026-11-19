#pragma once

#include "battle.h"
#include <raylib.h>

namespace BattleConfig
{
    constexpr int pointsPerMember = 2;
    constexpr int maxBuffStacks = 3;
    constexpr int buffDuration = 3;
    constexpr int battleEnemyCount = 5;
    constexpr int battleEnemyHp = 5;
    constexpr int battleEnemyHpCap = 100;
    constexpr int eliteEnemyHp = 100;
    constexpr int eliteEnemyDamage = 20;
    constexpr int eliteEnemyDamageStep = 10;
    constexpr int enemyDamageBase = 6;
    constexpr int enemyDamageCap = 14;
    constexpr float smirkChance = 0.30f;
    constexpr float anxietyChance = 0.15f;
}

namespace BattleInternal
{
    inline bool Chance(float probability)
    {
        return (GetRandomValue(0, 9999) / 10000.0f) < probability;
    }

    inline void ApplyBuff(BattleBuff& buff)
    {
        if (buff.stacks < BattleConfig::maxBuffStacks)
            buff.stacks++;

        buff.turns = BattleConfig::buffDuration;
    }

    inline void TickBuff(BattleBuff& buff)
    {
        if (buff.turns <= 0) return;

        buff.turns--;

        if (buff.turns == 0)
            buff.stacks = 0;
    }
}
