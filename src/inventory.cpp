#include "inventory.h"

const char* ItemName(ItemType type)
{
    switch (type)
    {
        case ItemType::Herb:   return "Herb";
        case ItemType::Potion: return "Potion";
    }

    return "ITEM";
}

int ItemHeal(ItemType type)
{
    switch (type)
    {
        case ItemType::Herb:   return 5;
        case ItemType::Potion: return 10;
    }

    return 0;
}

const char* ItemCategoryName(ItemCategory category)
{
    switch (category)
    {
        case ItemCategory::Healing: return "HEALING";
        case ItemCategory::Weapon:  return "WEAPON";
        case ItemCategory::Attack:  return "ATTACK";
        case ItemCategory::Other:   return "OTHER";
    }

    return "OTHER";
}

ItemCategory ItemCategoryOf(ItemType type)
{
    switch (type)
    {
        case ItemType::Herb:
        case ItemType::Potion:
            return ItemCategory::Healing;
    }

    return ItemCategory::Other;
}

bool ItemTargetsAll(ItemType type)
{
    (void)type;
    return false;
}

std::vector<const ItemStack*> ItemStacksInCategory(const Inventory& inventory,
    ItemCategory category)
{
    std::vector<const ItemStack*> result;

    for (const ItemStack& stack : inventory.Stacks())
    {
        if (ItemCategoryOf(stack.type) == category)
            result.push_back(&stack);
    }

    return result;
}

void Inventory::Add(ItemType type, int amount)
{
    for (ItemStack& stack : stacks)
    {
        if (stack.type == type)
        {
            stack.count += amount;
            return;
        }
    }

    stacks.push_back(ItemStack{ type, amount });
}

int Inventory::Count(ItemType type) const
{
    for (const ItemStack& stack : stacks)
    {
        if (stack.type == type) return stack.count;
    }

    return 0;
}

bool Inventory::Consume(ItemType type)
{
    for (int i = 0; i < static_cast<int>(stacks.size()); i++)
    {
        if (stacks[i].type != type) continue;
        if (stacks[i].count <= 0) return false;

        stacks[i].count--;

        if (stacks[i].count == 0)
            stacks.erase(stacks.begin() + i);

        return true;
    }

    return false;
}

const std::vector<ItemStack>& Inventory::Stacks() const
{
    return stacks;
}

void Inventory::AddEquipment(EquipmentId id, int amount)
{
    if (id == EquipmentId::None) return;

    for (EquipmentStack& stack : equipment)
    {
        if (stack.id == id)
        {
            stack.count += amount;
            return;
        }
    }

    equipment.push_back(EquipmentStack{ id, amount });
}

int Inventory::CountEquipment(EquipmentId id) const
{
    if (id == EquipmentId::None) return 0;

    for (const EquipmentStack& stack : equipment)
    {
        if (stack.id == id) return stack.count;
    }

    return 0;
}

bool Inventory::ConsumeEquipment(EquipmentId id)
{
    if (id == EquipmentId::None) return false;

    for (int i = 0; i < static_cast<int>(equipment.size()); i++)
    {
        if (equipment[i].id != id) continue;
        if (equipment[i].count <= 0) return false;

        equipment[i].count--;

        if (equipment[i].count == 0)
            equipment.erase(equipment.begin() + i);

        return true;
    }

    return false;
}

const std::vector<EquipmentStack>& Inventory::EquipmentStacks() const
{
    return equipment;
}

