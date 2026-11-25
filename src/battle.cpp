#include "battle.h"
#include "battle_internal.h"
#include <raylib.h>
#include <iostream>
#include <string>
#include <vector>

using namespace BattleConfig;
using namespace BattleInternal;
Battle::Battle(Party& party, Inventory& inventory, int floorNumber,
    bool enemyAmbush)
    : party(party)
    , inventory(inventory)
    , floorNumber(floorNumber)
    , enemyBaseDamage((floorNumber % 10 == 0)
        ? eliteEnemyDamage + (floorNumber / 10 - 1) * eliteEnemyDamageStep
        : enemyDamageBase + (floorNumber - 1) / 2)
    , isElite(floorNumber % 10 == 0)
    , phase(Phase::PlayerTurn)
    , turnTimer(0.0f)
    , slashTimer(0.0f)
    , hitFlashTimer(0.0f)
    , hitFlashTarget(0)
    , hitFlashOnParty(false)
    , finished(false)
    , won(false)
    , escaped(false)
    , playerPoints(0)
    , enemyPoints(0)
    , playerHalfIcons(0)
    , enemyHalfIcons(0)
    , selectedTarget(0)
    , selectedAlly(0)
    , slashActor(0)
    , enemyTurnIndex(0)
    , turnNumber(1)
    , defending(false)
    , enemySmirk(false)
    , enemyAnxious(false)
    , partyBuffs{}
    , enemyBuffs{}
    , message(isElite ? "A TREMENDOUS FOE APPEARS!" : "A RANDOM BATTLE BEGINS!")
    , menuState(MenuState::Command)
    , commandIndex(0)
    , pendingAction(PendingAction::None)
    , targetReturnState(MenuState::Command)
    , skillMenuIndex(0)
    , itemMenuIndex(0)
    , buffFxTimer(0.0f)
    , buffFxOnParty(false)
    , buffFxTarget(-1)
    , buffFxKind(SkillKind::Attack)
    , actionBannerTimer(0.0f)
    , actionCinematic(true)
    , actionWideParty(false)
    , actionAttackPose(false)
    , pendingVictory(false)
    , bossSummonPending(false)
    , bossMatarukajaPending(false)
    , bossChargePending(false)
    , bossCharged(false)
    , supportFlashTimer(0.0f)
    , supportFlashColor(WHITE)
    , actionTexts()
    , lastActorParty(-1)
    , lastActorEnemy(-1)
    , lastActionHasTarget(false)
    , lastActionTargetOnParty(false)
    , lastActionTargetIndex(-1)
    , xpReward((50 + floorNumber * 6) * (floorNumber % 10 == 0 ? 2 : 1))
    , moneyReward(40 + floorNumber * 4)
    , itemReward(1)
    , equipmentReward(false)
    , equipmentRewardId(EquipmentId::None)
{
    equipmentReward = (GetRandomValue(0, 99) < 50);
    if (equipmentReward)
    {
        equipmentRewardId = AllEquipmentIds()[GetRandomValue(0, AllEquipmentCount() - 1)];
    }
    const int enemyCount = isElite ? 1 : battleEnemyCount;
    int enemyHp = isElite
        ? eliteEnemyHp * (floorNumber / 10)
        : battleEnemyHp + (floorNumber - 1) / 2;

    if (!isElite && enemyHp > battleEnemyHpCap)
        enemyHp = battleEnemyHpCap;

    if (!isElite && enemyBaseDamage > enemyDamageCap)
        enemyBaseDamage = enemyDamageCap;

    for (int i = 0; i < enemyCount; i++)
    {
        const EnemyKind kind = RandomEnemyKindForFloor(floorNumber, isElite);
        const EnemyDef& def = GetEnemyDef(kind);

        int hp = enemyHp + def.hpBonus;
        if (hp < 1) hp = 1;

        enemies.emplace_back(0, 0, hp, kind);
    }

    weaknessKnown.assign(enemyCount, false);

    for (Character& member : party.Members())
    {
        member.smirk = false;
        member.anxious = false;
    }

    playerPoints = LivingPartyCount() * pointsPerMember;
    enemyPoints = LivingEnemyCount() * pointsPerMember;
    playerHalfIcons = 0;
    enemyHalfIcons = 0;

    std::cout.setf(std::ios::unitbuf);

    std::cout << "\n========== BATTLE START ==========\n";
    std::cout << "Dungeon Floor: " << floorNumber << "\n";
    std::cout << "PARTY:";

    for (const Character& member : party.Members())
        std::cout << " " << member.name << " " << member.hp << "/" << member.maxHp;

    std::cout << "\nENEMIES:\n";

    for (int i = 0; i < static_cast<int>(enemies.size()); i++)
    {
        std::cout << " E" << (i + 1) << " " << EnemyKindName(enemies[i].GetKind())
                  << " HP " << enemies[i].GetHp() << "/" << enemies[i].GetMaxHp()
                  << " WEAKNESS " << ElementName(enemies[i].GetWeakness()) << "\n";
    }

    std::cout << "Press Turn: Party " << LivingPartyCount()
              << " icons | Enemies " << LivingEnemyCount() << " icons\n";
    std::cout << "----------------------------------\n";

    if (enemyAmbush)
    {
        message = "AMBUSHED! The enemy strikes first!";
        std::cout << "=== AMBUSH ===\n";
        StartEnemyTurn();
        turnTimer = 1.0f;
    }
}

void Battle::Update()
{
    if (finished) return;

    const float delta = GetFrameTime();

    if (slashTimer > 0.0f)
        slashTimer -= delta;

    if (hitFlashTimer > 0.0f)
        hitFlashTimer -= delta;

    if (buffFxTimer > 0.0f)
        buffFxTimer -= delta;

    if (actionBannerTimer > 0.0f)
        actionBannerTimer -= delta;

    if (supportFlashTimer > 0.0f)
        supportFlashTimer -= delta;

    for (int i = static_cast<int>(actionTexts.size()) - 1; i >= 0; i--)
    {
        actionTexts[i].timer -= delta;

        if (actionTexts[i].timer <= 0.0f)
            actionTexts.erase(actionTexts.begin() + i);
    }

    for (int i = static_cast<int>(damagePopups.size()) - 1; i >= 0; i--)
    {
        damagePopups[i].timer -= delta;

        if (damagePopups[i].timer <= 0.0f)
            damagePopups.erase(damagePopups.begin() + i);
    }

    if (pendingVictory && phase == Phase::PlayerTurn && actionBannerTimer <= 0.0f)
    {
        pendingVictory = false;
        phase = Phase::Victory;
        message = "ALL ENEMIES FALL!";
        std::cout << "=== VICTORY ===\n";
        return;
    }

    switch (phase)
    {
        case Phase::PlayerTurn:
        {
            if (actionBannerTimer > 0.0f) break;

            Character& active = party.Active();

            if (menuState == MenuState::Command)
            {
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                    commandIndex = (commandIndex + commandCount - 1) % commandCount;

                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                    commandIndex = (commandIndex + 1) % commandCount;

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    IsKeyPressed(KEY_J))
                    ExecuteCommand(commandIndex);
            }
            else if (menuState == MenuState::Skill)
            {
                const int skillCount = static_cast<int>(active.skills.size());

                if (skillCount > 0)
                {
                    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                    {
                        for (int step = 1; step <= skillCount; step++)
                        {
                            const int index = (skillMenuIndex + skillCount - step) % skillCount;

                            if (active.level >= active.skills[index]->GetLevelRequired())
                            {
                                skillMenuIndex = index;
                                break;
                            }
                        }
                    }

                    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                    {
                        for (int step = 1; step <= skillCount; step++)
                        {
                            const int index = (skillMenuIndex + step) % skillCount;

                            if (active.level >= active.skills[index]->GetLevelRequired())
                            {
                                skillMenuIndex = index;
                                break;
                            }
                        }
                    }

                    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                        IsKeyPressed(KEY_J))
                    {
                        const Skill& skill = *active.skills[skillMenuIndex];

                        if (skill.IsRevive() && FindFallenAlly() < 0)
                            message = "No fallen ally to revive.";
                        else if (skill.IsAllTarget() || skill.IsSelfCast())
                            UseSkill(skillMenuIndex);
                        else
                            BeginTargetSelection(PendingAction::Skill);
                    }
                }

                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_K))
                {
                    menuState = MenuState::Command;
                }
            }
            else if (menuState == MenuState::Item)
            {
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                    itemMenuIndex = (itemMenuIndex + itemMenuCount - 1) % itemMenuCount;

                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                    itemMenuIndex = (itemMenuIndex + 1) % itemMenuCount;

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    IsKeyPressed(KEY_J))
                {
                    const ItemType type = ItemAtMenuIndex(itemMenuIndex);

                    if (ItemRevives(type) && FindFallenAlly() < 0)
                        message = "No fallen ally to revive.";
                    else
                        BeginTargetSelection(PendingAction::Item);
                }

                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_K))
                {
                    menuState = MenuState::Command;
                }
            }
            else if (menuState == MenuState::Target)
            {
                bool allyMode = false;

                if (pendingAction == PendingAction::Attack)
                {
                    allyMode = false;
                }
                else if (pendingAction == PendingAction::Item)
                {
                    allyMode = true;
                }
                else if (pendingAction == PendingAction::Skill &&
                    skillMenuIndex >= 0 &&
                    skillMenuIndex < static_cast<int>(active.skills.size()))
                {
                    allyMode = active.skills[skillMenuIndex]->TargetsAlly();
                }

                if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
                {
                    if (allyMode) SelectAlly(-1);
                    else SelectTarget(-1);
                }

                if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
                {
                    if (allyMode) SelectAlly(1);
                    else SelectTarget(1);
                }

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    IsKeyPressed(KEY_J))
                    ExecutePendingAction();

                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_K))
                {
                    menuState = targetReturnState;
                    pendingAction = PendingAction::None;

                    if (targetReturnState == MenuState::Skill)
                        message = "Choose a skill.";
                    else if (targetReturnState == MenuState::Item)
                        message = "Choose an item.";
                    else
                        message = "Choose an action.";
                }
            }
        } break;
        case Phase::EnemyTurn:
        {
            turnTimer -= delta;

            if (turnTimer <= 0.0f)
            {
                ResolveEnemyAttack();

                if (phase == Phase::EnemyTurn)
                    turnTimer = 1.4f;
            }
        } break;

        case Phase::Victory:
        {
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_J) || IsKeyPressed(KEY_ENTER))
            {
                finished = true;
                won = true;
                std::cout << "=== BATTLE END (VICTORY) ===\n";
            }
        } break;

        case Phase::Defeat:
        {
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_J) || IsKeyPressed(KEY_ENTER))
            {
                finished = true;
                won = false;
                std::cout << "=== BATTLE END (RETREAT) ===\n";
            }
        } break;
    }
}

void Battle::StartPlayerTurn()
{
    turnNumber++;
    defending = false;
    TickBuffs();

    if (party.IsWipedOut())
    {
        phase = Phase::Defeat;
        return;
    }

    if (party.Active().hp <= 0)
        party.SwitchActive();

    if (AllEnemiesDead())
    {
        phase = Phase::Victory;
        return;
    }

    playerPoints = LivingPartyCount() * pointsPerMember;
    playerHalfIcons = 0;
    phase = Phase::PlayerTurn;
    ResetMenu();

    std::cout << "--- PLAYER TURN | Press Turn: " << (playerPoints / 2.0f) << " icons ---\n";
}

void Battle::StartEnemyTurn()
{
    enemyPoints = LivingEnemyCount() * pointsPerMember * (isElite ? 2 : 1);
    enemyHalfIcons = 0;
    phase = Phase::EnemyTurn;
    enemyTurnIndex = FindNextAliveEnemy(0);

    if (enemyTurnIndex < 0)
    {
        phase = Phase::Victory;
        return;
    }

    const bool necromancerTurn = (floorNumber >= 50 &&
        enemies[enemyTurnIndex].GetKind() == EnemyKind::Necromancer);

    bossSummonPending = necromancerTurn && turnNumber % 3 == 0;
    bossMatarukajaPending = necromancerTurn && !bossSummonPending &&
        turnNumber % 2 == 0;
    bossChargePending = false;

    turnTimer = 1.4f;

    std::cout << "--- ENEMY TURN | Press Turn: " << (enemyPoints / 2.0f) << " icons ---\n";
}

void Battle::ResetMenu()
{
    menuState = MenuState::Command;
    commandIndex = 0;
    pendingAction = PendingAction::None;
    targetReturnState = MenuState::Command;
    skillMenuIndex = 0;
    itemMenuIndex = 0;
}

void Battle::ExecuteCommand(int command)
{
    if (phase != Phase::PlayerTurn) return;
    if (command < 0 || command >= commandCount) return;

    switch (static_cast<Command>(command))
    {
        case Command::Attack:
        {
            BeginTargetSelection(PendingAction::Attack);
        } break;

        case Command::Skill:
        {
            menuState = MenuState::Skill;
            message = "Choose a skill.";
            skillMenuIndex = 0;

            const Character& active = party.Active();

            for (int i = 0; i < static_cast<int>(active.skills.size()); i++)
            {
                if (active.level >= active.skills[i]->GetLevelRequired())
                {
                    skillMenuIndex = i;
                    break;
                }
            }

            selectedAlly = party.GetActiveIndex();

            if (party.Members()[selectedAlly].hp <= 0)
            {
                for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
                {
                    if (party.Members()[i].hp > 0)
                    {
                        selectedAlly = i;
                        break;
                    }
                }
            }
        } break;

        case Command::Defense:
            Defend();
            break;

        case Command::Item:
            menuState = MenuState::Item;
            itemMenuIndex = 0;
            message = "Choose an item.";
            break;

        case Command::Pass:
            SwitchCharacter();
            break;

        case Command::Escape:
            TryEscape();
            break;
    }
}

void Battle::BeginTargetSelection(PendingAction action)
{
    if (phase != Phase::PlayerTurn) return;

    pendingAction = action;
    targetReturnState = menuState;
    menuState = MenuState::Target;

    Character& active = party.Active();
    const bool reviveMode = IsReviveTargeting();

    if (action == PendingAction::Item)
    {
        const int best = reviveMode ? FindFallenAlly() : FindLowestHpAlly();
        if (best >= 0) selectedAlly = best;
    }
    else if (action == PendingAction::Skill &&
        skillMenuIndex >= 0 &&
        skillMenuIndex < static_cast<int>(active.skills.size()))
    {
        const Skill& skill = *active.skills[skillMenuIndex];

        if (skill.IsRevive())
        {
            const int best = FindFallenAlly();
            if (best >= 0) selectedAlly = best;
        }
        else if (skill.TargetsAlly())
        {
            if (skill.GetKind() == SkillKind::Heal)
            {
                const int best = FindLowestHpAlly();
                if (best >= 0) selectedAlly = best;
            }
            else
            {
                selectedAlly = party.GetActiveIndex();
            }
        }
    }

    if (selectedTarget < 0 || selectedTarget >= static_cast<int>(enemies.size()) ||
        !enemies[selectedTarget].IsAlive())
    {
        const int next = FindNextAliveEnemy(selectedTarget);
        if (next >= 0) selectedTarget = next;
    }

    if (selectedAlly < 0 || selectedAlly >= static_cast<int>(party.Members().size()) ||
        (reviveMode ? party.Members()[selectedAlly].hp > 0
            : party.Members()[selectedAlly].hp <= 0))
    {
        const int best = reviveMode ? FindFallenAlly() : FindLowestHpAlly();
        if (best >= 0) selectedAlly = best;
    }

    message = reviveMode ? "Choose a fallen ally." : "Choose a target.";
}

void Battle::ExecutePendingAction()
{
    if (phase != Phase::PlayerTurn || menuState != MenuState::Target) return;

    if (pendingAction == PendingAction::Attack)
    {
        const AttackSkill basicAttack;

        pendingAction = PendingAction::None;
        PlayerAttack(basicAttack);
    }
    else if (pendingAction == PendingAction::Skill)
    {
        const int index = skillMenuIndex;
        pendingAction = PendingAction::None;
        UseSkill(index);
    }
    else if (pendingAction == PendingAction::Item)
    {
        const ItemType type = ItemAtMenuIndex(itemMenuIndex);

        if (UseItem(type, selectedAlly))
            pendingAction = PendingAction::None;
    }
}

void Battle::SelectTarget(int direction)
{
    const int count = static_cast<int>(enemies.size());

    for (int i = 1; i <= count; i++)
    {
        const int index = ((selectedTarget + direction * i) % count + count) % count;

        if (enemies[index].IsAlive())
        {
            selectedTarget = index;
            return;
        }
    }
}

void Battle::SelectAlly(int direction)
{
    const int count = static_cast<int>(party.Members().size());
    const bool reviveMode = IsReviveTargeting();

    for (int i = 1; i <= count; i++)
    {
        const int index = ((selectedAlly + direction * i) % count + count) % count;

        if (reviveMode ? party.Members()[index].hp <= 0
            : party.Members()[index].hp > 0)
        {
            selectedAlly = index;
            return;
        }
    }
}


int Battle::FindNextAliveEnemy(int start) const
{
    const int count = static_cast<int>(enemies.size());

    for (int i = 0; i < count; i++)
    {
        const int index = (start + i) % count;

        if (enemies[index].IsAlive())
            return index;
    }

    return -1;
}

int Battle::FindLowestHpAlly() const
{
    int best = -1;

    for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
    {
        if (party.Members()[i].hp <= 0) continue;

        if (best < 0 || party.Members()[i].hp < party.Members()[best].hp)
            best = i;
    }

    return best;
}

int Battle::FindFallenAlly() const
{
    for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
    {
        if (party.Members()[i].hp <= 0)
            return i;
    }

    return -1;
}

ItemType Battle::ItemAtMenuIndex(int index)
{
    switch (index)
    {
        case 0: return ItemType::Herb;
        case 1: return ItemType::Potion;
        case 2: return ItemType::Revive;
    }

    return ItemType::Herb;
}

bool Battle::IsReviveTargeting() const
{
    if (pendingAction == PendingAction::Skill)
    {
        const Character& active = party.Active();

        if (skillMenuIndex >= 0 &&
            skillMenuIndex < static_cast<int>(active.skills.size()))
            return active.skills[skillMenuIndex]->IsRevive();
    }
    else if (pendingAction == PendingAction::Item)
    {
        return ItemRevives(ItemAtMenuIndex(itemMenuIndex));
    }

    return false;
}

int Battle::LivingPartyCount() const
{
    int count = 0;

    for (const Character& member : party.Members())
    {
        if (member.hp > 0) count++;
    }

    return count;
}

int Battle::LivingEnemyCount() const
{
    int count = 0;

    for (const Enemy& enemy : enemies)
    {
        if (enemy.IsAlive()) count++;
    }

    return count;
}

bool Battle::AllEnemiesDead() const
{
    return LivingEnemyCount() == 0;
}

bool Battle::IsFinished() const
{
    return finished;
}

bool Battle::PlayerWon() const
{
    return won;
}

bool Battle::WasEscaped() const
{
    return escaped;
}

bool Battle::IsElite() const
{
    return isElite;
}

Battle::Phase Battle::GetPhase() const
{
    return phase;
}

const Party& Battle::GetParty() const
{
    return party;
}

const Inventory& Battle::GetInventory() const
{
    return inventory;
}

const std::vector<Enemy>& Battle::GetEnemies() const
{
    return enemies;
}

int Battle::GetSelectedTarget() const
{
    return selectedTarget;
}

int Battle::GetSelectedAlly() const
{
    return selectedAlly;
}

int Battle::GetSlashActor() const
{
    return slashActor;
}

int Battle::GetPlayerPoints() const
{
    return playerPoints;
}

int Battle::GetEnemyPoints() const
{
    return enemyPoints;
}

int Battle::GetPlayerHalfIcons() const
{
    return playerHalfIcons;
}

int Battle::GetEnemyHalfIcons() const
{
    return enemyHalfIcons;
}

int Battle::GetTurnNumber() const
{
    return turnNumber;
}

Battle::MenuState Battle::GetMenuState() const
{
    return menuState;
}

int Battle::GetCommandIndex() const
{
    return commandIndex;
}

int Battle::GetSkillMenuIndex() const
{
    return skillMenuIndex;
}

int Battle::GetItemMenuIndex() const
{
    return itemMenuIndex;
}

Battle::PendingAction Battle::GetPendingAction() const
{
    return pendingAction;
}

bool Battle::IsEnemySmirk() const
{
    return enemySmirk;
}

bool Battle::IsEnemyAnxious() const
{
    return enemyAnxious;
}

float Battle::GetSlashTimer() const
{
    return slashTimer;
}

float Battle::GetHitFlashTimer() const
{
    return hitFlashTimer;
}

int Battle::GetHitFlashTarget() const
{
    return hitFlashTarget;
}

bool Battle::IsHitFlashOnParty() const
{
    return hitFlashOnParty;
}

const std::vector<DamagePopup>& Battle::GetDamagePopups() const
{
    return damagePopups;
}

float Battle::GetBuffFxTimer() const
{
    return buffFxTimer;
}

bool Battle::IsBuffFxOnParty() const
{
    return buffFxOnParty;
}

SkillKind Battle::GetBuffFxKind() const
{
    return buffFxKind;
}

const std::string& Battle::GetActionBanner() const
{
    return actionBanner;
}

float Battle::GetActionBannerTimer() const
{
    return actionBannerTimer;
}

bool Battle::IsActionCinematic() const
{
    return actionCinematic;
}

bool Battle::IsActionWideParty() const
{
    return actionWideParty;
}

bool Battle::IsActionAttackPose() const
{
    return actionAttackPose;
}

float Battle::GetSupportFlashTimer() const
{
    return supportFlashTimer;
}

Color Battle::GetSupportFlashColor() const
{
    return supportFlashColor;
}

const std::vector<ActionText>& Battle::GetActionTexts() const
{
    return actionTexts;
}

int Battle::GetLastActorParty() const
{
    return lastActorParty;
}

int Battle::GetLastActorEnemy() const
{
    return lastActorEnemy;
}

bool Battle::HasLastActionTarget() const
{
    return lastActionHasTarget;
}

bool Battle::IsLastActionTargetOnParty() const
{
    return lastActionTargetOnParty;
}

int Battle::GetLastActionTarget() const
{
    return lastActionTargetIndex;
}

int Battle::GetBuffFxTarget() const
{
    return buffFxTarget;
}

int Battle::GetXpReward() const
{
    return xpReward;
}

int Battle::GetMoneyReward() const
{
    return moneyReward;
}

int Battle::GetItemReward() const
{
    return itemReward;
}

bool Battle::HasEquipmentReward() const
{
    return equipmentReward;
}

EquipmentId Battle::GetEquipmentReward() const
{
    return equipmentRewardId;
}

const std::string& Battle::GetMessage() const
{
    return message;
}














