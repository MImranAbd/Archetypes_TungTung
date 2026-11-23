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
    const EnemySkillDef basicAttack = {
        "Attack", EnemyActionKind::BasicAttack, Element::Physical,
        EnemyStat::None, 0, 0.85f, 2
    };

    bool IsDamageAction(EnemyActionKind kind)
    {
        return kind == EnemyActionKind::BasicAttack ||
            kind == EnemyActionKind::Damage ||
            kind == EnemyActionKind::DamageAll ||
            kind == EnemyActionKind::Drain;
    }

    Color EnemySupportColor(EnemyActionKind kind, EnemyStat stat)
    {
        if (kind == EnemyActionKind::HealSelf ||
            kind == EnemyActionKind::HealAlly ||
            kind == EnemyActionKind::HealAllies)
            return { 120, 210, 140, 255 };

        if (kind == EnemyActionKind::BuffSelf ||
            kind == EnemyActionKind::BuffAllies)
        {
            switch (stat)
            {
                case EnemyStat::Defense: return { 120, 185, 135, 255 };
                case EnemyStat::Evasion: return { 165, 135, 225, 255 };
                default: return { 225, 130, 95, 255 };
            }
        }

        if (kind == EnemyActionKind::DebuffParty)
        {
            switch (stat)
            {
                case EnemyStat::Defense: return { 215, 165, 85, 255 };
                case EnemyStat::Accuracy: return { 170, 120, 210, 255 };
                default: return { 215, 95, 85, 255 };
            }
        }

        return WHITE;
    }

    const char* StatActionText(EnemyStat stat, bool up)
    {
        if (up)
        {
            switch (stat)
            {
                case EnemyStat::Attack: return "ATK UP";
                case EnemyStat::Defense: return "DEF UP";
                case EnemyStat::Evasion: return "EVA UP";
                default: return "POWER UP";
            }
        }

        switch (stat)
        {
            case EnemyStat::Attack: return "ATK DOWN";
            case EnemyStat::Defense: return "DEF DOWN";
            case EnemyStat::Accuracy: return "ACC DOWN";
            default: return "WEAKENED";
        }
    }

    SkillKind BuffFxKind(EnemyStat stat)
    {
        switch (stat)
        {
            case EnemyStat::Defense: return SkillKind::BuffDefense;
            case EnemyStat::Evasion: return SkillKind::BuffEvasion;
            default: return SkillKind::BuffAttack;
        }
    }

    SkillKind DebuffFxKind(EnemyStat stat)
    {
        switch (stat)
        {
            case EnemyStat::Defense: return SkillKind::DebuffDefense;
            case EnemyStat::Accuracy: return SkillKind::DebuffAccuracy;
            default: return SkillKind::DebuffAttack;
        }
    }

    BattleBuff* EnemyBuffForStat(BattleBuffs& set, EnemyStat stat)
    {
        switch (stat)
        {
            case EnemyStat::Attack: return &set.attackUp;
            case EnemyStat::Defense: return &set.defenseUp;
            case EnemyStat::Evasion: return &set.evasionUp;
            default: return nullptr;
        }
    }

    BattleBuff* PartyDebuffForStat(BattleBuffs& set, EnemyStat stat)
    {
        switch (stat)
        {
            case EnemyStat::Attack: return &set.enemyAttackDown;
            case EnemyStat::Defense: return &set.enemyDefenseDown;
            case EnemyStat::Accuracy: return &set.enemyAccuracyDown;
            default: return nullptr;
        }
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

    const EnemyDef& def = GetEnemyDef(enemies[enemyTurnIndex].GetKind());
    const std::string enemyName = "E" + std::to_string(enemyTurnIndex + 1);

    int weights[3] = { 40, 0, 0 };

    for (int s = 0; s < 2; s++)
    {
        const EnemySkillDef& skill = def.skills[s];
        int weight = 0;

        switch (skill.kind)
        {
            case EnemyActionKind::Damage:
            case EnemyActionKind::Drain:
                weight = 45;
                break;

            case EnemyActionKind::DamageAll:
                weight = 30;
                break;

            case EnemyActionKind::HealSelf:
            {
                const Enemy& self = enemies[enemyTurnIndex];
                weight = (self.GetHp() * 2 <= self.GetMaxHp()) ? 100 : 0;
            } break;

            case EnemyActionKind::HealAlly:
            {
                bool wounded = false;

                for (const Enemy& ally : enemies)
                {
                    if (ally.IsAlive() && ally.GetHp() * 2 <= ally.GetMaxHp())
                    {
                        wounded = true;
                        break;
                    }
                }

                weight = wounded ? 100 : 0;
            } break;

            case EnemyActionKind::HealAllies:
            {
                int wounded = 0;

                for (const Enemy& ally : enemies)
                {
                    if (ally.IsAlive() && ally.GetHp() < ally.GetMaxHp())
                        wounded++;
                }

                weight = (wounded >= 2) ? 120 : (wounded == 1 ? 50 : 0);
            } break;

            case EnemyActionKind::BuffSelf:
            case EnemyActionKind::BuffAllies:
            {
                bool anyLow = false;

                for (int i = 0; i < static_cast<int>(enemies.size()); i++)
                {
                    if (!enemies[i].IsAlive()) continue;
                    if (skill.kind == EnemyActionKind::BuffSelf &&
                        i != enemyTurnIndex) continue;

                    BattleBuff* buff = EnemyBuffForStat(enemyBuffs[i], skill.stat);

                    if (buff != nullptr && buff->stacks < 3)
                    {
                        anyLow = true;
                        break;
                    }
                }

                weight = anyLow ? 60 : 5;
            } break;

            case EnemyActionKind::DebuffParty:
            {
                bool anyClear = false;

                for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
                {
                    if (party.Members()[i].hp <= 0) continue;

                    BattleBuff* debuff = PartyDebuffForStat(partyBuffs[i], skill.stat);

                    if (debuff != nullptr && debuff->stacks == 0)
                    {
                        anyClear = true;
                        break;
                    }
                }

                weight = anyClear ? 60 : 10;
            } break;

            case EnemyActionKind::BasicAttack:
                break;
        }

        weights[s + 1] = weight;
    }

    const int totalWeight = weights[0] + weights[1] + weights[2];
    int roll = GetRandomValue(0, totalWeight > 0 ? totalWeight - 1 : 0);
    int chosen = 0;

    for (int i = 0; i < 3; i++)
    {
        if (roll < weights[i])
        {
            chosen = i;
            break;
        }

        roll -= weights[i];
    }

    const bool isBasic = (chosen == 0);
    const EnemySkillDef& action = isBasic ? basicAttack : def.skills[chosen - 1];

    const bool smirk = enemySmirk;
    const bool anxious = enemyAnxious;
    enemySmirk = false;
    enemyAnxious = false;

    actionCinematic = true;
    actionWideParty = false;
    actionAttackPose = IsDamageAction(action.kind);
    supportFlashTimer = 0.0f;
    actionTexts.clear();

    const Color supportColor = EnemySupportColor(action.kind, action.stat);

    if (!actionAttackPose)
    {
        supportFlashTimer = Battle::supportFlashDuration;
        supportFlashColor = supportColor;
    }

    actionBanner = isBasic
        ? "Enemy Attack"
        : std::string("Enemy ") + action.name;
    actionBannerTimer = Battle::actionDuration;

    lastActorParty = -1;
    lastActorEnemy = enemyTurnIndex;

    std::vector<int> targets;

    switch (action.kind)
    {
        case EnemyActionKind::BasicAttack:
        case EnemyActionKind::Damage:
        case EnemyActionKind::Drain:
        {
            const int targetIndex =
                livingTargets[GetRandomValue(0, static_cast<int>(livingTargets.size()) - 1)];

            targets.push_back(targetIndex);
            lastActionHasTarget = true;
            lastActionTargetOnParty = true;
            lastActionTargetIndex = targetIndex;
        } break;

        case EnemyActionKind::DamageAll:
        {
            targets = livingTargets;
            lastActionHasTarget = false;
            lastActionTargetOnParty = false;
            lastActionTargetIndex = -1;
        } break;

        case EnemyActionKind::HealSelf:
        case EnemyActionKind::BuffSelf:
        {
            lastActionHasTarget = true;
            lastActionTargetOnParty = false;
            lastActionTargetIndex = enemyTurnIndex;
        } break;

        case EnemyActionKind::HealAlly:
        {
            int target = enemyTurnIndex;
            int lowestHp = 9999;

            for (int i = 0; i < static_cast<int>(enemies.size()); i++)
            {
                if (!enemies[i].IsAlive()) continue;

                if (enemies[i].GetHp() < lowestHp)
                {
                    lowestHp = enemies[i].GetHp();
                    target = i;
                }
            }

            lastActionHasTarget = true;
            lastActionTargetOnParty = false;
            lastActionTargetIndex = target;
        } break;

        case EnemyActionKind::HealAllies:
        case EnemyActionKind::BuffAllies:
        case EnemyActionKind::DebuffParty:
        {
            lastActionHasTarget = false;
            lastActionTargetOnParty = false;
            lastActionTargetIndex = -1;
        } break;
    }

    auto spendEnemyCost = [&](int cost, bool grantsHalf)
    {
        if (cost >= 3)
        {
            SpendEnemyFullPoints();
            SpendEnemyHalfPoint(false);
        }
        else if (cost == 2)
        {
            SpendEnemyFullPoints();
        }
        else if (cost == 1)
        {
            SpendEnemyHalfPoint(grantsHalf);
        }
    };

    if (!IsDamageAction(action.kind))
    {
        int affected = 0;
        int healedTotal = 0;

        switch (action.kind)
        {
            case EnemyActionKind::HealSelf:
            {
                Enemy& self = enemies[enemyTurnIndex];
                const int missing = self.GetMaxHp() - self.GetHp();
                const int healed = (action.power < missing) ? action.power : missing;

                self.Heal(healed);
                healedTotal += healed;
                affected = 1;

                AddActionText("+" + std::to_string(healed), enemyTurnIndex, false,
                    supportColor);
                SetBuffEffect(SkillKind::Heal, false, enemyTurnIndex);
            } break;

            case EnemyActionKind::HealAlly:
            {
                const int target = lastActionTargetIndex;

                if (target >= 0 && target < static_cast<int>(enemies.size()) &&
                    enemies[target].IsAlive())
                {
                    Enemy& ally = enemies[target];
                    const int missing = ally.GetMaxHp() - ally.GetHp();
                    const int healed = (action.power < missing) ? action.power : missing;

                    ally.Heal(healed);
                    healedTotal += healed;
                    affected = 1;

                    AddActionText("+" + std::to_string(healed), target, false,
                        supportColor);
                    SetBuffEffect(SkillKind::Heal, false, target);
                }
            } break;

            case EnemyActionKind::HealAllies:
            {
                for (int i = 0; i < static_cast<int>(enemies.size()); i++)
                {
                    if (!enemies[i].IsAlive()) continue;

                    const int missing = enemies[i].GetMaxHp() - enemies[i].GetHp();
                    if (missing <= 0) continue;

                    const int healed = (action.power < missing) ? action.power : missing;

                    enemies[i].Heal(healed);
                    healedTotal += healed;
                    affected++;

                    AddActionText("+" + std::to_string(healed), i, false, supportColor);
                }

                SetBuffEffect(SkillKind::HealAll, false, -1);
            } break;

            case EnemyActionKind::BuffSelf:
            {
                BattleBuff* buff = EnemyBuffForStat(
                    enemyBuffs[enemyTurnIndex], action.stat);

                if (buff != nullptr)
                {
                    ApplyBuff(*buff);
                    affected = 1;

                    AddActionText(StatActionText(action.stat, true), enemyTurnIndex,
                        false, supportColor);
                    SetBuffEffect(BuffFxKind(action.stat), false, enemyTurnIndex);
                }
            } break;

            case EnemyActionKind::BuffAllies:
            {
                for (int i = 0; i < static_cast<int>(enemies.size()); i++)
                {
                    if (!enemies[i].IsAlive()) continue;

                    BattleBuff* buff = EnemyBuffForStat(enemyBuffs[i], action.stat);

                    if (buff == nullptr) continue;

                    ApplyBuff(*buff);
                    affected++;

                    AddActionText(StatActionText(action.stat, true), i, false,
                        supportColor);
                }

                SetBuffEffect(BuffFxKind(action.stat), false, -1);
            } break;

            case EnemyActionKind::DebuffParty:
            {
                for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
                {
                    if (party.Members()[i].hp <= 0) continue;

                    BattleBuff* debuff = PartyDebuffForStat(partyBuffs[i], action.stat);

                    if (debuff == nullptr) continue;

                    ApplyBuff(*debuff);
                    affected++;

                    AddActionText(StatActionText(action.stat, false), i, true,
                        supportColor);
                }

                SetBuffEffect(DebuffFxKind(action.stat), true, -1);
            } break;

            default:
                break;
        }

        switch (action.kind)
        {
            case EnemyActionKind::HealSelf:
                message = enemyName + " uses " + action.name + " and recovers "
                    + std::to_string(healedTotal) + " HP.";
                break;

            case EnemyActionKind::HealAlly:
                message = enemyName + " uses " + action.name + ". An ally recovers "
                    + std::to_string(healedTotal) + " HP.";
                break;

            case EnemyActionKind::HealAllies:
                message = enemyName + " uses " + action.name + ". The enemy party recovers "
                    + std::to_string(healedTotal) + " HP.";
                break;

            case EnemyActionKind::BuffSelf:
                message = enemyName + " uses " + action.name + ". "
                    + std::string(StatActionText(action.stat, true)) + "!";
                break;

            case EnemyActionKind::BuffAllies:
                message = enemyName + " uses " + action.name + ". Enemy "
                    + std::string(StatActionText(action.stat, true)) + "!";
                break;

            case EnemyActionKind::DebuffParty:
                message = enemyName + " uses " + action.name + ". Party "
                    + std::string(StatActionText(action.stat, false)) + "!";
                break;

            default:
                message = enemyName + " uses " + action.name + ".";
                break;
        }

        std::cout << "[ENEMY] " << enemyName << " " << action.name
                  << " (support) affected=" << affected << " | " << message << "\n";

        spendEnemyCost(action.cost, false);
    }
    else
    {
        const int primaryTarget = targets[0];
        const Character& primary = party.Members()[primaryTarget];

        float hitChance = action.accuracy;

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
            SpendEnemyMissPoints();

            message = enemyName + " uses " + action.name + " - MISS!";

            if (Chance(anxietyChance))
            {
                enemyAnxious = true;
                message += " ENEMY ANXIOUS!";
            }

            std::cout << "[ENEMY] " << enemyName << " " << action.name
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
                const bool weak = (action.element == target.weakness);

                int baseDamage = enemyBaseDamage + def.attackBonus;

                if (action.kind == EnemyActionKind::DamageAll)
                    baseDamage = (baseDamage * 2 + 2) / 3;

                int damage = baseDamage + action.power;
                damage -= target.defense / 3;

                if (enemyBuffs[enemyTurnIndex].enemyAttackDown.stacks > 0)
                    damage -= enemyBuffs[enemyTurnIndex].enemyAttackDown.stacks;

                if (enemyBuffs[enemyTurnIndex].attackUp.stacks > 0)
                    damage += enemyBuffs[enemyTurnIndex].attackUp.stacks;

                if (partyBuffs[targetIndex].defenseUp.stacks > 0)
                    damage -= partyBuffs[targetIndex].defenseUp.stacks;

                if (partyBuffs[targetIndex].enemyDefenseDown.stacks > 0)
                    damage += partyBuffs[targetIndex].enemyDefenseDown.stacks;

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

                std::cout << "[ENEMY] " << enemyName << " " << action.name
                          << " -> " << target.name << " "
                          << ElementName(action.element) << " " << damage << " DMG"
                          << (weak ? " WEAK" : "") << (crit ? " CRIT" : "") << "\n";
            }

            std::string prefix;
            if (anyWeak) prefix += "WEAK! ";
            if (anyCrit) prefix += "CRITICAL! ";

            if (action.kind == EnemyActionKind::DamageAll)
            {
                message = prefix + enemyName + " uses " + action.name
                    + " on the party for " + std::to_string(totalDamage)
                    + " total damage.";
            }
            else
            {
                message = prefix + enemyName + " uses " + action.name + " on "
                    + party.Members()[primaryTarget].name + " for "
                    + std::to_string(totalDamage) + " damage.";
            }

            if (action.kind == EnemyActionKind::Drain)
            {
                Enemy& self = enemies[enemyTurnIndex];
                int healed = totalDamage / 2;

                if (healed < 1) healed = 1;

                const int missing = self.GetMaxHp() - self.GetHp();
                if (healed > missing) healed = missing;

                self.Heal(healed);

                AddActionText("+" + std::to_string(healed), enemyTurnIndex, false,
                    { 120, 210, 140, 255 });
                message += " Recovers " + std::to_string(healed) + " HP.";
            }

            for (int targetIndex : fallen)
                message += " " + std::string(party.Members()[targetIndex].name) + " FALLS!";

            if ((anyWeak || anyCrit) && Chance(smirkChance))
            {
                enemySmirk = true;
                message += " ENEMY SMIRK!";
            }

            int cost = action.cost;

            if ((anyWeak || anyCrit) && cost > 1)
                cost--;

            spendEnemyCost(cost, anyWeak || anyCrit);
        }
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

