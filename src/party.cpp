#include "party.h"

#include <iostream>

const char* RoleName(Role role)
{
    switch (role)
    {
        case Role::Mage: return "MAGE";
        case Role::Thief: return "THIEF";
        case Role::Warrior: return "WARRIOR";
        case Role::Priest: return "PRIEST";
    }

    return "NONE";
}

const char* RoleDescription(Role role)
{
    switch (role)
    {
        case Role::Mage: return "Elemental caster. Fire, Ice, Light, and Dark.";
        case Role::Thief: return "Single-target physical strikes and debuffs.";
        case Role::Warrior: return "Front-line fighter. Fire and Physical.";
        case Role::Priest: return "Support healer with Light magic.";
    }

    return "";
}

SkillList SkillsForRole(Role role)
{
    switch (role)
    {
        case Role::Mage:
            return {
                std::make_shared<AgiSkill>(),
                std::make_shared<BufuSkill>(),
                std::make_shared<HamaSkill>(),
                std::make_shared<MudoSkill>(),
                std::make_shared<MaragionSkill>(),
                std::make_shared<MazioSkill>(),
                std::make_shared<MahamaSkill>(),
                std::make_shared<MamudoSkill>(),
                std::make_shared<MasukukajaSkill>(),
                std::make_shared<ConcentrateSkill>()
            };

        case Role::Thief:
            return {
                std::make_shared<StrikeSkill>(),
                std::make_shared<SkewerSkill>(),
                std::make_shared<MightyBlowSkill>(),
                std::make_shared<TarundaSkill>(),
                std::make_shared<RakundaSkill>(),
                std::make_shared<SukundaSkill>()
            };

        case Role::Warrior:
            return {
                std::make_shared<CleaveSkill>(),
                std::make_shared<HeatWaveSkill>(),
                std::make_shared<MightyBlowSkill>(),
                std::make_shared<AgiSkill>(),
                std::make_shared<AgilaoSkill>(),
                std::make_shared<MaragionSkill>(),
                std::make_shared<TarukajaSkill>(),
                std::make_shared<ChargeSkill>()
            };

        case Role::Priest:
            return {
                std::make_shared<DiaSkill>(),
                std::make_shared<DiaramaSkill>(),
                std::make_shared<MediaSkill>(),
                std::make_shared<MediaramaSkill>(),
                std::make_shared<RecarmSkill>(),
                std::make_shared<RakukajaSkill>(),
                std::make_shared<MarakukajaSkill>(),
                std::make_shared<HamaSkill>(),
                std::make_shared<MahamaSkill>()
            };
    }

    return {};
}

int RolePassiveCount(Role role)
{
    (void)role;
    return 2;
}

const Passive& RolePassive(Role role, int index)
{
    static const Passive magePassives[2] = {
        { "Arcane Affinity", "+1 damage with Fire, Ice, Light, and Dark skills." },
        { "Focused Mind", "+5% accuracy." }
    };
    static const Passive thiefPassives[2] = {
        { "Keen Eye", "+10% critical chance." },
        { "Nimble", "Enemies have -10% accuracy against you." }
    };
    static const Passive warriorPassives[2] = {
        { "Weapon Mastery", "+1 damage with Physical skills." },
        { "Iron Body", "Takes 1 less damage from enemy attacks." }
    };
    static const Passive priestPassives[2] = {
        { "Blessed Hands", "Healing skills restore 2 extra HP." },
        { "Radiance", "+1 damage with Light skills." }
    };

    if (index < 0 || index > 1)
        index = 0;

    switch (role)
    {
        case Role::Mage: return magePassives[index];
        case Role::Thief: return thiefPassives[index];
        case Role::Warrior: return warriorPassives[index];
        case Role::Priest: return priestPassives[index];
    }

    return magePassives[0];
}

namespace
{
    Character MakeCharacter(const char* name, Element element, Element weakness,
        int baseHp, int baseAttack, int baseDefense, Role role)
    {
        Character character{};

        character.name = name;
        character.role = role;
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
        character.charged = false;
        character.concentrated = false;
        character.skills = SkillsForRole(role);

        return character;
    }
}

Party::Party()
    : activeIndex(0)
    , money(0)
{
    members.push_back(MakeCharacter(
        "Eliza", Element::Light, Element::Dark, 10, 4, 1, Role::Mage));

    members.push_back(MakeCharacter(
        "Ares", Element::Fire, Element::Ice, 14, 4, 3, Role::Warrior));

    members.push_back(MakeCharacter(
        "Leo", Element::Light, Element::Dark, 11, 3, 2, Role::Priest));

    members.push_back(MakeCharacter(
        "Troy", Element::Physical, Element::Light, 12, 4, 2, Role::Thief));

    for (Character& member : members)
    {
        RecalculateStats(member);
        member.hp = member.maxHp;
    }
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
        member.charged = false;
        member.concentrated = false;
    }

    activeIndex = 0;
}

void Party::ReviveFallenOneHp()
{
    for (Character& member : members)
    {
        member.smirk = false;
        member.anxious = false;
        member.charged = false;
        member.concentrated = false;

        if (member.hp <= 0)
            member.hp = 1;
    }
}

int Party::XpForNextLevel(int level)
{
    return 20 + (level - 1) * 8;
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

void Party::SetRole(int memberIndex, Role role)
{
    if (memberIndex < 0 || memberIndex >= static_cast<int>(members.size()))
        return;

    Character& member = members[memberIndex];
    member.role = role;
    member.skills = SkillsForRole(role);
    RecalculateStats(member);
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


