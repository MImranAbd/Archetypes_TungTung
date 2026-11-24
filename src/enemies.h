#pragma once

class GameMap;

enum class Element
{
    Physical,
    Ice,
    Fire,
    Light,
    Dark
};

inline const char* ElementName(Element element)
{
    switch (element)
    {
        case Element::Physical: return "PHYS";
        case Element::Ice:      return "ICE";
        case Element::Fire:     return "FIRE";
        case Element::Light:    return "LIGHT";
        case Element::Dark:     return "DARK";
    }

    return "???";
}

enum class EnemyKind
{
    Slime,
    Bat,
    Skeleton,
    SkeletonArcher,
    Archer,
    GreatswordSkeleton,
    ArmoredSkeleton,
    Soldier,
    Swordsman,
    Lancer,
    Knight,
    KnightTemplar,
    Orc,
    EliteOrc,
    ArmoredOrc,
    ArmoredAxeman,
    OrcRider,
    Werewolf,
    Werebear,
    Wizard,
    Priest,
    Necromancer,
    Demon,
    BloodMonster,
    Count
};

constexpr int EnemyKindCount = static_cast<int>(EnemyKind::Count);

enum class EnemyActionKind
{
    BasicAttack,
    Damage,
    DamageAll,
    Drain,
    HealSelf,
    HealAlly,
    HealAllies,
    BuffSelf,
    BuffAllies,
    DebuffParty
};

enum class EnemyStat
{
    None,
    Attack,
    Defense,
    Evasion,
    Accuracy
};

struct EnemySkillDef
{
    const char* name;
    EnemyActionKind kind;
    Element element;
    EnemyStat stat;
    int power;
    float accuracy;
    int cost;
};

struct EnemyDef
{
    const char* name;
    const char* spritePath;
    Element weakness;
    int hpBonus;
    int attackBonus;
    EnemySkillDef skills[2];
};

const EnemyDef& GetEnemyDef(EnemyKind kind);
const char* EnemyKindName(EnemyKind kind);
EnemyKind BossKindForFloor(int floorNumber);
EnemyKind RandomEnemyKindForFloor(int floorNumber, bool elite);

class Enemy
{
public:
    Enemy(int startGridX, int startGridY, int maxHp = 6,
        EnemyKind kind = EnemyKind::Slime);
    void Update(int targetGridX, int targetGridY, const GameMap& map);
    void Draw() const;

    int GetGridX() const;
    int GetGridY() const;
    float GetPixelX() const;
    float GetPixelY() const;
    int GetHp() const;
    int GetMaxHp() const;
    bool IsAlive() const;
    bool IsActive() const;
    void SetActive(bool value);
    void TakeDamage(int amount);
    void Heal(int amount);
    void Reset(int startGridX, int startGridY, int maxHp = 6);
    void SetKind(EnemyKind kind);
    EnemyKind GetKind() const;
    Element GetWeakness() const;

    static constexpr int cellSize = 40;
    static constexpr float moveDelay = 0.4f;

private:
    int gridX;
    int gridY;
    float x;
    float y;
    float moveTimer;
    int radius;
    int hp;
    int maxHp;
    bool active;
    EnemyKind kind;
    Element weakness;
};
