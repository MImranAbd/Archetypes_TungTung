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
    void SpendHalfPoints(int& points, int& halfIcons, bool grantsHalf)
    {
        if (points <= 0)
        {
            halfIcons = 0;
            return;
        }

        const int fullIcons = (points - halfIcons) / 2;

        if (grantsHalf)
        {
            if (fullIcons > 0) halfIcons++;
            else if (halfIcons > 0) halfIcons--;
        }
        else
        {
            if (halfIcons > 0) halfIcons--;
            else if (fullIcons > 0) halfIcons++;
        }

        points -= 1;

        if (points < 0) points = 0;
        if (points == 0) halfIcons = 0;
    }

    void SpendFullPoints(int& points, int& halfIcons)
    {
        const int fullIcons = (points - halfIcons) / 2;

        if (fullIcons > 0)
        {
        }
        else if (halfIcons >= 2)
        {
            halfIcons -= 2;
        }
        else
        {
            halfIcons = 0;
        }

        points -= 2;

        if (points < 0) points = 0;
        if (points == 0) halfIcons = 0;
    }

    void SpendMissPoints(int& points, int& halfIcons)
    {
        const int fullIcons = (points - halfIcons) / 2;

        if (halfIcons >= 2)
        {
            halfIcons -= 2;
        }
        else if (halfIcons == 1)
        {
            halfIcons = (fullIcons > 0) ? 1 : 0;
        }
        else
        {
            halfIcons = 0;
        }

        points -= 2;

        if (points < 0) points = 0;
        if (points == 0) halfIcons = 0;
    }

    Color SupportEffectColor(SkillKind kind)
    {
        switch (kind)
        {
            case SkillKind::Heal:
            case SkillKind::HealAll:
                return { 120, 210, 140, 255 };
            case SkillKind::Revive:
                return { 235, 210, 120, 255 };
            case SkillKind::BuffAttack:
            case SkillKind::BuffAttackAll:
                return { 225, 130, 95, 255 };
            case SkillKind::BuffDefense:
            case SkillKind::BuffDefenseAll:
                return { 120, 185, 135, 255 };
            case SkillKind::BuffEvasion:
            case SkillKind::BuffEvasionAll:
                return { 165, 135, 225, 255 };
            case SkillKind::DebuffAttack:
            case SkillKind::DebuffAttackAll:
                return { 215, 95, 85, 255 };
            case SkillKind::DebuffDefense:
            case SkillKind::DebuffDefenseAll:
                return { 215, 165, 85, 255 };
            case SkillKind::DebuffAccuracy:
            case SkillKind::DebuffAccuracyAll:
                return { 170, 120, 210, 255 };
            default:
                return { 220, 205, 175, 255 };
        }
    }
}

void Battle::Defend()
{
    if (phase != Phase::PlayerTurn) return;

    Character& actor = party.Active();
    actor.smirk = false;
    actor.anxious = false;

    defending = true;
    actionBanner = "Defense";
    actionBannerTimer = Battle::actionDuration;
    actionCinematic = true;
    actionWideParty = false;
    actionAttackPose = false;
    supportFlashTimer = Battle::supportFlashDuration;
    supportFlashColor = SupportEffectColor(SkillKind::BuffDefense);
    lastActorParty = party.GetActiveIndex();
    lastActorEnemy = -1;
    lastActionHasTarget = false;
    lastActionTargetOnParty = false;
    lastActionTargetIndex = -1;
    SetBuffEffect(SkillKind::BuffDefense, true, -1);
    AddActionText("GUARD", party.GetActiveIndex(), true, supportFlashColor);

    message = std::string(party.Active().name) + " braces the party. GUARD!";

    SpendPlayerFullPoints();

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

void Battle::AddActionText(const std::string& text, int target, bool onParty, Color color)
{
    actionTexts.push_back(ActionText{
        text, target, onParty, color, supportTextDuration
    });
}

void Battle::SpendPlayerHalfPoint(bool grantsHalf)
{
    SpendHalfPoints(playerPoints, playerHalfIcons, grantsHalf);
}

void Battle::SpendPlayerFullPoints()
{
    SpendFullPoints(playerPoints, playerHalfIcons);
}

void Battle::SpendPlayerMissPoints()
{
    SpendMissPoints(playerPoints, playerHalfIcons);
}

void Battle::SpendEnemyHalfPoint(bool grantsHalf)
{
    SpendHalfPoints(enemyPoints, enemyHalfIcons, grantsHalf);
}

void Battle::SpendEnemyFullPoints()
{
    SpendFullPoints(enemyPoints, enemyHalfIcons);
}

void Battle::SpendEnemyMissPoints()
{
    SpendMissPoints(enemyPoints, enemyHalfIcons);
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

    const Color supportColor = SupportEffectColor(skill.GetKind());

    actionBanner = skill.GetName();
    actionBannerTimer = Battle::actionDuration;
    actionCinematic = true;
    actionWideParty = skill.IsHealAll() || skill.IsPartyBuffAll();
    actionAttackPose = skill.IsEnemyDebuff();
    supportFlashTimer = Battle::supportFlashDuration;
    supportFlashColor = supportColor;

    actor.smirk = false;
    actor.anxious = false;

    lastActorParty = party.GetActiveIndex();
    lastActorEnemy = -1;
    lastActionHasTarget = false;
    lastActionTargetOnParty = false;
    lastActionTargetIndex = -1;

    const bool reviving = skill.IsRevive();
    int allyTarget = selectedAlly;

    if (allyTarget < 0 || allyTarget >= static_cast<int>(party.Members().size()) ||
        (reviving ? party.Members()[allyTarget].hp > 0
            : party.Members()[allyTarget].hp <= 0))
    {
        allyTarget = reviving ? FindFallenAlly() : FindLowestHpAlly();

        if (allyTarget < 0)
        {
            message = reviving ? "No fallen ally to revive." : "No ally to target.";
            return;
        }

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
                int healPower = skill.GetPower();

                if (actor.role == Role::Priest)
                    healPower += 2;

                const int healed = (healPower < missing) ? healPower : missing;

                target.hp += healed;
                lastActionHasTarget = true;
                lastActionTargetOnParty = true;
                lastActionTargetIndex = allyTarget;
                AddActionText("+" + std::to_string(healed), allyTarget, true,
                    supportColor);
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

            for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
            {
                Character& member = party.Members()[i];

                if (member.hp <= 0 || member.hp >= member.maxHp) continue;

                int healPower = skill.GetPower();

                if (actor.role == Role::Priest)
                    healPower += 2;

                const int healed = (healPower < member.maxHp - member.hp)
                    ? healPower : (member.maxHp - member.hp);

                member.hp += healed;
                healedTotal += healed;
                AddActionText("+" + std::to_string(healed), i, true, supportColor);
            }

            message = who + " casts " + skill.GetName() + ". The party recovers "
                + std::to_string(healedTotal) + " HP.";
        } break;

        case SkillKind::Revive:
        {
            Character& target = party.Members()[allyTarget];

            if (target.hp > 0)
            {
                message = who + " casts " + skill.GetName() + ". " + target.name
                    + " is not fallen.";
            }
            else
            {
                int reviveHp = skill.GetPower();

                if (actor.role == Role::Priest)
                    reviveHp += 2;

                if (reviveHp > target.maxHp)
                    reviveHp = target.maxHp;

                target.hp = reviveHp;
                target.smirk = false;
                target.anxious = false;

                lastActionHasTarget = true;
                lastActionTargetOnParty = true;
                lastActionTargetIndex = allyTarget;

                AddActionText("REVIVE", allyTarget, true, supportColor);
                message = who + " casts " + skill.GetName() + ". " + target.name
                    + " returns with " + std::to_string(reviveHp) + " HP!";
            }
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
                AddActionText("ATK UP", i, true, supportColor);
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
                AddActionText("DEF UP", i, true, supportColor);
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
                AddActionText("EVA UP", i, true, supportColor);
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
                AddActionText("ATK DOWN", i, false, supportColor);
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
                AddActionText("DEF DOWN", i, false, supportColor);
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
                AddActionText("ACC DOWN", i, false, supportColor);
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
    else if (skill.GetKind() == SkillKind::Heal)
    {
        SetBuffEffect(SkillKind::Heal, true, allyTarget);
    }
    else if (skill.GetKind() == SkillKind::HealAll)
    {
        SetBuffEffect(SkillKind::HealAll, true, -1);
    }
    else if (skill.GetKind() == SkillKind::Revive)
    {
        SetBuffEffect(SkillKind::Revive, true, allyTarget);
    }

    std::cout << "[PARTY] " << actor.name << " uses " << skill.GetName() << " | "
              << message << "\n";

    if (playerHalfIcons > 0)
        SpendPlayerHalfPoint(false);
    else
        SpendPlayerFullPoints();

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
    const bool revives = ItemRevives(type);

    if (revives)
    {
        if (target.hp > 0)
        {
            message = std::string(target.name) + " is not fallen.";
            return false;
        }
    }
    else
    {
        if (target.hp <= 0)
        {
            message = std::string("Cannot heal a fallen ally with ")
                + ItemName(type) + ".";
            return false;
        }

        if (target.hp >= target.maxHp)
        {
            message = std::string(target.name) + " is already at full health.";
            return false;
        }
    }

    int restored = 0;

    if (revives)
    {
        restored = ItemReviveHp(type);
        if (restored > target.maxHp) restored = target.maxHp;

        target.hp = restored;
        target.smirk = false;
        target.anxious = false;
    }
    else
    {
        restored = ItemHeal(type);

        const int missing = target.maxHp - target.hp;
        if (restored > missing) restored = missing;

        target.hp += restored;
    }

    inventory.Consume(type);

    Character& actor = party.Active();
    actor.smirk = false;
    actor.anxious = false;

    lastActorParty = party.GetActiveIndex();
    lastActorEnemy = -1;
    lastActionHasTarget = true;
    lastActionTargetOnParty = true;
    lastActionTargetIndex = targetIndex;

    if (revives)
    {
        message = std::string(ItemName(type)) + " revives " + target.name
            + " with " + std::to_string(restored) + " HP!";
    }
    else
    {
        message = std::string(ItemName(type)) + " restores "
            + std::to_string(restored) + " HP to " + target.name + ".";
    }

    std::cout << "[ITEM] " << target.name
              << (revives ? " revived with " : " recovers ")
              << restored << " HP with " << ItemName(type) << "\n";

    SpendPlayerFullPoints();

    const SkillKind fxKind = revives ? SkillKind::Revive : SkillKind::Heal;
    const Color supportColor = SupportEffectColor(fxKind);

    actionBanner = ItemName(type);
    actionBannerTimer = Battle::actionDuration;
    actionCinematic = true;
    actionWideParty = false;
    actionAttackPose = false;
    supportFlashTimer = Battle::supportFlashDuration;
    supportFlashColor = supportColor;
    AddActionText(revives ? "REVIVE" : ("+" + std::to_string(restored)),
        targetIndex, true, supportColor);
    SetBuffEffect(fxKind, true, targetIndex);

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

    actionCinematic = true;
    actionWideParty = false;
    actionAttackPose = true;
    supportFlashTimer = 0.0f;
    actionTexts.clear();
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
    if (partyBuffs[actorIndex].enemyAccuracyDown.stacks > 0)
        hitChance -= 0.15f * partyBuffs[actorIndex].enemyAccuracyDown.stacks;
    if (enemyBuffs[selectedTarget].evasionUp.stacks > 0)
        hitChance -= 0.15f * enemyBuffs[selectedTarget].evasionUp.stacks;
    if (actor.role == Role::Mage)
        hitChance += 0.05f;

    if (hitChance < 0.05f) hitChance = 0.05f;
    if (hitChance > 0.99f) hitChance = 0.99f;

    const std::string targetName = "E" + std::to_string(selectedTarget + 1);

    if (!Chance(hitChance))
    {
        SpendPlayerMissPoints();

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
        if (actor.role == Role::Thief) critChance += 0.10f;
        if (critChance < 0.0f) critChance = 0.0f;

        const bool crit = Chance(critChance);
        const bool weak = (skill.GetElement() == target.GetWeakness());

        int damage = actor.attack + skill.GetPower();
        if (partyBuffs[actorIndex].attackUp.stacks > 0)
            damage += partyBuffs[actorIndex].attackUp.stacks;
        if (partyBuffs[actorIndex].enemyAttackDown.stacks > 0)
            damage -= partyBuffs[actorIndex].enemyAttackDown.stacks;
        if (enemyBuffs[selectedTarget].enemyDefenseDown.stacks > 0)
            damage += enemyBuffs[selectedTarget].enemyDefenseDown.stacks;
        if (enemyBuffs[selectedTarget].defenseUp.stacks > 0)
            damage -= enemyBuffs[selectedTarget].defenseUp.stacks;
        if (actor.role == Role::Warrior && skill.GetElement() == Element::Physical)
            damage += 1;
        if (actor.role == Role::Mage && skill.GetElement() != Element::Physical)
            damage += 1;
        if (actor.role == Role::Priest && skill.GetElement() == Element::Light)
            damage += 1;
        if (weak) damage += 1;
        if (crit) damage += 1;
        if (damage < 1) damage = 1;

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

        if (weak || crit)
            SpendPlayerHalfPoint(true);
        else
            SpendPlayerFullPoints();

        std::cout << "[PARTY] " << actor.name << " -> " << targetName
                  << " " << ElementName(skill.GetElement()) << " " << damage << " DMG"
                  << (weak ? " WEAK" : "") << (crit ? " CRIT" : "")
                  << (actor.smirk ? " SMIRK" : "") << "\n";
    }

    if (playerPoints < 0) playerPoints = 0;

    LogState();

    if (AllEnemiesDead())
    {
        pendingVictory = true;
        message = "ALL ENEMIES FALL!";
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

    actionCinematic = true;
    actionWideParty = false;
    actionAttackPose = true;
    supportFlashTimer = 0.0f;
    actionTexts.clear();
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
    if (partyBuffs[actorIndex].enemyAccuracyDown.stacks > 0)
        hitChance -= 0.15f * partyBuffs[actorIndex].enemyAccuracyDown.stacks;
    if (actor.role == Role::Mage)
        hitChance += 0.05f;

    int maxEvasion = 0;

    for (int i = 0; i < static_cast<int>(enemies.size()); i++)
    {
        if (!enemies[i].IsAlive()) continue;

        if (enemyBuffs[i].evasionUp.stacks > maxEvasion)
            maxEvasion = enemyBuffs[i].evasionUp.stacks;
    }

    hitChance -= 0.15f * maxEvasion;

    if (hitChance < 0.05f) hitChance = 0.05f;
    if (hitChance > 0.99f) hitChance = 0.99f;

    if (!Chance(hitChance))
    {
        SpendPlayerMissPoints();

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
            if (actor.role == Role::Thief) critChance += 0.10f;
            if (critChance < 0.0f) critChance = 0.0f;

            const bool crit = Chance(critChance);
            const bool weak = (skill.GetElement() == target.GetWeakness());

            int damage = actor.attack + skill.GetPower();
            if (partyBuffs[actorIndex].attackUp.stacks > 0)
                damage += partyBuffs[actorIndex].attackUp.stacks;
            if (partyBuffs[actorIndex].enemyAttackDown.stacks > 0)
                damage -= partyBuffs[actorIndex].enemyAttackDown.stacks;
            if (enemyBuffs[i].enemyDefenseDown.stacks > 0)
                damage += enemyBuffs[i].enemyDefenseDown.stacks;
            if (enemyBuffs[i].defenseUp.stacks > 0)
                damage -= enemyBuffs[i].defenseUp.stacks;
            if (actor.role == Role::Warrior && skill.GetElement() == Element::Physical)
                damage += 1;
            if (actor.role == Role::Mage && skill.GetElement() != Element::Physical)
                damage += 1;
            if (actor.role == Role::Priest && skill.GetElement() == Element::Light)
                damage += 1;
            if (weak) damage += 1;
            if (crit) damage += 1;
            if (damage < 1) damage = 1;

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

        if (anyWeak || anyCrit)
            SpendPlayerHalfPoint(true);
        else
            SpendPlayerFullPoints();

        slashTimer = 0.9f;
    }

    if (playerPoints < 0) playerPoints = 0;

    LogState();

    if (AllEnemiesDead())
    {
        pendingVictory = true;
        message = "ALL ENEMIES FALL!";
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

    SpendPlayerHalfPoint(false);
    party.SwitchActive();

    message = std::string(party.Active().name) + " takes the lead. PASS!";
    std::cout << "[PASS] Next: " << party.Active().name << " | cost 0.5 icon\n";

    ResetMenu();
    LogState();

    if (playerPoints <= 0)
        StartEnemyTurn();
}





