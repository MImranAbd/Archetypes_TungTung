#include "battle.h"
#include "battle_internal.h"
#include <raylib.h>
#include <iostream>
#include <string>
#include <vector>

using namespace BattleConfig;
using namespace BattleInternal;
void Battle::Defend()
{
    if (phase != Phase::PlayerTurn) return;

    Character& actor = party.Active();
    actor.smirk = false;
    actor.anxious = false;

    defending = true;
    actionBanner = "Defense";
    actionBannerTimer = Battle::actionDuration;
    lastActorParty = party.GetActiveIndex();
    lastActorEnemy = -1;
    lastActionHasTarget = false;
    lastActionTargetOnParty = false;
    lastActionTargetIndex = -1;
    SetBuffEffect(SkillKind::BuffDefense, true, -1);

    message = std::string(party.Active().name) + " braces the party. GUARD!";

    playerPoints -= 2;
    if (playerPoints < 0) playerPoints = 0;

    LogState();
    RotateCharacter();
    ResetMenu();

    if (playerPoints <= 0)
        StartEnemyTurn();
}

void Battle::TryEscape()
{
    escaped = true;
    finished = true;
    won = false;
    message = "ESCAPED!";

    std::cout << "=== ESCAPED ===\n";
}

void Battle::AddDamagePopup(int target, bool onParty, int amount, bool crit, bool weak)
{
    damagePopups.push_back(DamagePopup{
        target, onParty, amount, crit, weak, damagePopupDuration
    });
}

void Battle::SetBuffEffect(SkillKind kind, bool onParty, int target)
{
    buffFxKind = kind;
    buffFxOnParty = onParty;
    buffFxTarget = target;
    buffFxTimer = 0.6f;
}

void Battle::UseSkill(int skillIndex)
{
    if (phase != Phase::PlayerTurn) return;

    Character& actor = party.Active();

    if (actor.hp <= 0)
    {
        SwitchCharacter();
        return;
    }

    if (AllEnemiesDead())
    {
        phase = Phase::Victory;
        return;
    }

    if (skillIndex < 0 || skillIndex >= static_cast<int>(actor.skills.size()))
        return;

    const Skill& skill = *actor.skills[skillIndex];

    if (actor.level < skill.GetLevelRequired())
    {
        message = std::string(skill.GetName()) + " is not learned yet.";
        return;
    }

    if (skill.IsAttack())
    {
        if (skill.IsAttackAll())
            PlayerAttackAll(skill);
        else
            PlayerAttack(skill);

        return;
    }

    actionBanner = skill.GetName();
    actionBannerTimer = Battle::actionDuration;

    actor.smirk = false;
    actor.anxious = false;

    lastActorParty = party.GetActiveIndex();
    lastActorEnemy = -1;
    lastActionHasTarget = false;
    lastActionTargetOnParty = false;
    lastActionTargetIndex = -1;

    int allyTarget = selectedAlly;

    if (allyTarget < 0 || allyTarget >= static_cast<int>(party.Members().size()) ||
        party.Members()[allyTarget].hp <= 0)
    {
        allyTarget = FindLowestHpAlly();
        if (allyTarget < 0) return;
        selectedAlly = allyTarget;
    }

    int enemyTarget = selectedTarget;

    if (enemyTarget < 0 || enemyTarget >= static_cast<int>(enemies.size()) ||
        !enemies[enemyTarget].IsAlive())
    {
        enemyTarget = FindNextAliveEnemy(enemyTarget);

        if (enemyTarget < 0)
        {
            phase = Phase::Victory;
            return;
        }

        selectedTarget = enemyTarget;
    }

    std::string who = actor.name;

    switch (skill.GetKind())
    {
        case SkillKind::Heal:
        {
            Character& target = party.Members()[allyTarget];
            const int missing = target.maxHp - target.hp;

            if (missing > 0)
            {
                const int healed = (skill.GetPower() < missing) ? skill.GetPower() : missing;

                target.hp += healed;
                lastActionHasTarget = true;
                lastActionTargetOnParty = true;
                lastActionTargetIndex = allyTarget;
                message = who + " casts " + skill.GetName() + ". " + target.name
                    + " recovers " + std::to_string(healed) + " HP.";
            }
            else
            {
                message = who + " casts " + skill.GetName() + ". " + target.name
                    + " is already at full health.";
            }
        } break;

        case SkillKind::HealAll:
        {
            int healedTotal = 0;

            for (Character& member : party.Members())
            {
                if (member.hp <= 0 || member.hp >= member.maxHp) continue;

                const int healed = (skill.GetPower() < member.maxHp - member.hp)
                    ? skill.GetPower() : (member.maxHp - member.hp);

                member.hp += healed;
                healedTotal += healed;
            }

            message = who + " casts " + skill.GetName() + ". The party recovers "
                + std::to_string(healedTotal) + " HP.";
        } break;

        case SkillKind::BuffAttack:
        case SkillKind::BuffAttackAll:
        {
            int stacks = 0;

            for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
            {
                if (party.Members()[i].hp <= 0) continue;
                if (!skill.IsPartyBuffAll() && i != allyTarget) continue;

                ApplyBuff(partyBuffs[i].attackUp);
                stacks = partyBuffs[i].attackUp.stacks;
            }

            if (skill.IsPartyBuffAll())
                message = who + " casts " + skill.GetName() + ". The party's ATK rises!";
            else
                message = who + " casts " + skill.GetName() + " on "
                    + party.Members()[allyTarget].name + ". ATK UP x"
                    + std::to_string(stacks) + "!";
        } break;

        case SkillKind::BuffDefense:
        case SkillKind::BuffDefenseAll:
        {
            int stacks = 0;

            for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
            {
                if (party.Members()[i].hp <= 0) continue;
                if (!skill.IsPartyBuffAll() && i != allyTarget) continue;

                ApplyBuff(partyBuffs[i].defenseUp);
                stacks = partyBuffs[i].defenseUp.stacks;
            }

            if (skill.IsPartyBuffAll())
                message = who + " casts " + skill.GetName() + ". The party's DEF rises!";
            else
                message = who + " casts " + skill.GetName() + " on "
                    + party.Members()[allyTarget].name + ". DEF UP x"
                    + std::to_string(stacks) + "!";
        } break;

        case SkillKind::BuffEvasion:
        case SkillKind::BuffEvasionAll:
        {
            int stacks = 0;

            for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
            {
                if (party.Members()[i].hp <= 0) continue;
                if (!skill.IsPartyBuffAll() && i != allyTarget) continue;

                ApplyBuff(partyBuffs[i].evasionUp);
                stacks = partyBuffs[i].evasionUp.stacks;
            }

            if (skill.IsPartyBuffAll())
                message = who + " casts " + skill.GetName() + ". The party's EVA rises!";
            else
                message = who + " casts " + skill.GetName() + " on "
                    + party.Members()[allyTarget].name + ". EVA UP x"
                    + std::to_string(stacks) + "!";
        } break;

        case SkillKind::DebuffAttack:
        case SkillKind::DebuffAttackAll:
        {
            int stacks = 0;

            for (int i = 0; i < static_cast<int>(enemies.size()); i++)
            {
                if (!enemies[i].IsAlive()) continue;
                if (!skill.IsEnemyDebuffAll() && i != enemyTarget) continue;

                ApplyBuff(enemyBuffs[i].enemyAttackDown);
                stacks = enemyBuffs[i].enemyAttackDown.stacks;
            }

            if (skill.IsEnemyDebuffAll())
                message = who + " casts " + skill.GetName() + ". All foes' ATK falls!";
            else
                message = who + " casts " + skill.GetName() + " on E"
                    + std::to_string(enemyTarget + 1) + ". ATK DOWN x"
                    + std::to_string(stacks) + "!";
        } break;

        case SkillKind::DebuffDefense:
        case SkillKind::DebuffDefenseAll:
        {
            int stacks = 0;

            for (int i = 0; i < static_cast<int>(enemies.size()); i++)
            {
                if (!enemies[i].IsAlive()) continue;
                if (!skill.IsEnemyDebuffAll() && i != enemyTarget) continue;

                ApplyBuff(enemyBuffs[i].enemyDefenseDown);
                stacks = enemyBuffs[i].enemyDefenseDown.stacks;
            }

            if (skill.IsEnemyDebuffAll())
                message = who + " casts " + skill.GetName() + ". All foes' DEF falls!";
            else
                message = who + " casts " + skill.GetName() + " on E"
                    + std::to_string(enemyTarget + 1) + ". DEF DOWN x"
                    + std::to_string(stacks) + "!";
        } break;

        case SkillKind::DebuffAccuracy:
        case SkillKind::DebuffAccuracyAll:
        {
            int stacks = 0;

            for (int i = 0; i < static_cast<int>(enemies.size()); i++)
            {
                if (!enemies[i].IsAlive()) continue;
                if (!skill.IsEnemyDebuffAll() && i != enemyTarget) continue;

                ApplyBuff(enemyBuffs[i].enemyAccuracyDown);
                stacks = enemyBuffs[i].enemyAccuracyDown.stacks;
            }

            if (skill.IsEnemyDebuffAll())
                message = who + " casts " + skill.GetName() + ". All foes' ACC falls!";
            else
                message = who + " casts " + skill.GetName() + " on E"
                    + std::to_string(enemyTarget + 1) + ". ACC DOWN x"
                    + std::to_string(stacks) + "!";
        } break;

        case SkillKind::Attack:
        case SkillKind::AttackAll:
            break;
    }

    if (skill.IsPartyBuff())
    {
        if (skill.IsPartyBuffAll())
        {
            SetBuffEffect(skill.GetKind(), true, -1);
        }
        else
        {
            lastActionHasTarget = true;
            lastActionTargetOnParty = true;
            lastActionTargetIndex = allyTarget;
            SetBuffEffect(skill.GetKind(), true, allyTarget);
        }
    }
    else if (skill.IsEnemyDebuff())
    {
        if (skill.IsEnemyDebuffAll())
        {
            SetBuffEffect(skill.GetKind(), false, -1);
        }
        else
        {
            lastActionHasTarget = true;
            lastActionTargetOnParty = false;
            lastActionTargetIndex = enemyTarget;
            SetBuffEffect(skill.GetKind(), false, enemyTarget);
        }
    }

    std::cout << "[PARTY] " << actor.name << " uses " << skill.GetName() << " | "
              << message << "\n";

    playerPoints -= 2;
    if (playerPoints < 0) playerPoints = 0;

    LogState();
    RotateCharacter();
    ResetMenu();

    if (playerPoints <= 0)
        StartEnemyTurn();
}

bool Battle::UseItem(ItemType type, int targetIndex)
{
    if (phase != Phase::PlayerTurn) return false;

    if (inventory.Count(type) <= 0)
    {
        message = std::string("No ") + ItemName(type) + " left.";
        return false;
    }

    if (targetIndex < 0 || targetIndex >= static_cast<int>(party.Members().size()))
        return false;

    Character& target = party.Members()[targetIndex];

    if (target.hp <= 0) return false;

    if (target.hp >= target.maxHp)
    {
        message = std::string(target.name) + " is already at full health.";
        return false;
    }

    int healed = ItemHeal(type);
    const int missing = target.maxHp - target.hp;
    if (healed > missing) healed = missing;

    target.hp += healed;
    inventory.Consume(type);

    Character& actor = party.Active();
    actor.smirk = false;
    actor.anxious = false;

    lastActorParty = party.GetActiveIndex();
    lastActorEnemy = -1;
    lastActionHasTarget = true;
    lastActionTargetOnParty = true;
    lastActionTargetIndex = targetIndex;

    message = std::string(ItemName(type)) + " restores " + std::to_string(healed)
        + " HP to " + target.name + ".";

    std::cout << "[ITEM] " << target.name << " recovers " << healed
              << " HP with " << ItemName(type) << "\n";

    playerPoints -= 2;
    if (playerPoints < 0) playerPoints = 0;

    actionBanner = ItemName(type);
    actionBannerTimer = Battle::actionDuration;

    LogState();
    RotateCharacter();
    ResetMenu();

    if (playerPoints <= 0)
        StartEnemyTurn();

    return true;
}
void Battle::PlayerAttack(const Skill& skill)
{
    Character& actor = party.Active();

    if (selectedTarget < 0 || selectedTarget >= static_cast<int>(enemies.size()) ||
        !enemies[selectedTarget].IsAlive())
    {
        const int next = FindNextAliveEnemy(selectedTarget);

        if (next < 0)
        {
            phase = Phase::Victory;
            return;
        }

        selectedTarget = next;
    }

    Enemy& target = enemies[selectedTarget];
    const int actorIndex = party.GetActiveIndex();

    slashActor = actorIndex;
    lastActorParty = actorIndex;
    lastActorEnemy = -1;
    lastActionHasTarget = true;
    lastActionTargetOnParty = false;
    lastActionTargetIndex = selectedTarget;
    actionBanner = skill.GetName();
    actionBannerTimer = Battle::actionDuration;

    const bool smirk = actor.smirk;
    const bool anxious = actor.anxious;
    actor.smirk = false;
    actor.anxious = false;

    float hitChance = 0.90f;
    if (smirk) hitChance += 0.10f;
    if (anxious) hitChance -= 0.25f;
    if (enemySmirk) hitChance -= 0.30f;
    if (enemyAnxious) hitChance += 0.20f;

    if (hitChance < 0.05f) hitChance = 0.05f;
    if (hitChance > 0.99f) hitChance = 0.99f;

    const std::string targetName = "E" + std::to_string(selectedTarget + 1);

    if (!Chance(hitChance))
    {
        playerPoints -= 4;

        message = actor.name + std::string(" uses ") + skill.GetName() + " on "
            + targetName + " - MISS!";

        if (Chance(anxietyChance))
        {
            actor.anxious = true;
            message += " ANXIETY!";
        }

        std::cout << "[PARTY] " << actor.name << " -> " << targetName
                  << " MISS | " << message << "\n";
    }
    else
    {
        float critChance = 0.10f;
        if (smirk) critChance += 0.40f;
        if (anxious) critChance -= 0.05f;
        if (critChance < 0.0f) critChance = 0.0f;

        const bool crit = Chance(critChance);
        const bool weak = (skill.GetElement() == target.GetWeakness());

        int damage = actor.attack + skill.GetPower();
        if (partyBuffs[actorIndex].attackUp.stacks > 0)
            damage += partyBuffs[actorIndex].attackUp.stacks;
        if (enemyBuffs[selectedTarget].enemyDefenseDown.stacks > 0)
            damage += enemyBuffs[selectedTarget].enemyDefenseDown.stacks;
        if (weak) damage += 1;
        if (crit) damage += 1;

        target.TakeDamage(damage);
        slashTimer = 0.9f;
        hitFlashTimer = hitFlashDuration;
        hitFlashTarget = selectedTarget;
        hitFlashOnParty = false;
        AddDamagePopup(selectedTarget, false, damage, crit, weak);

        if (weak && !weaknessKnown[selectedTarget])
            weaknessKnown[selectedTarget] = true;

        std::string prefix;
        if (weak) prefix += "WEAK! ";
        if (crit) prefix += "CRITICAL! ";

        message = prefix + actor.name + " uses " + skill.GetName() + " on "
            + targetName + ": " + std::to_string(damage) + " damage.";

        if ((weak || crit) && Chance(smirkChance))
        {
            actor.smirk = true;
            message += " SMIRK!";
        }

        playerPoints -= (weak || crit) ? 1 : 2;

        std::cout << "[PARTY] " << actor.name << " -> " << targetName
                  << " " << ElementName(skill.GetElement()) << " " << damage << " DMG"
                  << (weak ? " WEAK" : "") << (crit ? " CRIT" : "")
                  << (actor.smirk ? " SMIRK" : "") << "\n";
    }

    if (playerPoints < 0) playerPoints = 0;

    LogState();

    if (AllEnemiesDead())
    {
        phase = Phase::Victory;
        message = "ALL ENEMIES FALL!";
        std::cout << "=== VICTORY ===\n";
        return;
    }

    if (!target.IsAlive())
    {
        const int next = FindNextAliveEnemy((selectedTarget + 1) % static_cast<int>(enemies.size()));
        if (next >= 0) selectedTarget = next;
    }

    RotateCharacter();
    ResetMenu();

    if (playerPoints <= 0)
        StartEnemyTurn();
}

void Battle::PlayerAttackAll(const Skill& skill)
{
    Character& actor = party.Active();
    const int actorIndex = party.GetActiveIndex();

    slashActor = actorIndex;
    lastActorParty = actorIndex;
    lastActorEnemy = -1;
    lastActionHasTarget = false;
    lastActionTargetOnParty = false;
    lastActionTargetIndex = -1;
    actionBanner = skill.GetName();
    actionBannerTimer = Battle::actionDuration;

    const bool smirk = actor.smirk;
    const bool anxious = actor.anxious;
    actor.smirk = false;
    actor.anxious = false;

    float hitChance = 0.90f;
    if (smirk) hitChance += 0.10f;
    if (anxious) hitChance -= 0.25f;
    if (enemySmirk) hitChance -= 0.30f;
    if (enemyAnxious) hitChance += 0.20f;

    if (hitChance < 0.05f) hitChance = 0.05f;
    if (hitChance > 0.99f) hitChance = 0.99f;

    if (!Chance(hitChance))
    {
        playerPoints -= 4;

        message = actor.name + std::string(" uses ") + skill.GetName()
            + " - MISS!";

        if (Chance(anxietyChance))
        {
            actor.anxious = true;
            message += " ANXIETY!";
        }

        std::cout << "[PARTY] " << actor.name << " -> ALL MISS | "
                  << message << "\n";
    }
    else
    {
        int totalDamage = 0;
        bool anyWeak = false;
        bool anyCrit = false;
        bool flashed = false;

        for (int i = 0; i < static_cast<int>(enemies.size()); i++)
        {
            Enemy& target = enemies[i];

            if (!target.IsAlive()) continue;

            float critChance = 0.10f;
            if (smirk) critChance += 0.40f;
            if (anxious) critChance -= 0.05f;
            if (critChance < 0.0f) critChance = 0.0f;

            const bool crit = Chance(critChance);
            const bool weak = (skill.GetElement() == target.GetWeakness());

            int damage = actor.attack + skill.GetPower();
            if (partyBuffs[actorIndex].attackUp.stacks > 0)
                damage += partyBuffs[actorIndex].attackUp.stacks;
            if (enemyBuffs[i].enemyDefenseDown.stacks > 0)
                damage += enemyBuffs[i].enemyDefenseDown.stacks;
            if (weak) damage += 1;
            if (crit) damage += 1;

            target.TakeDamage(damage);
            AddDamagePopup(i, false, damage, crit, weak);

            if (!flashed)
            {
                hitFlashTimer = hitFlashDuration;
                hitFlashTarget = i;
                hitFlashOnParty = false;
                flashed = true;
            }

            if (weak && !weaknessKnown[i])
                weaknessKnown[i] = true;

            totalDamage += damage;
            anyWeak = anyWeak || weak;
            anyCrit = anyCrit || crit;

            std::cout << "[PARTY] " << actor.name << " -> E" << (i + 1)
                      << " " << ElementName(skill.GetElement()) << " " << damage
                      << " DMG" << (weak ? " WEAK" : "") << (crit ? " CRIT" : "")
                      << "\n";
        }

        std::string prefix;
        if (anyWeak) prefix += "WEAK! ";
        if (anyCrit) prefix += "CRITICAL! ";

        message = prefix + actor.name + " uses " + skill.GetName()
            + " on all foes: " + std::to_string(totalDamage) + " total damage.";

        if ((anyWeak || anyCrit) && Chance(smirkChance))
        {
            actor.smirk = true;
            message += " SMIRK!";
        }

        playerPoints -= (anyWeak || anyCrit) ? 1 : 2;

        slashTimer = 0.9f;
    }

    if (playerPoints < 0) playerPoints = 0;

    LogState();

    if (AllEnemiesDead())
    {
        phase = Phase::Victory;
        message = "ALL ENEMIES FALL!";
        std::cout << "=== VICTORY ===\n";
        return;
    }

    if (selectedTarget < 0 || selectedTarget >= static_cast<int>(enemies.size()) ||
        !enemies[selectedTarget].IsAlive())
    {
        const int next = FindNextAliveEnemy(selectedTarget);
        if (next >= 0) selectedTarget = next;
    }

    RotateCharacter();
    ResetMenu();

    if (playerPoints <= 0)
        StartEnemyTurn();
}

void Battle::TickBuffs()
{
    for (BattleBuffs& buffSet : partyBuffs)
    {
        TickBuff(buffSet.attackUp);
        TickBuff(buffSet.defenseUp);
        TickBuff(buffSet.evasionUp);
        TickBuff(buffSet.enemyAttackDown);
        TickBuff(buffSet.enemyDefenseDown);
        TickBuff(buffSet.enemyAccuracyDown);
    }

    for (BattleBuffs& buffSet : enemyBuffs)
    {
        TickBuff(buffSet.attackUp);
        TickBuff(buffSet.defenseUp);
        TickBuff(buffSet.evasionUp);
        TickBuff(buffSet.enemyAttackDown);
        TickBuff(buffSet.enemyDefenseDown);
        TickBuff(buffSet.enemyAccuracyDown);
    }
}

void Battle::RotateCharacter()
{
    if (party.IsWipedOut()) return;

    const int before = party.GetActiveIndex();
    party.SwitchActive();

    if (party.GetActiveIndex() != before)
        std::cout << "[ROTATE] Next: " << party.Active().name << "\n";
}

void Battle::SwitchCharacter()
{
    if (phase != Phase::PlayerTurn) return;
    if (LivingPartyCount() <= 1) return;
    if (playerPoints <= 0) return;

    playerPoints -= 1;
    party.SwitchActive();

    message = std::string(party.Active().name) + " takes the lead. PASS!";
    std::cout << "[PASS] Next: " << party.Active().name << " | cost 0.5 icon\n";

    ResetMenu();
    LogState();

    if (playerPoints <= 0)
        StartEnemyTurn();
}





