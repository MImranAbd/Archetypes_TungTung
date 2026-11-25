#pragma once

#include <vector>
#include "enemies.h"
#include "equipment.h"
#include "inventory.h"
#include "skill.h"

enum class Role
{
    Mage,
    Thief,
    Warrior,
    Priest
};

struct Passive
{
    const char* name;
    const char* description;
};

const char* RoleName(Role role);
const char* RoleDescription(Role role);
int RolePassiveCount(Role role);
const Passive& RolePassive(Role role, int index);
SkillList SkillsForRole(Role role);

struct Character
{
    const char* name;
    Role role;
    Element element;
    Element weakness;
    int hp;
    int maxHp;
    int attack;
    int defense;
    int level;
    int xp;
    int xpToNext;
    int baseMaxHp;
    int baseAttack;
    int baseDefense;
    EquipmentId weapon;
    EquipmentId armor;
    EquipmentId accessory;
    bool smirk;
    bool anxious;
    bool charged;
    bool concentrated;
    SkillList skills;
};

std::vector<EquipmentId> EquipmentOptions(const Character& member,
    EquipSlot slot, const Inventory& inventory);

class Party
{
public:
    Party();

    std::vector<Character>& Members();
    const std::vector<Character>& Members() const;

    Character& Active();
    const Character& Active() const;

    int GetActiveIndex() const;
    void SwitchActive();
    bool IsWipedOut() const;

    int GetTotalHp() const;
    int GetTotalMaxHp() const;
    int GetMoney() const;
    void AddMoney(int amount);
    void RestoreAll();
    void ReviveFallenOneHp();

    void GainXp(int amount);
    void RecalculateStats(Character& member);
    bool CycleEquipment(int memberIndex, EquipSlot slot, Inventory& inventory);
    bool EquipItem(int memberIndex, EquipSlot slot, EquipmentId id,
        Inventory& inventory);
    void SetRole(int memberIndex, Role role);

    static int XpForNextLevel(int level);

private:
    std::vector<Character> members;
    int activeIndex;
    int money;
};

