#pragma once

enum class EquipSlot
{
    Weapon,
    Armor,
    Accessory
};

enum class EquipmentId
{
    None = -1,
    IronSword = 0,
    OakStaff,
    HunterBow,
    LeatherArmor,
    Chainmail,
    Robe,
    PowerRing,
    WardRing,
    VitalityCharm,
    Count
};

struct EquipmentDef
{
    const char* name;
    EquipSlot slot;
    int hp;
    int attack;
    int defense;
};

const EquipmentDef& GetEquipment(EquipmentId id);
const char* EquipSlotName(EquipSlot slot);
const EquipmentId* AllEquipmentIds();
int AllEquipmentCount();
