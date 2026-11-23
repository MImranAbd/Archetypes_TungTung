#include "battle.h"
#include "battle_internal.h"
#include <raylib.h>
#include <iostream>
#include <string>
#include <vector>

using namespace BattleConfig;
using namespace BattleInternal;

namespace
{
    enum class EnemyMove
    {
        Strike,
        Heavy,
        Sweep
    };

    EnemyMove ChooseEnemyMove()
    {
        const int roll = GetRandomValue(0, 99);

        if (roll < 55) return EnemyMove::Strike;
        if (roll < 82) return EnemyMove::Heavy;

        return EnemyMove::Sweep;
    }

    const char* EnemyMoveName(EnemyMove move)
    {
        switch (move)
        {
            case EnemyMove::Strike: return "Strike";
            case EnemyMove::Heavy:  return "Heavy Strike";
            case EnemyMove::Sweep:  return "Sweep";
        }

        return "Attack";
    }

    int MoveDamage(int baseDamage, EnemyMove move)
    {
        if (move == EnemyMove::Heavy)
            return baseDamage * 3 / 2 + 1;

        if (move == EnemyMove::Sweep)
            return (baseDamage * 2 + 2) / 3;

        return baseDamage;
    }
}

void Battle::ResolveEnemyAttack()
{
    if (AllEnemiesDead())
    {
        phase = Phase::Victory;
        return;
    }

    if (!enemies[enemyTurnIndex].IsAlive())
    {
        const int next = FindNextAliveEnemy(enemyTurnIndex);

        if (next < 0)
        {
            phase = Phase::Victory;
            return;
        }

        enemyTurnIndex = next;
    }

    std::vector<int> livingTargets;

    for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
    {
        if (party.Members()[i].hp > 0)
            livingTargets.push_back(i);
    }

    if (livingTargets.empty())
    {
        phase = Phase::Defeat;
        return;
    }

    const bool smirk = enemySmirk;
    const bool anxious = enemyAnxious;
    enemySmirk = false;
    enemyAnxious = false;

    const Element element = static_cast<Element>(GetRandomValue(0, 2));
    const EnemyMove move = ChooseEnemyMove();

    actionCinematic = true;
    actionWideParty = false;
    actionAttackPose = true;
    supportFlashTimer = 0.0f;
    actionTexts.clear();
    actionBanner = std::string("Enemy ") + EnemyMoveName(move);
    actionBannerTimer = Battle::actionDuration;

    lastActorParty = -1;
    lastActorEnemy = enemyTurnIndex;

    const std::string enemyName = "E" + std::to_string(enemyTurnIndex + 1);

    std::vector<int> targets;

    if (move == EnemyMove::Sweep)
    {
        targets = livingTargets;
        lastActionHasTarget = false;
        lastActionTargetOnParty = false;
        lastActionTargetIndex = -1;
    }
    else
    {
        const int targetIndex =
            livingTargets[GetRandomValue(0, static_cast<int>(livingTargets.size()) - 1)];

        targets.push_back(targetIndex);
        lastActionHasTarget = true;
        lastActionTargetOnParty = true;
        lastActionTargetIndex = targetIndex;
    }

    const int primaryTarget = targets[0];
    const Character& primary = party.Members()[primaryTarget];

    float hitChance = (move == EnemyMove::Heavy) ? 0.70f : 0.85f;
    if (smirk) hitChance += 0.10f;
    if (anxious) hitChance -= 0.25f;
    if (primary.smirk) hitChance -= 0.30f;
    if (primary.anxious) hitChance += 0.20f;
    if (partyBuffs[primaryTarget].evasionUp.stacks > 0)
        hitChance -= 0.15f * partyBuffs[primaryTarget].evasionUp.stacks;
    if (enemyBuffs[enemyTurnIndex].enemyAccuracyDown.stacks > 0)
        hitChance -= 0.15f * enemyBuffs[enemyTurnIndex].enemyAccuracyDown.stacks;

    if (hitChance < 0.05f) hitChance = 0.05f;
    if (hitChance > 0.99f) hitChance = 0.99f;

    if (!Chance(hitChance))
    {
        enemyPoints -= 4;

        message = enemyName + " uses " + EnemyMoveName(move) + " - MISS!";

        if (Chance(anxietyChance))
        {
            enemyAnxious = true;
            message += " ENEMY ANXIOUS!";
        }

        std::cout << "[ENEMY] " << enemyName << " " << EnemyMoveName(move)
                  << " MISS | " << message << "\n";
    }
    else
    {
        int totalDamage = 0;
        bool anyWeak = false;
        bool anyCrit = false;
        bool flashed = false;
        std::vector<int> fallen;

        for (int targetIndex : targets)
        {
            Character& target = party.Members()[targetIndex];

            float critChance = 0.10f;
            if (smirk) critChance += 0.40f;
            if (anxious) critChance -= 0.05f;
            if (critChance < 0.0f) critChance = 0.0f;

            const bool crit = Chance(critChance);
            const bool weak = (element == target.weakness);

            int damage = MoveDamage(enemyBaseDamage, move);
            damage -= target.defense / 3;

            if (enemyBuffs[enemyTurnIndex].enemyAttackDown.stacks > 0)
                damage -= enemyBuffs[enemyTurnIndex].enemyAttackDown.stacks;

            if (partyBuffs[targetIndex].defenseUp.stacks > 0)
                damage -= partyBuffs[targetIndex].defenseUp.stacks;

            if (defending) damage -= 3;
            if (weak) damage += 1;
            if (crit) damage += 1;
            if (damage < 1) damage = 1;

            target.hp -= damage;
            if (target.hp < 0) target.hp = 0;

            AddDamagePopup(targetIndex, true, damage, crit, weak);

            if (!flashed)
            {
                hitFlashTimer = Battle::hitFlashDuration;
                hitFlashTarget = targetIndex;
                hitFlashOnParty = true;
                flashed = true;
            }

            if (target.hp <= 0)
            {
                target.smirk = false;
                target.anxious = false;
                fallen.push_back(targetIndex);
            }

            totalDamage += damage;
            anyWeak = anyWeak || weak;
            anyCrit = anyCrit || crit;

            std::cout << "[ENEMY] " << enemyName << " " << EnemyMoveName(move)
                      << " -> " << target.name << " " << ElementName(element)
                      << " " << damage << " DMG"
                      << (weak ? " WEAK" : "") << (crit ? " CRIT" : "") << "\n";
        }

        std::string prefix;
        if (anyWeak) prefix += "WEAK! ";
        if (anyCrit) prefix += "CRITICAL! ";

        if (move == EnemyMove::Sweep)
        {
            message = prefix + enemyName + " sweeps the party for "
                + std::to_string(totalDamage) + " total damage.";
        }
        else
        {
            message = prefix + enemyName + " uses " + EnemyMoveName(move) + " on "
                + party.Members()[primaryTarget].name + " for "
                + std::to_string(totalDamage) + " damage.";
        }

        for (int targetIndex : fallen)
            message += " " + std::string(party.Members()[targetIndex].name) + " FALLS!";

        if ((anyWeak || anyCrit) && Chance(smirkChance))
        {
            enemySmirk = true;
            message += " ENEMY SMIRK!";
        }

        int cost = (move == EnemyMove::Strike) ? 2 : 3;
        if (anyWeak || anyCrit) cost--;

        enemyPoints -= cost;
    }

    if (enemyPoints < 0) enemyPoints = 0;

    LogState();

    if (party.IsWipedOut())
    {
        phase = Phase::Defeat;
        message = "THE PARTY FALLS...";
        std::cout << "=== DEFEAT ===\n";
        return;
    }

    if (enemyPoints <= 0)
    {
        StartPlayerTurn();
        return;
    }

    const int next = FindNextAliveEnemy((enemyTurnIndex + 1) % static_cast<int>(enemies.size()));

    if (next < 0)
    {
        phase = Phase::Victory;
        return;
    }

    enemyTurnIndex = next;
    turnTimer = 1.4f;
}
