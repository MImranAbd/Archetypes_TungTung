#pragma once

#include <string>
#include <raylib.h>
#include "ball.h"
#include "enemies.h"
#include "battle.h"
#include "party.h"
#include "map.h"
#include "inventory.h"

struct AttackPlan
{
    int dashCells;
    int reach;
    int targetX;
    int targetY;
    bool willHit;
};

AttackPlan MakeAttackPlan(const Ball& ball, const Enemy& enemy, const GameMap& map);

class UI
{
public:
    UI();

    void Update(float deltaTime);
    void StartSlash(int cellX, int cellY);

    int DrawMainMenu(int selectedIndex, bool showHelp) const;

    void DrawOverworld(const Ball& ball, const Enemy& enemy, const Party& party,
        const GameMap& map, bool pendingBattle, int dungeonFloor) const;

    void DrawPauseMenu(const Party& party, const Inventory& inventory,
        int tab, int memberIndex, int itemIndex, const std::string& message) const;

    void DrawBattle(Battle& battle);

    static constexpr float slashDuration = 0.25f;

private:
    void DrawAttackIndicator(const Ball& ball, const Enemy& enemy, const GameMap& map) const;
    void DrawSlash() const;

    float slashTimer;
    int slashCellX;
    int slashCellY;
};

