#include <raylib.h>
#include <ctime>
#include <memory>
#include <string>
#include "ball.h"
#include "enemies.h"
#include "battle.h"
#include "party.h"
#include "inventory.h"
#include "equipment.h"
#include "map.h"
#include "ui.h"

enum class AppState
{
    MainMenu,
    Playing
};

int main()
{
    constexpr int screenWidth = 1920;
    constexpr int screenHeight = 1080;

    SetRandomSeed(static_cast<unsigned int>(time(nullptr)));

    Ball ball;
    Party party;
    Inventory inventory;
    GameMap map;

    int spawnX = 0;
    int spawnY = 0;
    map.Generate(spawnX, spawnY);
    ball.SetGridPosition(spawnX, spawnY);

    int enemyX = spawnX;
    int enemyY = spawnY;
    map.FindFreeCell(enemyX, enemyY, spawnX, spawnY, 8);

    Enemy enemy(enemyX, enemyY, 6);

    inventory.Add(ItemType::Herb, 1);
    inventory.AddEquipment(EquipmentId::IronSword, 1);
    inventory.AddEquipment(EquipmentId::LeatherArmor, 1);

    std::unique_ptr<Battle> battle;

    InitWindow(screenWidth, screenHeight, "ARCHETYPES");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    UI ui;

    AppState state = AppState::MainMenu;
    int menuSelection = 0;
    bool helpOpen = false;
    bool quitRequested = false;

    bool pendingBattle = false;
    bool pendingAmbush = false;
    float pendingBattleTimer = 0.0f;

    bool pauseOpen = false;
    int pauseTab = 0;
    int pauseMember = 0;
    int pauseItem = 0;
    std::string pauseMessage;

    int dungeonFloor = 1;

    auto loadDungeon = [&]()
    {
        int newSpawnX = 0;
        int newSpawnY = 0;
        map.Generate(newSpawnX, newSpawnY);
        ball.SetGridPosition(newSpawnX, newSpawnY);

        int newEnemyX = newSpawnX;
        int newEnemyY = newSpawnY;

        if (map.FindFreeCell(newEnemyX, newEnemyY, newSpawnX, newSpawnY, 8))
            enemy.Reset(newEnemyX, newEnemyY, 6);
        else
            enemy.SetActive(false);

        pendingBattle = false;
        pendingAmbush = false;
    };

    auto startNewGame = [&]()
    {
        party = Party();
        inventory = Inventory();
        inventory.Add(ItemType::Herb, 1);
        inventory.AddEquipment(EquipmentId::IronSword, 1);
        inventory.AddEquipment(EquipmentId::LeatherArmor, 1);

        dungeonFloor = 1;
        battle.reset();
        pendingBattle = false;
        pendingAmbush = false;
        pauseOpen = false;
        pauseTab = 0;
        pauseMember = 0;
        pauseItem = 0;
        pauseMessage.clear();

        loadDungeon();
        state = AppState::Playing;
    };

    auto usePauseItem = [&]()
    {
        const std::vector<ItemStack>& stacks = inventory.Stacks();

        if (stacks.empty())
        {
            pauseItem = 0;
            return;
        }

        if (pauseItem < 0 || pauseItem >= static_cast<int>(stacks.size()))
            pauseItem = 0;

        const ItemType type = stacks[pauseItem].type;

        int targetIndex = -1;

        for (int i = 0; i < static_cast<int>(party.Members().size()); i++)
        {
            const Character& member = party.Members()[i];

            if (member.hp <= 0 || member.hp >= member.maxHp) continue;

            if (targetIndex < 0 || member.hp < party.Members()[targetIndex].hp)
                targetIndex = i;
        }

        if (targetIndex < 0)
        {
            pauseMessage = "No one needs healing.";
            return;
        }

        Character& target = party.Members()[targetIndex];

        int healed = ItemHeal(type);
        const int missing = target.maxHp - target.hp;
        if (healed > missing) healed = missing;

        target.hp += healed;
        inventory.Consume(type);

        pauseMessage = std::string(ItemName(type)) + " restores "
            + std::to_string(healed) + " HP to " + target.name + ".";

        if (inventory.Stacks().empty())
            pauseItem = 0;
        else if (pauseItem >= static_cast<int>(inventory.Stacks().size()))
            pauseItem = static_cast<int>(inventory.Stacks().size()) - 1;
    };

    auto cycleEquipment = [&](EquipSlot slot)
    {
        if (party.CycleEquipment(pauseMember, slot, inventory))
        {
            const Character& member = party.Members()[pauseMember];
            EquipmentId id = EquipmentId::None;

            if (slot == EquipSlot::Weapon) id = member.weapon;
            else if (slot == EquipSlot::Armor) id = member.armor;
            else id = member.accessory;

            pauseMessage = std::string(EquipSlotName(slot)) + ": "
                + GetEquipment(id).name;
        }
        else
        {
            pauseMessage = "Nothing available for that slot.";
        }
    };

    while (!quitRequested && !WindowShouldClose())
    {
        ui.Update(GetFrameTime());

        if (state == AppState::MainMenu) //tung
        {
            if (helpOpen)
            {
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    IsKeyPressed(KEY_J) || IsKeyPressed(KEY_ESCAPE) ||
                    IsKeyPressed(KEY_K))
                {
                    helpOpen = false;
                }
            }
            else
            {
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                    menuSelection = (menuSelection + 2) % 3;

                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                    menuSelection = (menuSelection + 1) % 3;

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    IsKeyPressed(KEY_J))
                {
                    if (menuSelection == 0) startNewGame();
                    else if (menuSelection == 1) helpOpen = true;
                    else quitRequested = true;
                }
            }

            BeginDrawing();
            const int clicked = ui.DrawMainMenu(menuSelection, helpOpen);
            EndDrawing();

            if (!helpOpen && clicked >= 0)
            {
                if (clicked == 0) startNewGame();
                else if (clicked == 1) helpOpen = true;
                else quitRequested = true;
            }

            continue;
        }

        if (!battle)
        {
            if (pauseOpen)
            {
                if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE) ||
                    IsKeyPressed(KEY_I) || IsKeyPressed(KEY_K))
                {
                    pauseOpen = false;
                }
                else
                {
                    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT) ||
                        IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
                    {
                        pauseTab = 1 - pauseTab;
                        pauseMessage.clear();
                    }

                    if (pauseTab == 0)
                    {
                        const int memberCount = static_cast<int>(party.Members().size());

                        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                            pauseMember = (pauseMember + memberCount - 1) % memberCount;

                        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                            pauseMember = (pauseMember + 1) % memberCount;

                        if (IsKeyPressed(KEY_ONE))
                            cycleEquipment(EquipSlot::Weapon);

                        if (IsKeyPressed(KEY_TWO))
                            cycleEquipment(EquipSlot::Armor);

                        if (IsKeyPressed(KEY_THREE))
                            cycleEquipment(EquipSlot::Accessory);
                    }
                    else
                    {
                        const int stackCount = static_cast<int>(inventory.Stacks().size());

                        if (stackCount > 0)
                        {
                            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                                pauseItem = (pauseItem + stackCount - 1) % stackCount;

                            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                                pauseItem = (pauseItem + 1) % stackCount;

                            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                                IsKeyPressed(KEY_J))
                                usePauseItem();
                        }
                        else
                        {
                            pauseItem = 0;
                        }
                    }
                }
            }
            else if (pendingBattle)
            {
                ball.UpdateVisual();
                pendingBattleTimer -= GetFrameTime();

                if (pendingBattleTimer <= 0.0f)
                {
                    pendingBattle = false;
                    battle.reset(new Battle(party, inventory, dungeonFloor, pendingAmbush));
                    pendingAmbush = false;
                }
            }
            else
            {
                if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE))
                {
                    pauseOpen = true;
                    pauseTab = 0;
                    pauseMessage.clear();
                }
                else if (IsKeyPressed(KEY_I))
                {
                    pauseOpen = true;
                    pauseTab = 1;
                    pauseItem = 0;
                    pauseMessage.clear();
                }
                else
                {
                    const int previousX = ball.GetGridX();
                    const int previousY = ball.GetGridY();

                    ball.Update(map,
                        enemy.IsActive() ? enemy.GetGridX() : -1,
                        enemy.IsActive() ? enemy.GetGridY() : -1);

                    const int stepX = (ball.GetGridX() > previousX) ? 1
                        : ((ball.GetGridX() < previousX) ? -1 : 0);
                    const int stepY = (ball.GetGridY() > previousY) ? 1
                        : ((ball.GetGridY() < previousY) ? -1 : 0);

                    int cellX = previousX;
                    int cellY = previousY;
                    bool reachedExit = (cellX == map.GetExitX() && cellY == map.GetExitY());

                    while (cellX != ball.GetGridX() || cellY != ball.GetGridY())
                    {
                        cellX += stepX;
                        cellY += stepY;

                        map.TryCollect(cellX, cellY, inventory);

                        if (cellX == map.GetExitX() && cellY == map.GetExitY())
                            reachedExit = true;
                    }

                    if (reachedExit)
                    {
                        dungeonFloor++;
                        loadDungeon();
                    }
                    else
                    {
                        int exitX = 0;
                        int exitY = 0;

                        if (ball.ConsumeExit(exitX, exitY))
                        {
                            dungeonFloor++;
                            loadDungeon();
                        }
                    }

                    enemy.Update(ball.GetGridX(), ball.GetGridY(), map);

                    if (enemy.IsActive() &&
                        enemy.GetGridX() == ball.GetGridX() &&
                        enemy.GetGridY() == ball.GetGridY())
                    {
                        pendingBattle = true;
                        pendingAmbush = true;
                        pendingBattleTimer = UI::slashDuration;
                    }

                    if (!pendingBattle &&
                        (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_J)))
                    {
                        const AttackPlan plan = MakeAttackPlan(ball, enemy, map);

                        ball.Dash(plan.dashCells, map);

                        if (plan.reach > 0)
                            ui.StartSlash(plan.targetX, plan.targetY);

                        if (plan.willHit)
                        {
                            pendingBattle = true;
                            pendingAmbush = false;
                            pendingBattleTimer = UI::slashDuration;
                        }
                    }
                }
            }
        }
        else
        {
            battle->Update();

            if (battle->IsFinished())
            {
                if (battle->WasEscaped())
                {
                    // Escaped: the encounter remains in the overworld.
                }
                else if (battle->PlayerWon())
                {
                    enemy.SetActive(false);

                    party.GainXp(battle->GetXpReward());
                    party.AddMoney(battle->GetMoneyReward());

                    if (battle->GetItemReward() > 0)
                        inventory.Add(ItemType::Potion, battle->GetItemReward());

                    if (battle->HasEquipmentReward())
                        inventory.AddEquipment(battle->GetEquipmentReward(), 1);
                }
                else
                {
                    party.RestoreAll();
                }

                battle.reset();

                if (enemy.IsActive() &&
                    enemy.GetGridX() == ball.GetGridX() &&
                    enemy.GetGridY() == ball.GetGridY())
                {
                    int enemyX = ball.GetGridX();
                    int enemyY = ball.GetGridY();

                    if (map.FindFreeCell(enemyX, enemyY, ball.GetGridX(), ball.GetGridY(), 8))
                        enemy.Reset(enemyX, enemyY, 6);
                    else
                        enemy.SetActive(false);
                }
            }
        }

        BeginDrawing();

        if (!battle)
        {
            ui.DrawOverworld(ball, enemy, party, map, pendingBattle, dungeonFloor);

            if (pauseOpen)
                ui.DrawPauseMenu(party, inventory, pauseTab, pauseMember, pauseItem, pauseMessage);
        }
        else
        {
            ui.DrawBattle(*battle);
        }

        EndDrawing();
    }

    CloseWindow();
}

