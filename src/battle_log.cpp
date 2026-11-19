#include "battle.h"
#include "battle_internal.h"
#include <iostream>
#include <string>
#include <vector>

using namespace BattleConfig;
using namespace BattleInternal;
void Battle::LogState() const
{
    std::cout << "[STATE] Party:";

    for (const Character& member : party.Members())
        std::cout << " " << member.name << " " << member.hp << "/" << member.maxHp;

    std::cout << " | Active: " << party.Active().name;
    std::cout << " | Enemies:";

    for (int i = 0; i < static_cast<int>(enemies.size()); i++)
    {
        std::cout << " E" << (i + 1) << " " << enemies[i].GetHp()
                  << "/" << enemies[i].GetMaxHp();
    }

    std::cout << " | Press Turn P:" << (playerPoints / 2.0f)
              << " E:" << (enemyPoints / 2.0f)
              << " | Buffs";

    for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
    {
        const BattleBuffs& b = partyBuffs[i];
        if (b.attackUp.stacks == 0 && b.defenseUp.stacks == 0 && b.evasionUp.stacks == 0)
            continue;

        std::cout << " " << party.Members()[i].name << ":";
        if (b.attackUp.stacks > 0) std::cout << " ATK+" << b.attackUp.stacks << "(" << b.attackUp.turns << "t)";
        if (b.defenseUp.stacks > 0) std::cout << " DEF+" << b.defenseUp.stacks << "(" << b.defenseUp.turns << "t)";
        if (b.evasionUp.stacks > 0) std::cout << " EVA+" << b.evasionUp.stacks << "(" << b.evasionUp.turns << "t)";
    }

    for (int i = 0; i < static_cast<int>(enemies.size()); i++)
    {
        const BattleBuffs& b = enemyBuffs[i];
        if (b.enemyAttackDown.stacks == 0 &&
            b.enemyDefenseDown.stacks == 0 &&
            b.enemyAccuracyDown.stacks == 0)
            continue;

        std::cout << " E" << (i + 1) << ":";
        if (b.enemyAttackDown.stacks > 0) std::cout << " ATK-" << b.enemyAttackDown.stacks << "(" << b.enemyAttackDown.turns << "t)";
        if (b.enemyDefenseDown.stacks > 0) std::cout << " DEF-" << b.enemyDefenseDown.stacks << "(" << b.enemyDefenseDown.turns << "t)";
        if (b.enemyAccuracyDown.stacks > 0) std::cout << " ACC-" << b.enemyAccuracyDown.stacks << "(" << b.enemyAccuracyDown.turns << "t)";
    }

    std::cout << " | Status Party";

    bool anyStatus = false;

    for (const Character& member : party.Members())
    {
        if (member.smirk)
        {
            std::cout << " " << member.name << " SMIRK";
            anyStatus = true;
        }
        else if (member.anxious)
        {
            std::cout << " " << member.name << " ANXIOUS";
            anyStatus = true;
        }
    }

    if (!anyStatus) std::cout << " normal";
    std::cout << " | Enemy ";

    if (enemySmirk) std::cout << "SMIRK";
    else if (enemyAnxious) std::cout << "ANXIOUS";
    else std::cout << "normal";

    std::cout << "\n";
}



