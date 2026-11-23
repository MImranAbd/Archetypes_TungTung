#include "enemies.h"
#include "map.h"
#include "theme.h"
#include <raylib.h>
#include <cmath>
#include <cstdlib>

namespace
{
    const EnemyDef enemyDefs[EnemyKindCount] = {
        {
            "Slime",
            "res/draw/Character/Enemies list/Slime.png",
            Element::Fire, 0, 0,
            {
                { "Acid Spit", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 1, 0.90f, 2 },
                { "Split", EnemyActionKind::HealSelf, Element::Physical,
                    EnemyStat::None, 6, 1.0f, 2 }
            }
        },
        {
            "Bat",
            "res/draw/Character/Enemies list/Bat.png",
            Element::Ice, -1, 1,
            {
                { "Sonic Screech", EnemyActionKind::DebuffParty, Element::Physical,
                    EnemyStat::Accuracy, 0, 1.0f, 2 },
                { "Leech Bite", EnemyActionKind::Drain, Element::Physical,
                    EnemyStat::None, 1, 0.90f, 3 }
            }
        },
        {
            "Skeleton",
            "res/draw/Character/Enemies list/Skeleton.png",
            Element::Fire, 1, 0,
            {
                { "Bone Toss", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 1, 0.90f, 2 },
                { "Rattle Guard", EnemyActionKind::BuffSelf, Element::Physical,
                    EnemyStat::Defense, 0, 1.0f, 2 }
            }
        },
        {
            "Skeleton Archer",
            "res/draw/Character/Enemies list/Skeleton Archer.png",
            Element::Ice, 0, 1,
            {
                { "Aimed Shot", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.95f, 3 },
                { "Arrow Volley", EnemyActionKind::DamageAll, Element::Physical,
                    EnemyStat::None, 0, 0.80f, 3 }
            }
        },
        {
            "Archer",
            "res/draw/Character/Enemies list/Archer.png",
            Element::Ice, 0, 1,
            {
                { "Piercing Arrow", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.90f, 3 },
                { "Suppressing Fire", EnemyActionKind::DebuffParty, Element::Physical,
                    EnemyStat::Accuracy, 0, 1.0f, 2 }
            }
        },
        {
            "Greatsword Skeleton",
            "res/draw/Character/Enemies list/Greatsword Skeleton.png",
            Element::Fire, 3, 1,
            {
                { "Cleaving Arc", EnemyActionKind::DamageAll, Element::Physical,
                    EnemyStat::None, 1, 0.80f, 3 },
                { "Bone Crush", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.75f, 3 }
            }
        },
        {
            "Armored Skeleton",
            "res/draw/Character/Enemies list/Armored Skeleton.png",
            Element::Ice, 2, 0,
            {
                { "Iron Guard", EnemyActionKind::BuffSelf, Element::Physical,
                    EnemyStat::Defense, 0, 1.0f, 2 },
                { "Bone Spear", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.85f, 3 }
            }
        },
        {
            "Soldier",
            "res/draw/Character/Enemies list/Soldier.png",
            Element::Fire, 1, 0,
            {
                { "Shield Bash", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 1, 0.90f, 2 },
                { "Rally", EnemyActionKind::BuffAllies, Element::Physical,
                    EnemyStat::Attack, 0, 1.0f, 2 }
            }
        },
        {
            "Swordsman",
            "res/draw/Character/Enemies list/Swordsman.png",
            Element::Ice, 1, 1,
            {
                { "Cross Slash", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.85f, 3 },
                { "Parry", EnemyActionKind::BuffSelf, Element::Physical,
                    EnemyStat::Evasion, 0, 1.0f, 2 }
            }
        },
        {
            "Lancer",
            "res/draw/Character/Enemies list/Lancer.png",
            Element::Fire, 0, 1,
            {
                { "Piercing Thrust", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.90f, 3 },
                { "Brace", EnemyActionKind::BuffSelf, Element::Physical,
                    EnemyStat::Defense, 0, 1.0f, 2 }
            }
        },
        {
            "Knight",
            "res/draw/Character/Enemies list/Knight.png",
            Element::Ice, 3, 0,
            {
                { "Shield Wall", EnemyActionKind::BuffAllies, Element::Physical,
                    EnemyStat::Defense, 0, 1.0f, 3 },
                { "Heavy Blow", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.80f, 3 }
            }
        },
        {
            "Knight Templar",
            "res/draw/Character/Enemies list/Knight Templar.png",
            Element::Fire, 4, 1,
            {
                { "Holy Strike", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.85f, 3 },
                { "Guard All", EnemyActionKind::BuffAllies, Element::Physical,
                    EnemyStat::Defense, 0, 1.0f, 3 }
            }
        },
        {
            "Orc",
            "res/draw/Character/Enemies list/Orc.png",
            Element::Fire, 2, 1,
            {
                { "Savage Blow", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.85f, 3 },
                { "War Cry", EnemyActionKind::BuffAllies, Element::Physical,
                    EnemyStat::Attack, 0, 1.0f, 2 }
            }
        },
        {
            "Elite Orc",
            "res/draw/Character/Enemies list/Elite Orc.png",
            Element::Ice, 4, 2,
            {
                { "Rampage", EnemyActionKind::DamageAll, Element::Physical,
                    EnemyStat::None, 1, 0.80f, 3 },
                { "Bloodlust", EnemyActionKind::HealSelf, Element::Physical,
                    EnemyStat::None, 8, 1.0f, 2 }
            }
        },
        {
            "Armored Orc",
            "res/draw/Character/Enemies list/Armored Orc.png",
            Element::Fire, 3, 1,
            {
                { "Shield Slam", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.85f, 3 },
                { "Roar", EnemyActionKind::DebuffParty, Element::Physical,
                    EnemyStat::Attack, 0, 1.0f, 2 }
            }
        },
        {
            "Armored Axeman",
            "res/draw/Character/Enemies list/Armored Axeman.png",
            Element::Ice, 3, 2,
            {
                { "Axe Cleave", EnemyActionKind::DamageAll, Element::Physical,
                    EnemyStat::None, 1, 0.80f, 3 },
                { "Battle Cry", EnemyActionKind::BuffAllies, Element::Physical,
                    EnemyStat::Attack, 0, 1.0f, 2 }
            }
        },
        {
            "Orc Rider",
            "res/draw/Character/Enemies list/Orc rider.png",
            Element::Ice, 2, 2,
            {
                { "Charge", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.80f, 3 },
                { "Trample", EnemyActionKind::DamageAll, Element::Physical,
                    EnemyStat::None, 1, 0.80f, 3 }
            }
        },
        {
            "Werewolf",
            "res/draw/Character/Enemies list/Werewolf.png",
            Element::Fire, 1, 2,
            {
                { "Rend", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.90f, 3 },
                { "Howl", EnemyActionKind::BuffAllies, Element::Physical,
                    EnemyStat::Attack, 0, 1.0f, 2 }
            }
        },
        {
            "Werebear",
            "res/draw/Character/Enemies list/Werebear.png",
            Element::Fire, 4, 1,
            {
                { "Maul", EnemyActionKind::Damage, Element::Physical,
                    EnemyStat::None, 2, 0.80f, 3 },
                { "Hibernate", EnemyActionKind::HealSelf, Element::Physical,
                    EnemyStat::None, 10, 1.0f, 2 }
            }
        },
        {
            "Wizard",
            "res/draw/Character/Enemies list/Wizard.png",
            Element::Physical, 0, 2,
            {
                { "Agi", EnemyActionKind::Damage, Element::Fire,
                    EnemyStat::None, 2, 0.90f, 3 },
                { "Bufu", EnemyActionKind::Damage, Element::Ice,
                    EnemyStat::None, 2, 0.90f, 3 }
            }
        },
        {
            "Priest",
            "res/draw/Character/Enemies list/Priest.png",
            Element::Physical, 0, 1,
            {
                { "Dia", EnemyActionKind::HealAlly, Element::Physical,
                    EnemyStat::None, 6, 1.0f, 2 },
                { "Media", EnemyActionKind::HealAllies, Element::Physical,
                    EnemyStat::None, 4, 1.0f, 3 }
            }
        },
        {
            "Necromancer",
            "res/draw/Character/Enemies list/Necromancer.png",
            Element::Fire, 1, 2,
            {
                { "Drain", EnemyActionKind::Drain, Element::Physical,
                    EnemyStat::None, 2, 0.90f, 3 },
                { "Curse", EnemyActionKind::DebuffParty, Element::Physical,
                    EnemyStat::Attack, 0, 1.0f, 2 }
            }
        },
        {
            "Demon",
            "res/draw/Character/Enemies list/Demon_A.png",
            Element::Ice, 3, 2,
            {
                { "Dark Flame", EnemyActionKind::Damage, Element::Fire,
                    EnemyStat::None, 2, 0.85f, 3 },
                { "Fear", EnemyActionKind::DebuffParty, Element::Physical,
                    EnemyStat::Accuracy, 0, 1.0f, 3 }
            }
        },
        {
            "Blood Monster",
            "res/draw/Character/Enemies list/Blood Monster_A.png",
            Element::Fire, 2, 2,
            {
                { "Blood Drain", EnemyActionKind::Drain, Element::Physical,
                    EnemyStat::None, 2, 0.90f, 3 },
                { "Hemorrhage", EnemyActionKind::DamageAll, Element::Physical,
                    EnemyStat::None, 1, 0.85f, 3 }
            }
        }
    };
}

const EnemyDef& GetEnemyDef(EnemyKind kind)
{
    const int index = static_cast<int>(kind);

    if (index < 0 || index >= EnemyKindCount)
        return enemyDefs[0];

    return enemyDefs[index];
}

const char* EnemyKindName(EnemyKind kind)
{
    return GetEnemyDef(kind).name;
}

EnemyKind RandomEnemyKindForFloor(int floorNumber, bool elite)
{
    if (elite)
    {
        static const EnemyKind bosses[] = {
            EnemyKind::Demon,
            EnemyKind::Werebear,
            EnemyKind::KnightTemplar,
            EnemyKind::BloodMonster,
            EnemyKind::Necromancer
        };

        return bosses[GetRandomValue(0, 4)];
    }

    int maxKind = 4 + (floorNumber / 3) * 4;

    if (maxKind > EnemyKindCount - 1)
        maxKind = EnemyKindCount - 1;

    return static_cast<EnemyKind>(GetRandomValue(0, maxKind));
}

Enemy::Enemy(int startGridX, int startGridY, int maxHp, EnemyKind kind)
    : gridX(startGridX)
    , gridY(startGridY)
    , x(0)
    , y(0)
    , moveTimer(0.0f)
    , radius(15)
    , hp(maxHp)
    , maxHp(maxHp)
    , active(true)
    , kind(kind)
    , weakness(GetEnemyDef(kind).weakness)
{
    x = gridX * cellSize + cellSize / 2.0f;
    y = gridY * cellSize + cellSize / 2.0f;
}

void Enemy::Update(int targetGridX, int targetGridY, const GameMap& map)
{
    if (!active) return;

    moveTimer -= GetFrameTime();

    if (moveTimer <= 0.0f)
    {
        const int dx = targetGridX - gridX;
        const int dy = targetGridY - gridY;

        auto tryStep = [&](int nextX, int nextY) -> bool
        {
            if (!map.IsInside(nextX, nextY)) return false;
            if (map.IsWall(nextX, nextY)) return false;

            gridX = nextX;
            gridY = nextY;
            return true;
        };

        if (std::abs(dx) >= std::abs(dy) && dx != 0)
        {
            if (!tryStep(gridX + ((dx > 0) ? 1 : -1), gridY) && dy != 0)
                tryStep(gridX, gridY + ((dy > 0) ? 1 : -1));
        }
        else if (dy != 0)
        {
            if (!tryStep(gridX, gridY + ((dy > 0) ? 1 : -1)) && dx != 0)
                tryStep(gridX + ((dx > 0) ? 1 : -1), gridY);
        }

        moveTimer = moveDelay;
    }

    const float targetX = gridX * cellSize + cellSize / 2.0f;
    const float targetY = gridY * cellSize + cellSize / 2.0f;

    const float t = 1.0f - std::exp(-12.0f * GetFrameTime());
    x += (targetX - x) * t;
    y += (targetY - y) * t;
}

void Enemy::Draw() const
{
    if (!active) return;

    DrawCircle(x, y, radius, Theme::Burgundy);
    DrawCircle(x, y, radius - 4, Theme::Parchment);
    DrawCircle(x, y, 3, Theme::Burgundy);
}

int Enemy::GetGridX() const
{
    return gridX;
}

int Enemy::GetGridY() const
{
    return gridY;
}

float Enemy::GetPixelX() const
{
    return x;
}

float Enemy::GetPixelY() const
{
    return y;
}

int Enemy::GetHp() const
{
    return hp;
}

int Enemy::GetMaxHp() const
{
    return maxHp;
}

bool Enemy::IsAlive() const
{
    return hp > 0;
}

bool Enemy::IsActive() const
{
    return active;
}

void Enemy::SetActive(bool value)
{
    active = value;
}

void Enemy::TakeDamage(int amount)
{
    hp -= amount;

    if (hp <= 0)
    {
        hp = 0;
        active = false;
    }
}

void Enemy::Heal(int amount)
{
    if (hp <= 0) return;

    hp += amount;

    if (hp > maxHp)
        hp = maxHp;
}

void Enemy::Reset(int startGridX, int startGridY, int newMaxHp)
{
    gridX = startGridX;
    gridY = startGridY;
    x = gridX * cellSize + cellSize / 2.0f;
    y = gridY * cellSize + cellSize / 2.0f;
    moveTimer = 0.0f;
    hp = newMaxHp;
    maxHp = newMaxHp;
    active = true;
    weakness = GetEnemyDef(kind).weakness;
}

void Enemy::SetKind(EnemyKind newKind)
{
    kind = newKind;
    weakness = GetEnemyDef(kind).weakness;
}

EnemyKind Enemy::GetKind() const
{
    return kind;
}

Element Enemy::GetWeakness() const
{
    return weakness;
}

