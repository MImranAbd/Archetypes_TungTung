#pragma once

class GameMap;

enum class Element
{
    Physical,
    Ice,
    Fire
};

inline const char* ElementName(Element element)
{
    switch (element)
    {
        case Element::Physical: return "PHYS";
        case Element::Ice:      return "ICE";
        case Element::Fire:     return "FIRE";
    }

    return "???";
}

class Enemy
{
public:
    Enemy(int startGridX, int startGridY, int maxHp = 6);
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
    void Reset(int startGridX, int startGridY, int maxHp = 6);
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
    Element weakness;
};
