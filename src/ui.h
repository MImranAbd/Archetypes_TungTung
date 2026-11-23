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

enum class PausePage
{
    Root,
    Items,
    Equip,
    Skills,
    Quests,
    Member,
    Systems
};

struct PauseMenuState
{
    PausePage page = PausePage::Root;
    int menuIndex = 0;
    int memberIndex = 0;
    int itemIndex = 0;
    int itemCategory = 0;
    bool itemTargeting = false;
    int targetIndex = 0;
    int equipStep = 0;
    int equipSlot = 0;
    int equipChoice = 0;
    std::string message;
};

class UI
{
public:
    UI();

    void Update(float deltaTime);
    void StartSlash(int cellX, int cellY);

    int DrawMainMenu(int selectedIndex, bool showHelp) const;

    void DrawOverworld(const Ball& ball, const Enemy& enemy, const Party& party,
        const GameMap& map, bool pendingBattle, int dungeonFloor,
        bool showHud, const Camera2D& camera) const;

    void DrawPauseMenu(const Party& party, const Inventory& inventory,
        const PauseMenuState& menu, int dungeonFloor) const;

    void DrawBattle(Battle& battle);

    static constexpr float slashDuration = 0.25f;

private:
    void DrawAttackIndicator(const Ball& ball, const Enemy& enemy, const GameMap& map) const;
    void DrawSlash() const;

    float slashTimer;
    int slashCellX;
    int slashCellY;
};

