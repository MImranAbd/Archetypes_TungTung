#include "party.h"

#include <iostream>

namespace
{
    Character MakeCharacter(const char* name, Element element, Element weakness,
        int baseHp, int baseAttack, int baseDefense, SkillList skills)
    {
        Character character{};

        character.name = name;
        character.element = element;
        character.weakness = weakness;
        character.level = 20;
        character.xp = 0;
        character.xpToNext = Party::XpForNextLevel(1);
        character.baseMaxHp = baseHp;
        character.baseAttack = baseAttack;
        character.baseDefense = baseDefense;
        character.maxHp = baseHp;
        character.hp = baseHp;
        character.attack = baseAttack;
        character.defense = baseDefense;
        character.weapon = EquipmentId::None;
        character.armor = EquipmentId::None;
        character.accessory = EquipmentId::None;
        character.smirk = false;
        character.anxious = false;
        character.skills = std::move(skills);

        return character;
    }
}

Party::Party()
    : activeIndex(0)
    , money(0)
{
    members.push_back(MakeCharacter(
        "Eliza", Element::Physical, Element::Ice, 12, 3, 2,
        {
            std::make_shared<SlashSkill>(),
            std::make_shared<TarukajaSkill>(),
            std::make_shared<RakukajaSkill>(),
            std::make_shared<CleaveSkill>(),
            std::make_shared<MatarukajaSkill>()
        }));

    members.push_back(MakeCharacter(
        "Ares", Element::Ice, Element::Fire, 10, 3, 1,
        {
            std::make_shared<BufuSkill>(),
            std::make_shared<RakundaSkill>(),
            std::make_shared<DiaSkill>(),
            std::make_shared<BufulaSkill>(),
            std::make_shared<MazioSkill>()
        }));

    members.push_back(MakeCharacter(
        "Leo", Element::Fire, Element::Ice, 11, 3, 1,
        {
            std::make_shared<AgiSkill>(),
            std::make_shared<TarundaSkill>(),
            std::make_shared<MediaSkill>(),
            std::make_shared<AgilaoSkill>(),
            std::make_shared<MaragiSkill>()
        }));

    members.push_back(MakeCharacter(
        "Troy", Element::Physical, Element::Fire, 14, 2, 3,
        {
            std::make_shared<StrikeSkill>(),
            std::make_shared<SukukajaSkill>(),
            std::make_shared<SukundaSkill>(),
            std::make_shared<MightyBlowSkill>(),
            std::make_shared<MasukukajaSkill>()
        }));
}

std::vector<Character>& Party::Members()
{
    return members;
}

const std::vector<Character>& Party::Members() const
{
    return members;
}

Character& Party::Active()
{
    return members[activeIndex];
}

const Character& Party::Active() const
{
    return members[activeIndex];
}

int Party::GetActiveIndex() const
{
    return activeIndex;
}

void Party::SwitchActive()
{
    const int count = static_cast<int>(members.size());

    for (int i = 1; i <= count; i++)
    {
        const int index = (activeIndex + i) % count;

        if (members[index].hp > 0)
        {
            activeIndex = index;
            return;
        }
    }
}

bool Party::IsWipedOut() const
{
    for (const Character& member : members)
    {
        if (member.hp > 0) return false;
    }

    return true;
}

int Party::GetTotalHp() const
{
    int total = 0;

    for (const Character& member : members)
    {
        total += member.hp;
    }

    return total;
}

int Party::GetTotalMaxHp() const
{
    int total = 0;

    for (const Character& member : members)
    {
        total += member.maxHp;
    }

    return total;
}

int Party::GetMoney() const
{
    return money;
}

void Party::AddMoney(int amount)
{
    money += amount;
    if (money < 0) money = 0;
}

void Party::RestoreAll()
{
    for (Character& member : members)
    {
        member.hp = member.maxHp;
        member.smirk = false;
        member.anxious = false;
    }

    activeIndex = 0;
}

void Party::ReviveFallenOneHp()
{
    for (Character& member : members)
    {
        if (member.hp <= 0)
        {
            member.hp = 1;
            member.smirk = false;
            member.anxious = false;
        }
    }
}

int Party::XpForNextLevel(int level)
{
    return 20 + (level - 1) * 15;
}

void Party::RecalculateStats(Character& member)
{
    const EquipmentDef& weapon = GetEquipment(member.weapon);
    const EquipmentDef& armor = GetEquipment(member.armor);
    const EquipmentDef& accessory = GetEquipment(member.accessory);

    const int oldMaxHp = member.maxHp;

    member.maxHp = member.baseMaxHp + (member.level - 1) * 2
        + weapon.hp + armor.hp + accessory.hp;
    member.attack = member.baseAttack + (member.level - 1)
        + weapon.attack + armor.attack + accessory.attack;
    member.defense = member.baseDefense + (member.level - 1) / 2
        + weapon.defense + armor.defense + accessory.defense;

    if (member.maxHp < 1) member.maxHp = 1;

    if (member.hp > 0 && member.maxHp > oldMaxHp)
        member.hp += member.maxHp - oldMaxHp;

    if (member.hp > member.maxHp) member.hp = member.maxHp;
    if (member.hp < 0) member.hp = 0;
}

void Party::GainXp(int amount)
{
    for (Character& member : members)
    {
        member.xp += amount;

        while (member.xp >= member.xpToNext)
        {
            member.xp -= member.xpToNext;
            member.level++;
            member.xpToNext = XpForNextLevel(member.level);
            RecalculateStats(member);
            member.hp = member.maxHp;

            std::cout << "[LEVEL] " << member.name << " reached level "
                      << member.level << "\n";
        }
    }
}

std::vector<EquipmentId> EquipmentOptions(const Character& member,
    EquipSlot slot, const Inventory& inventory)
{
    std::vector<EquipmentId> options;
    options.push_back(EquipmentId::None);

    EquipmentId current = EquipmentId::None;
    if (slot == EquipSlot::Weapon) current = member.weapon;
    else if (slot == EquipSlot::Armor) current = member.armor;
    else current = member.accessory;

    const EquipmentId* allIds = AllEquipmentIds();
    const int allCount = AllEquipmentCount();

    for (int i = 0; i < allCount; i++)
    {
        const EquipmentId id = allIds[i];

        if (GetEquipment(id).slot != slot) continue;
        if (id == current || inventory.CountEquipment(id) > 0)
            options.push_back(id);
    }

    return options;
}

bool Party::EquipItem(int memberIndex, EquipSlot slot, EquipmentId id,
    Inventory& inventory)
{
    if (memberIndex < 0 || memberIndex >= static_cast<int>(members.size()))
        return false;

    Character& member = members[memberIndex];

    EquipmentId current = EquipmentId::None;
    if (slot == EquipSlot::Weapon) current = member.weapon;
    else if (slot == EquipSlot::Armor) current = member.armor;
    else current = member.accessory;

    if (id == current)
        return false;

    if (id != EquipmentId::None && !inventory.ConsumeEquipment(id))
        return false;

    if (current != EquipmentId::None)
        inventory.AddEquipment(current, 1);

    if (slot == EquipSlot::Weapon) member.weapon = id;
    else if (slot == EquipSlot::Armor) member.armor = id;
    else member.accessory = id;

    RecalculateStats(member);
    return true;
}

bool Party::CycleEquipment(int memberIndex, EquipSlot slot, Inventory& inventory)
{
    if (memberIndex < 0 || memberIndex >= static_cast<int>(members.size()))
        return false;

    Character& member = members[memberIndex];

    EquipmentId current = EquipmentId::None;
    if (slot == EquipSlot::Weapon) current = member.weapon;
    else if (slot == EquipSlot::Armor) current = member.armor;
    else current = member.accessory;

    std::vector<EquipmentId> options;
    options.push_back(EquipmentId::None);

    const EquipmentId* allIds = AllEquipmentIds();
    const int allCount = AllEquipmentCount();

    for (int i = 0; i < allCount; i++)
    {
        const EquipmentId id = allIds[i];

        if (GetEquipment(id).slot != slot) continue;
        if (id == current || inventory.CountEquipment(id) > 0)
            options.push_back(id);
    }

    int currentIndex = 0;
    for (int i = 0; i < static_cast<int>(options.size()); i++)
    {
        if (options[i] == current)
        {
            currentIndex = i;
            break;
        }
    }

    const EquipmentId next = options[(currentIndex + 1) % options.size()];

    if (next == current)
        return false;

    if (current != EquipmentId::None)
        inventory.AddEquipment(current, 1);

    if (next != EquipmentId::None)
    {
        if (!inventory.ConsumeEquipment(next))
            return false;
    }

    if (slot == EquipSlot::Weapon) member.weapon = next;
    else if (slot == EquipSlot::Armor) member.armor = next;
    else member.accessory = next;

    RecalculateStats(member);
    return true;
}


