#pragma once

#include <memory>
#include <vector>
#include "enemies.h"

enum class SkillKind
{
    Attack,
    AttackAll,
    Heal,
    HealAll,
    Revive,
    BuffAttack,
    BuffAttackAll,
    BuffDefense,
    BuffDefenseAll,
    BuffEvasion,
    BuffEvasionAll,
    DebuffAttack,
    DebuffAttackAll,
    DebuffDefense,
    DebuffDefenseAll,
    DebuffAccuracy,
    DebuffAccuracyAll
};

class Skill
{
public:
    virtual ~Skill() = default;

    const char* GetName() const;
    SkillKind GetKind() const;
    Element GetElement() const;
    int GetPower() const;
    int GetLevelRequired() const;
    const char* GetDescription() const;

    bool IsAttack() const;
    bool IsAttackAll() const;
    bool IsHealAll() const;
    bool IsRevive() const;
    bool IsPartyBuff() const;
    bool IsPartyBuffAll() const;
    bool IsEnemyDebuff() const;
    bool IsEnemyDebuffAll() const;
    bool IsAllTarget() const;
    bool TargetsAlly() const;

protected:
    Skill(const char* name, SkillKind kind, Element element, int power,
        int levelRequired, const char* description);

private:
    const char* name;
    SkillKind kind;
    Element element;
    int power;
    int levelRequired;
    const char* description;
};

using SkillList = std::vector<std::shared_ptr<Skill>>;

class AttackSkill : public Skill
{
public:
    AttackSkill();
};

class SlashSkill : public Skill
{
public:
    SlashSkill();
};

class CleaveSkill : public Skill
{
public:
    CleaveSkill();
};

class MightyBlowSkill : public Skill
{
public:
    MightyBlowSkill();
};

class StrikeSkill : public Skill
{
public:
    StrikeSkill();
};

class BufuSkill : public Skill
{
public:
    BufuSkill();
};

class BufulaSkill : public Skill
{
public:
    BufulaSkill();
};

class AgiSkill : public Skill
{
public:
    AgiSkill();
};

class AgilaoSkill : public Skill
{
public:
    AgilaoSkill();
};

class DiaSkill : public Skill
{
public:
    DiaSkill();
};

class MediaSkill : public Skill
{
public:
    MediaSkill();
};

class TarukajaSkill : public Skill
{
public:
    TarukajaSkill();
};

class RakukajaSkill : public Skill
{
public:
    RakukajaSkill();
};

class SukukajaSkill : public Skill
{
public:
    SukukajaSkill();
};

class TarundaSkill : public Skill
{
public:
    TarundaSkill();
};

class RakundaSkill : public Skill
{
public:
    RakundaSkill();
};

class SukundaSkill : public Skill
{
public:
    SukundaSkill();
};

class SkewerSkill : public Skill
{
public:
    SkewerSkill();
};

class RampageSkill : public Skill
{
public:
    RampageSkill();
};

class DiaramaSkill : public Skill
{
public:
    DiaramaSkill();
};

class MaragionSkill : public Skill
{
public:
    MaragionSkill();
};

class HamaSkill : public Skill
{
public:
    HamaSkill();
};

class MudoSkill : public Skill
{
public:
    MudoSkill();
};

class MahamaSkill : public Skill
{
public:
    MahamaSkill();
};

class MamudoSkill : public Skill
{
public:
    MamudoSkill();
};

class MediaramaSkill : public Skill
{
public:
    MediaramaSkill();
};

class RecarmSkill : public Skill
{
public:
    RecarmSkill();
};

class HeatWaveSkill : public Skill
{
public:
    HeatWaveSkill();
};

class MazioSkill : public Skill
{
public:
    MazioSkill();
};

class MaragiSkill : public Skill
{
public:
    MaragiSkill();
};

class MatarukajaSkill : public Skill
{
public:
    MatarukajaSkill();
};

class MarakukajaSkill : public Skill
{
public:
    MarakukajaSkill();
};

class MasukukajaSkill : public Skill
{
public:
    MasukukajaSkill();
};

class MatarundaSkill : public Skill
{
public:
    MatarundaSkill();
};

class MarakundaSkill : public Skill
{
public:
    MarakundaSkill();
};

class MasukundaSkill : public Skill
{
public:
    MasukundaSkill();
};
