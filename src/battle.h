#pragma once

#include <string>
#include <vector>
#include "enemies.h"
#include "party.h"
#include "inventory.h"

struct BattleBuff
{
    int stacks;
    int turns;
};

struct BattleBuffs
{
    BattleBuff attackUp;
    BattleBuff defenseUp;
    BattleBuff evasionUp;
    BattleBuff enemyAttackDown;
    BattleBuff enemyDefenseDown;
    BattleBuff enemyAccuracyDown;
};

struct DamagePopup
{
    int target;
    bool onParty;
    int amount;
    bool crit;
    bool weak;
    float timer;
};

class Battle
{
public:
    enum class Phase { PlayerTurn, EnemyTurn, Victory, Defeat };
    enum class MenuState { Command, Skill, Item, Target };
    enum class PendingAction { None, Attack, Skill, Item };
    enum class Command { Attack, Skill, Defense, Item, Pass, Escape };

    static constexpr int commandCount = 6;
    static constexpr float actionDuration = 1.4f;
    static constexpr float hitFlashDuration = 0.6f;
    static constexpr float damagePopupDuration = 1.2f;

    explicit Battle(Party& party, Inventory& inventory, int floorNumber,
        bool enemyAmbush = false);

    void Update();

    bool IsFinished() const;
    bool PlayerWon() const;
    bool WasEscaped() const;
    bool IsElite() const;

    Phase GetPhase() const;
    const Party& GetParty() const;
    const Inventory& GetInventory() const;
    const std::vector<Enemy>& GetEnemies() const;
    int GetSelectedTarget() const;
    int GetSelectedAlly() const;
    int GetSlashActor() const;
    int GetPlayerPoints() const;
    int GetEnemyPoints() const;
    int GetTurnNumber() const;
    bool IsEnemySmirk() const;
    bool IsEnemyAnxious() const;
    float GetSlashTimer() const;
    float GetHitFlashTimer() const;
    int GetHitFlashTarget() const;
    bool IsHitFlashOnParty() const;
    MenuState GetMenuState() const;
    int GetCommandIndex() const;
    int GetSkillMenuIndex() const;
    int GetItemMenuIndex() const;
    PendingAction GetPendingAction() const;
    const std::vector<DamagePopup>& GetDamagePopups() const;
    float GetBuffFxTimer() const;
    bool IsBuffFxOnParty() const;
    SkillKind GetBuffFxKind() const;
    const std::string& GetActionBanner() const;
    float GetActionBannerTimer() const;
    int GetLastActorParty() const;
    int GetLastActorEnemy() const;
    bool HasLastActionTarget() const;
    bool IsLastActionTargetOnParty() const;
    int GetLastActionTarget() const;
    int GetBuffFxTarget() const;
    int GetXpReward() const;
    int GetMoneyReward() const;
    int GetItemReward() const;
    bool HasEquipmentReward() const;
    EquipmentId GetEquipmentReward() const;
    const std::string& GetMessage() const;

    void SwitchCharacter();

private:
    void StartPlayerTurn();
    void StartEnemyTurn();
    void ExecuteCommand(int command);
    void UseSkill(int skillIndex);
    bool UseItem(ItemType type, int targetIndex);
    void BeginTargetSelection(PendingAction action);
    void ExecutePendingAction();
    void PlayerAttack(const Skill& skill);
    void PlayerAttackAll(const Skill& skill);
    void Defend();
    void TryEscape();
    void ResolveEnemyAttack();
    void TickBuffs();
    void RotateCharacter();
    void SelectTarget(int direction);
    void SelectAlly(int direction);
    void ResetMenu();
    void AddDamagePopup(int target, bool onParty, int amount, bool crit, bool weak);
    void SetBuffEffect(SkillKind kind, bool onParty, int target);
    int FindNextAliveEnemy(int start) const;
    int FindLowestHpAlly() const;
    int LivingPartyCount() const;
    int LivingEnemyCount() const;
    bool AllEnemiesDead() const;
    void LogState() const;

    Party& party;
    Inventory& inventory;
    int floorNumber;
    int enemyBaseDamage;
    bool isElite;
    std::vector<Enemy> enemies;
    std::vector<bool> weaknessKnown;
    Phase phase;
    float turnTimer;
    float slashTimer;
    float hitFlashTimer;
    int hitFlashTarget;
    bool hitFlashOnParty;
    bool finished;
    bool won;
    bool escaped;

    int playerPoints;
    int enemyPoints;
    int selectedTarget;
    int selectedAlly;
    int slashActor;
    int enemyTurnIndex;
    int turnNumber;
    bool defending;
    bool enemySmirk;
    bool enemyAnxious;
    BattleBuffs partyBuffs[4];
    BattleBuffs enemyBuffs[5];
    std::string message;

    MenuState menuState;
    int commandIndex;
    PendingAction pendingAction;
    MenuState targetReturnState;
    int skillMenuIndex;
    int itemMenuIndex;

    std::vector<DamagePopup> damagePopups;
    float buffFxTimer;
    bool buffFxOnParty;
    int buffFxTarget;
    SkillKind buffFxKind;
    std::string actionBanner;
    float actionBannerTimer;
    int lastActorParty;
    int lastActorEnemy;
    bool lastActionHasTarget;
    bool lastActionTargetOnParty;
    int lastActionTargetIndex;

    int xpReward;
    int moneyReward;
    int itemReward;
    bool equipmentReward;
    EquipmentId equipmentRewardId;
};






