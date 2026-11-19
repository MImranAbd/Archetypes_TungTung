#pragma once

#include <vector>
#include "enemies.h"
#include "equipment.h"
#include "inventory.h"
#include "skill.h"

struct Character
{
    const char* name;
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
    SkillList skills;
};

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

    void GainXp(int amount);
    void RecalculateStats(Character& member);
    bool CycleEquipment(int memberIndex, EquipSlot slot, Inventory& inventory);

    static int XpForNextLevel(int level);

private:
    std::vector<Character> members;
    int activeIndex;
    int money;
};

