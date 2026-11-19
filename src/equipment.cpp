#include "equipment.h"

namespace
{
    const EquipmentDef EquipmentTable[] = {
        { "Iron Sword", EquipSlot::Weapon, 0, 2, 0 },
        { "Oak Staff", EquipSlot::Weapon, 0, 1, 1 },
        { "Hunter Bow", EquipSlot::Weapon, 3, 1, 0 },
        { "Leather Armor", EquipSlot::Armor, 5, 0, 2 },
        { "Chainmail", EquipSlot::Armor, 8, 0, 3 },
        { "Robe", EquipSlot::Armor, 4, 1, 1 },
        { "Power Ring", EquipSlot::Accessory, 0, 2, 0 },
        { "Ward Ring", EquipSlot::Accessory, 0, 0, 2 },
        { "Vitality Charm", EquipSlot::Accessory, 10, 0, 0 }
    };

    const EquipmentId AllIds[] = {
        EquipmentId::IronSword,
        EquipmentId::OakStaff,
        EquipmentId::HunterBow,
        EquipmentId::LeatherArmor,
        EquipmentId::Chainmail,
        EquipmentId::Robe,
        EquipmentId::PowerRing,
        EquipmentId::WardRing,
        EquipmentId::VitalityCharm
    };

    const EquipmentDef EmptyEquipment = { "NONE", EquipSlot::Weapon, 0, 0, 0 };
}

const EquipmentDef& GetEquipment(EquipmentId id)
{
    if (id == EquipmentId::None || id == EquipmentId::Count)
        return EmptyEquipment;

    return EquipmentTable[static_cast<int>(id)];
}

const char* EquipSlotName(EquipSlot slot)
{
    switch (slot)
    {
        case EquipSlot::Weapon:    return "WEAPON";
        case EquipSlot::Armor:     return "ARMOR";
        case EquipSlot::Accessory: return "ACCESSORY";
    }

    return "SLOT";
}

const EquipmentId* AllEquipmentIds()
{
    return AllIds;
}

int AllEquipmentCount()
{
    return static_cast<int>(sizeof(AllIds) / sizeof(AllIds[0]));
}

