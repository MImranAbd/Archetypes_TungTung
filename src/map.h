#pragma once

#include <vector>
#include "inventory.h"

class GameMap
{
public:
    static constexpr int Cols = 48;
    static constexpr int Rows = 27;
    static constexpr int TileSize = 40;

    void Generate(int& spawnX, int& spawnY, bool bossFloor = false);
    bool IsBossLayout() const;

    int GetExitX() const;
    int GetExitY() const;

    bool IsInside(int x, int y) const;
    bool IsWall(int x, int y) const;
    bool TryCollect(int x, int y, Inventory& inventory);
    bool FindFreeCell(int& x, int& y, int avoidX, int avoidY, int minDistance) const;

    void Draw() const;

private:
    enum class Tile { Floor, Wall };

    struct Room
    {
        int x;
        int y;
        int w;
        int h;
    };

    struct Pickup
    {
        int x;
        int y;
        ItemType type;
    };

    void CarveRoom(const Room& room);
    void CarveCorridor(int fromX, int fromY, int toX, int toY);
    void GenerateBossLayout(int& outSpawnX, int& outSpawnY);
    bool HasPickupAt(int x, int y) const;

    Tile tiles[Rows][Cols];
    std::vector<Pickup> pickups;
    int spawnX;
    int spawnY;
    int exitX;
    int exitY;
    bool bossLayout = false;
};
