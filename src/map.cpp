#include "map.h"
#include "theme.h"

#include <raylib.h>
#include <cstdlib>

void GameMap::CarveRoom(const Room& room)
{
    for (int y = room.y; y < room.y + room.h; y++)
    {
        for (int x = room.x; x < room.x + room.w; x++)
            tiles[y][x] = Tile::Floor;
    }
}

void GameMap::CarveCorridor(int fromX, int fromY, int toX, int toY)
{
    int x = fromX;
    int y = fromY;

    const bool horizontalFirst = (GetRandomValue(0, 1) == 0);

    if (horizontalFirst)
    {
        while (x != toX)
        {
            tiles[y][x] = Tile::Floor;
            x += (toX > x) ? 1 : -1;
        }

        while (y != toY)
        {
            tiles[y][x] = Tile::Floor;
            y += (toY > y) ? 1 : -1;
        }
    }
    else
    {
        while (y != toY)
        {
            tiles[y][x] = Tile::Floor;
            y += (toY > y) ? 1 : -1;
        }

        while (x != toX)
        {
            tiles[y][x] = Tile::Floor;
            x += (toX > x) ? 1 : -1;
        }
    }

    tiles[y][x] = Tile::Floor;
}

void GameMap::Generate(int& outSpawnX, int& outSpawnY, bool bossFloor)
{
    for (int y = 0; y < Rows; y++)
    {
        for (int x = 0; x < Cols; x++)
            tiles[y][x] = Tile::Wall;
    }

    pickups.clear();
    bossLayout = false;

    if (bossFloor)
    {
        GenerateBossLayout(outSpawnX, outSpawnY);
        return;
    }

    std::vector<Room> rooms;
    const int targetRooms = GetRandomValue(6, 9);

    for (int attempt = 0; attempt < 300 && static_cast<int>(rooms.size()) < targetRooms; attempt++)
    {
        Room room;

        room.w = GetRandomValue(6, 11);
        room.h = GetRandomValue(4, 7);
        room.x = GetRandomValue(1, Cols - room.w - 2);
        room.y = GetRandomValue(3, Rows - room.h - 3);

        bool overlaps = false;

        for (const Room& other : rooms)
        {
            if (room.x - 2 <= other.x + other.w && room.x + room.w + 2 >= other.x &&
                room.y - 2 <= other.y + other.h && room.y + room.h + 2 >= other.y)
            {
                overlaps = true;
                break;
            }
        }

        if (overlaps) continue;

        rooms.push_back(room);
        CarveRoom(room);
    }

    if (rooms.size() < 2)
    {
        rooms.clear();

        Room first = { 2, 3, 12, 7 };
        Room second = { Cols - 14, Rows - 10, 12, 7 };

        rooms.push_back(first);
        rooms.push_back(second);

        CarveRoom(first);
        CarveRoom(second);
        CarveCorridor(first.x + first.w / 2, first.y + first.h / 2,
            second.x + second.w / 2, second.y + second.h / 2);
    }
    else
    {
        for (int i = 1; i < static_cast<int>(rooms.size()); i++)
        {
            const Room& previous = rooms[i - 1];
            const Room& current = rooms[i];

            CarveCorridor(previous.x + previous.w / 2, previous.y + previous.h / 2,
                current.x + current.w / 2, current.y + current.h / 2);
        }
    }

    const Room& startRoom = rooms[0];
    spawnX = startRoom.x + startRoom.w / 2;
    spawnY = startRoom.y + startRoom.h / 2;

    int bestRoom = 1;
    int bestDistance = -1;

    for (int i = 1; i < static_cast<int>(rooms.size()); i++)
    {
        const Room& room = rooms[i];
        const int centerX = room.x + room.w / 2;
        const int centerY = room.y + room.h / 2;
        const int distance = std::abs(centerX - spawnX) + std::abs(centerY - spawnY);

        if (distance > bestDistance)
        {
            bestDistance = distance;
            bestRoom = i;
        }
    }

    const Room& exitRoom = rooms[bestRoom];
    exitX = exitRoom.x + exitRoom.w / 2;
    exitY = exitRoom.y + exitRoom.h / 2;

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            const int x = spawnX + dx;
            const int y = spawnY + dy;

            if (IsInside(x, y))
                tiles[y][x] = Tile::Floor;
        }
    }

    const int pickupCount = GetRandomValue(3, 5);

    for (int i = 0; i < pickupCount; i++)
    {
        for (int attempt = 0; attempt < 300; attempt++)
        {
            const int x = GetRandomValue(1, Cols - 2);
            const int y = GetRandomValue(1, Rows - 2);

            if (tiles[y][x] == Tile::Wall) continue;
            if (x == spawnX && y == spawnY) continue;
            if (x == exitX && y == exitY) continue;
            if (std::abs(x - spawnX) + std::abs(y - spawnY) < 3) continue;
            if (HasPickupAt(x, y)) continue;

            const ItemType type = (GetRandomValue(0, 9) < 7)
                ? ItemType::Herb : ItemType::Potion;

            pickups.push_back(Pickup{ x, y, type });
            break;
        }
    }

    outSpawnX = spawnX;
    outSpawnY = spawnY;
}

void GameMap::GenerateBossLayout(int& outSpawnX, int& outSpawnY)
{
    CarveRoom({ 14, 4, 20, 20 });
    CarveRoom({ 2, 11, 8, 6 });
    CarveRoom({ 36, 8, 10, 11 });
    CarveRoom({ 35, 1, 8, 5 });

    CarveCorridor(9, 13, 14, 13);
    CarveCorridor(33, 13, 36, 13);
    CarveCorridor(38, 5, 38, 8);

    spawnX = 41;
    spawnY = 13;
    exitX = 5;
    exitY = 13;
    bossLayout = true;

    outSpawnX = spawnX;
    outSpawnY = spawnY;
}

bool GameMap::IsBossLayout() const
{
    return bossLayout;
}

int GameMap::GetExitX() const
{
    return exitX;
}

int GameMap::GetExitY() const
{
    return exitY;
}

bool GameMap::IsInside(int x, int y) const
{
    return x >= 0 && x < Cols && y >= 0 && y < Rows;
}

bool GameMap::IsWall(int x, int y) const
{
    if (!IsInside(x, y)) return true;

    return tiles[y][x] == Tile::Wall;
}

bool GameMap::HasPickupAt(int x, int y) const
{
    for (const Pickup& pickup : pickups)
    {
        if (pickup.x == x && pickup.y == y) return true;
    }

    return false;
}

bool GameMap::TryCollect(int x, int y, Inventory& inventory)
{
    for (int i = 0; i < static_cast<int>(pickups.size()); i++)
    {
        if (pickups[i].x != x || pickups[i].y != y) continue;

        inventory.Add(pickups[i].type, 1);
        pickups.erase(pickups.begin() + i);
        return true;
    }

    return false;
}

bool GameMap::FindFreeCell(int& x, int& y, int avoidX, int avoidY, int minDistance) const
{
    for (int attempt = 0; attempt < 600; attempt++)
    {
        const int candidateX = GetRandomValue(0, Cols - 1);
        const int candidateY = GetRandomValue(0, Rows - 1);

        if (IsWall(candidateX, candidateY)) continue;
        if (HasPickupAt(candidateX, candidateY)) continue;
        if (candidateX == avoidX && candidateY == avoidY) continue;
        if (candidateX == exitX && candidateY == exitY) continue;
        if (candidateX == spawnX && candidateY == spawnY) continue;

        if (std::abs(candidateX - avoidX) + std::abs(candidateY - avoidY) < minDistance)
            continue;

        x = candidateX;
        y = candidateY;
        return true;
    }

    for (int cy = 0; cy < Rows; cy++)
    {
        for (int cx = 0; cx < Cols; cx++)
        {
            if (IsWall(cx, cy)) continue;
            if (HasPickupAt(cx, cy)) continue;
            if (cx == avoidX && cy == avoidY) continue;
            if (cx == exitX && cy == exitY) continue;
            if (cx == spawnX && cy == spawnY) continue;

            x = cx;
            y = cy;
            return true;
        }
    }

    x = avoidX;
    y = avoidY;
    return false;
}

void GameMap::Draw() const
{
    for (int y = 0; y < Rows; y++)
    {
        for (int x = 0; x < Cols; x++)
        {
            if (tiles[y][x] != Tile::Wall) continue;

            const int px = x * TileSize;
            const int py = y * TileSize;

            DrawRectangle(px, py, TileSize, TileSize, Theme::Wall);

            if (IsInside(x, y - 1) && !IsWall(x, y - 1))
                DrawRectangle(px, py + TileSize - 3, TileSize, 3, Theme::Muted);

            if (IsInside(x, y + 1) && !IsWall(x, y + 1))
                DrawRectangle(px, py, TileSize, 3, Theme::Muted);

            if (IsInside(x - 1, y) && !IsWall(x - 1, y))
                DrawRectangle(px + TileSize - 3, py, 3, TileSize, Theme::Muted);

            if (IsInside(x + 1, y) && !IsWall(x + 1, y))
                DrawRectangle(px, py, 3, TileSize, Theme::Muted);
        }
    }

    const int exitPx = exitX * TileSize;
    const int exitPy = exitY * TileSize;

    DrawRectangle(exitPx + 6, exitPy + 6, TileSize - 12, TileSize - 12, Theme::Burgundy);
    DrawRectangleLinesEx(
        { static_cast<float>(exitPx) + 6.0f, static_cast<float>(exitPy) + 6.0f,
          static_cast<float>(TileSize) - 12.0f, static_cast<float>(TileSize) - 12.0f },
        2.0f, Theme::Gold);

    for (int step = 0; step < 3; step++)
    {
        DrawRectangle(exitPx + 11 + step * 3, exitPy + 27 - step * 6,
            18 - step * 4, 3, Theme::Gold);
    }

    for (const Pickup& pickup : pickups)
    {
        const float cx = pickup.x * TileSize + TileSize / 2.0f;
        const float cy = pickup.y * TileSize + TileSize / 2.0f;
        const Color color = (pickup.type == ItemType::Herb)
            ? Theme::Forest : Theme::Gold;

        DrawPoly({ cx, cy }, 4, 10.0f, 45.0f, color);
        DrawPolyLinesEx({ cx, cy }, 4, 10.0f, 45.0f, 2.0f, Theme::Ink);
    }
}

