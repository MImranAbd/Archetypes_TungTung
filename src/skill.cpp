#include "skill.h"

Skill::Skill(const char* name, SkillKind kind, Element element, int power,
    int levelRequired, const char* description)
    : name(name)
    , kind(kind)
    , element(element)
    , power(power)
    , levelRequired(levelRequired)
    , description(description)
{
}

const char* Skill::GetName() const
{
    return name;
}

SkillKind Skill::GetKind() const
{
    return kind;
}

Element Skill::GetElement() const
{
    return element;
}

int Skill::GetPower() const
{
    return power;
}

int Skill::GetLevelRequired() const
{
    return levelRequired;
}

const char* Skill::GetDescription() const
{
    return description;
}

bool Skill::IsAttack() const
{
    return kind == SkillKind::Attack || kind == SkillKind::AttackAll;
}

bool Skill::IsAttackAll() const
{
    return kind == SkillKind::AttackAll;
}

bool Skill::IsHealAll() const
{
    return kind == SkillKind::HealAll;
}

bool Skill::IsRevive() const
{
    return kind == SkillKind::Revive;
}

bool Skill::IsPartyBuff() const
{
    return kind == SkillKind::BuffAttack ||
        kind == SkillKind::BuffAttackAll ||
        kind == SkillKind::BuffDefense ||
        kind == SkillKind::BuffDefenseAll ||
        kind == SkillKind::BuffEvasion ||
        kind == SkillKind::BuffEvasionAll;
}

bool Skill::IsPartyBuffAll() const
{
    return kind == SkillKind::BuffAttackAll ||
        kind == SkillKind::BuffDefenseAll ||
        kind == SkillKind::BuffEvasionAll;
}

bool Skill::IsEnemyDebuff() const
{
    return kind == SkillKind::DebuffAttack ||
        kind == SkillKind::DebuffAttackAll ||
        kind == SkillKind::DebuffDefense ||
        kind == SkillKind::DebuffDefenseAll ||
        kind == SkillKind::DebuffAccuracy ||
        kind == SkillKind::DebuffAccuracyAll;
}

bool Skill::IsEnemyDebuffAll() const
{
    return kind == SkillKind::DebuffAttackAll ||
        kind == SkillKind::DebuffDefenseAll ||
        kind == SkillKind::DebuffAccuracyAll;
}

bool Skill::IsAllTarget() const
{
    return IsAttackAll() || IsHealAll() || IsPartyBuffAll() || IsEnemyDebuffAll();
}

bool Skill::TargetsAlly() const
{
    return kind == SkillKind::Heal ||
        kind == SkillKind::Revive ||
        (IsPartyBuff() && !IsPartyBuffAll());
}

AttackSkill::AttackSkill()
    : Skill("Attack", SkillKind::Attack, Element::Physical, 0, 1,
        "A basic physical strike.")
{
}

SlashSkill::SlashSkill()
    : Skill("Slash", SkillKind::Attack, Element::Physical, 0, 1,
        "A basic physical slash.")
{
}

CleaveSkill::CleaveSkill()
    : Skill("Cleave", SkillKind::Attack, Element::Physical, 2, 2,
        "A heavy physical strike.")
{
}

MightyBlowSkill::MightyBlowSkill()
    : Skill("Mighty Blow", SkillKind::Attack, Element::Physical, 3, 2,
        "A massive physical strike.")
{
}

StrikeSkill::StrikeSkill()
    : Skill("Strike", SkillKind::Attack, Element::Physical, 0, 1,
        "A basic physical strike.")
{
}

BufuSkill::BufuSkill()
    : Skill("Bufu", SkillKind::Attack, Element::Ice, 0, 1,
        "Light ice damage to one foe.")
{
}

BufulaSkill::BufulaSkill()
    : Skill("Bufula", SkillKind::Attack, Element::Ice, 2, 2,
        "Heavy ice damage to one foe.")
{
}

AgiSkill::AgiSkill()
    : Skill("Agi", SkillKind::Attack, Element::Fire, 0, 1,
        "Light fire damage to one foe.")
{
}

AgilaoSkill::AgilaoSkill()
    : Skill("Agilao", SkillKind::Attack, Element::Fire, 2, 2,
        "Heavy fire damage to one foe.")
{
}

DiaSkill::DiaSkill()
    : Skill("Dia", SkillKind::Heal, Element::Ice, 5, 1,
        "Restores 5 HP to the weakest ally.")
{
}

MediaSkill::MediaSkill()
    : Skill("Media", SkillKind::HealAll, Element::Fire, 3, 1,
        "Restores 3 HP to all allies.")
{
}

TarukajaSkill::TarukajaSkill()
    : Skill("Tarukaja", SkillKind::BuffAttack, Element::Physical, 0, 1,
        "Raises attack for one ally for 3 turns.")
{
}

RakukajaSkill::RakukajaSkill()
    : Skill("Rakukaja", SkillKind::BuffDefense, Element::Physical, 0, 1,
        "Raises defense for one ally for 3 turns.")
{
}

SukukajaSkill::SukukajaSkill()
    : Skill("Sukukaja", SkillKind::BuffEvasion, Element::Physical, 0, 1,
        "Raises evasion for one ally for 3 turns.")
{
}

TarundaSkill::TarundaSkill()
    : Skill("Tarunda", SkillKind::DebuffAttack, Element::Fire, 0, 1,
        "Lowers one enemy's attack for 3 turns.")
{
}

RakundaSkill::RakundaSkill()
    : Skill("Rakunda", SkillKind::DebuffDefense, Element::Ice, 0, 1,
        "Lowers one enemy's defense for 3 turns.")
{
}

SukundaSkill::SukundaSkill()
    : Skill("Sukunda", SkillKind::DebuffAccuracy, Element::Physical, 0, 1,
        "Lowers one enemy's accuracy for 3 turns.")
{
}

SkewerSkill::SkewerSkill()
    : Skill("Skewer", SkillKind::Attack, Element::Physical, 3, 3,
        "A precise thrust that deals heavy physical damage.")
{
}

RampageSkill::RampageSkill()
    : Skill("Rampage", SkillKind::AttackAll, Element::Physical, 2, 4,
        "A wild assault that strikes all foes.")
{
}

DiaramaSkill::DiaramaSkill()
    : Skill("Diarama", SkillKind::Heal, Element::Ice, 10, 4,
        "Restores 10 HP to one ally.")
{
}

MaragionSkill::MaragionSkill()
    : Skill("Maragion", SkillKind::AttackAll, Element::Fire, 2, 4,
        "Heavy fire damage to all foes.")
{
}

HamaSkill::HamaSkill()
    : Skill("Hama", SkillKind::Attack, Element::Light, 2, 3,
        "Light damage to one foe.")
{
}

MudoSkill::MudoSkill()
    : Skill("Mudo", SkillKind::Attack, Element::Dark, 2, 3,
        "Dark damage to one foe.")
{
}

MahamaSkill::MahamaSkill()
    : Skill("Mahama", SkillKind::AttackAll, Element::Light, 1, 4,
        "Light damage to all foes.")
{
}

MamudoSkill::MamudoSkill()
    : Skill("Mamudo", SkillKind::AttackAll, Element::Dark, 1, 4,
        "Dark damage to all foes.")
{
}

MediaramaSkill::MediaramaSkill()
    : Skill("Mediarama", SkillKind::HealAll, Element::Light, 6, 5,
        "Restores 6 HP to all allies.")
{
}

RecarmSkill::RecarmSkill()
    : Skill("Recarm", SkillKind::Revive, Element::Light, 10, 4,
        "Revives one fallen ally with 10 HP.")
{
}

HeatWaveSkill::HeatWaveSkill()
    : Skill("Heat Wave", SkillKind::AttackAll, Element::Physical, 1, 3,
        "Physical damage to all foes.")
{
}

MazioSkill::MazioSkill()
    : Skill("Mazio", SkillKind::AttackAll, Element::Ice, 0, 3,
        "Light ice damage to all foes.")
{
}

MaragiSkill::MaragiSkill()
    : Skill("Maragi", SkillKind::AttackAll, Element::Fire, 0, 3,
        "Light fire damage to all foes.")
{
}

MatarukajaSkill::MatarukajaSkill()
    : Skill("Matarukaja", SkillKind::BuffAttackAll, Element::Physical, 0, 3,
        "Raises attack for all allies for 3 turns.")
{
}

MarakukajaSkill::MarakukajaSkill()
    : Skill("Marakukaja", SkillKind::BuffDefenseAll, Element::Physical, 0, 3,
        "Raises defense for all allies for 3 turns.")
{
}

MasukukajaSkill::MasukukajaSkill()
    : Skill("Masukukaja", SkillKind::BuffEvasionAll, Element::Physical, 0, 3,
        "Raises evasion for all allies for 3 turns.")
{
}

MatarundaSkill::MatarundaSkill()
    : Skill("Matarunda", SkillKind::DebuffAttackAll, Element::Fire, 0, 3,
        "Lowers all enemies' attack for 3 turns.")
{
}

MarakundaSkill::MarakundaSkill()
    : Skill("Marakunda", SkillKind::DebuffDefenseAll, Element::Ice, 0, 3,
        "Lowers all enemies' defense for 3 turns.")
{
}

MasukundaSkill::MasukundaSkill()
    : Skill("Masukunda", SkillKind::DebuffAccuracyAll, Element::Physical, 0, 3,
        "Lowers all enemies' accuracy for 3 turns.")
{
}
