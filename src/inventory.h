#pragma once

#include <vector>
#include "equipment.h"

enum class ItemType
{
    Herb,
    Potion,
    Revive
};

enum class ItemCategory
{
    Healing,
    Weapon,
    Attack,
    Other
};

struct ItemStack
{
    ItemType type;
    int count;
};

struct EquipmentStack
{
    EquipmentId id;
    int count;
};

const char* ItemName(ItemType type);
int ItemHeal(ItemType type);
const char* ItemCategoryName(ItemCategory category);
ItemCategory ItemCategoryOf(ItemType type);
bool ItemTargetsAll(ItemType type);
bool ItemRevives(ItemType type);
int ItemReviveHp(ItemType type);

class Inventory
{
public:
    void Add(ItemType type, int amount = 1);
    int Count(ItemType type) const;
    bool Consume(ItemType type);
    const std::vector<ItemStack>& Stacks() const;

    void AddEquipment(EquipmentId id, int amount = 1);
    int CountEquipment(EquipmentId id) const;
    bool ConsumeEquipment(EquipmentId id);
    const std::vector<EquipmentStack>& EquipmentStacks() const;

private:
    std::vector<ItemStack> stacks;
    std::vector<EquipmentStack> equipment;
};

std::vector<const ItemStack*> ItemStacksInCategory(const Inventory& inventory,
    ItemCategory category);
