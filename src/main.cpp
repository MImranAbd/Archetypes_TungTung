#include <raylib.h>
#include <cmath>
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

    InitAudioDevice();

    Music battleMusic = {};
    bool battleMusicActive = false;

    if (IsAudioDeviceReady())
    {
        battleMusic = LoadMusicStream("res/ost/BattleB2.mp3");
        battleMusic.looping = true;

        if (battleMusic.frameCount > 0)
            SetMusicVolume(battleMusic, 0.7f);
    }

    AppState state = AppState::MainMenu;
    int menuSelection = 0;
    bool helpOpen = false;
    bool quitRequested = false;

    bool pendingBattle = false;
    bool pendingAmbush = false;
    float pendingBattleTimer = 0.0f;

    bool pauseOpen = false;
    PauseMenuState pauseMenu;

    int dungeonFloor = 1;

    Camera2D overworldCamera = {};
    overworldCamera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    overworldCamera.zoom = 2.0f;
    bool snapCamera = true;

    auto updateOverworldCamera = [&](float deltaTime)
    {
        const float targetX = ball.GetPixelX();
        const float targetY = ball.GetPixelY();

        if (snapCamera)
        {
            overworldCamera.target = { targetX, targetY };
        }
        else
        {
            const float follow = 1.0f - std::exp(-8.0f * deltaTime);
            overworldCamera.target.x += (targetX - overworldCamera.target.x) * follow;
            overworldCamera.target.y += (targetY - overworldCamera.target.y) * follow;
        }

        const float halfWidth = screenWidth / (2.0f * overworldCamera.zoom);
        const float halfHeight = screenHeight / (2.0f * overworldCamera.zoom);
        const float worldWidth = GameMap::Cols * Ball::cellSize;
        const float worldHeight = GameMap::Rows * Ball::cellSize;

        if (worldWidth <= halfWidth * 2.0f)
            overworldCamera.target.x = worldWidth / 2.0f;
        else if (overworldCamera.target.x < halfWidth)
            overworldCamera.target.x = halfWidth;
        else if (overworldCamera.target.x > worldWidth - halfWidth)
            overworldCamera.target.x = worldWidth - halfWidth;

        if (worldHeight <= halfHeight * 2.0f)
            overworldCamera.target.y = worldHeight / 2.0f;
        else if (overworldCamera.target.y < halfHeight)
            overworldCamera.target.y = halfHeight;
        else if (overworldCamera.target.y > worldHeight - halfHeight)
            overworldCamera.target.y = worldHeight - halfHeight;
    };

    auto loadDungeon = [&]()
    {
        int newSpawnX = 0;
        int newSpawnY = 0;
        map.Generate(newSpawnX, newSpawnY);
        ball.SetGridPosition(newSpawnX, newSpawnY);
        snapCamera = true;

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

        if (battleMusicActive)
        {
            StopMusicStream(battleMusic);
            battleMusicActive = false;
        }

        pendingBattle = false;
        pendingAmbush = false;
        pauseOpen = false;
        pauseMenu = PauseMenuState();

        loadDungeon();
        state = AppState::Playing;
    };

    auto usePauseItem = [&](int targetIndex)
    {
        const ItemCategory category = static_cast<ItemCategory>(pauseMenu.itemCategory);
        std::vector<const ItemStack*> items = ItemStacksInCategory(inventory, category);

        if (items.empty())
        {
            pauseMenu.itemIndex = 0;
            pauseMenu.itemTargeting = false;
            return;
        }

        if (pauseMenu.itemIndex < 0 ||
            pauseMenu.itemIndex >= static_cast<int>(items.size()))
            pauseMenu.itemIndex = 0;

        const ItemType type = items[pauseMenu.itemIndex]->type;

        if (ItemCategoryOf(type) != ItemCategory::Healing)
        {
            pauseMenu.message = "That item cannot be used here.";
            return;
        }

        if (ItemTargetsAll(type))
        {
            int healedTotal = 0;
            int affected = 0;

            for (Character& member : party.Members())
            {
                if (member.hp <= 0) continue;

                const int missing = member.maxHp - member.hp;
                if (missing <= 0) continue;

                int healed = ItemHeal(type);
                if (healed > missing) healed = missing;

                member.hp += healed;
                healedTotal += healed;
                affected++;
            }

            if (affected == 0)
            {
                pauseMenu.message = "No one needs healing.";
                return;
            }

            inventory.Consume(type);
            pauseMenu.message = std::string(ItemName(type)) + " restores "
                + std::to_string(healedTotal) + " HP to the party.";
            pauseMenu.itemTargeting = false;
        }
        else
        {
            if (targetIndex < 0 ||
                targetIndex >= static_cast<int>(party.Members().size()))
                return;

            Character& target = party.Members()[targetIndex];

            if (target.hp <= 0)
            {
                pauseMenu.message = "Cannot heal a fallen ally.";
                return;
            }

            if (target.hp >= target.maxHp)
            {
                pauseMenu.message = "HP is already full.";
                return;
            }

            int healed = ItemHeal(type);
            const int missing = target.maxHp - target.hp;
            if (healed > missing) healed = missing;

            target.hp += healed;
            inventory.Consume(type);

            pauseMenu.message = std::string(ItemName(type)) + " restores "
                + std::to_string(healed) + " HP to " + target.name + ".";
            pauseMenu.itemTargeting = false;
        }

        std::vector<const ItemStack*> remaining =
            ItemStacksInCategory(inventory, category);

        if (remaining.empty())
            pauseMenu.itemIndex = 0;
        else if (pauseMenu.itemIndex >= static_cast<int>(remaining.size()))
            pauseMenu.itemIndex = static_cast<int>(remaining.size()) - 1;
    };

    auto applyEquipment = [&]()
    {
        if (pauseMenu.memberIndex < 0 ||
            pauseMenu.memberIndex >= static_cast<int>(party.Members().size()))
            return;

        const EquipSlot slot = static_cast<EquipSlot>(pauseMenu.equipSlot);
        std::vector<EquipmentId> options = EquipmentOptions(
            party.Members()[pauseMenu.memberIndex], slot, inventory);

        if (options.empty())
            return;

        if (pauseMenu.equipChoice < 0 ||
            pauseMenu.equipChoice >= static_cast<int>(options.size()))
            pauseMenu.equipChoice = 0;

        const EquipmentId id = options[pauseMenu.equipChoice];

        if (party.EquipItem(pauseMenu.memberIndex, slot, id, inventory))
        {
            pauseMenu.message = std::string(EquipSlotName(slot)) + ": "
                + GetEquipment(id).name;
        }
        else
        {
            pauseMenu.message = "Cannot equip that.";
        }

        pauseMenu.equipStep = 1;
    };

    auto centerEquipmentChoice = [&]()
    {
        const EquipSlot slot = static_cast<EquipSlot>(pauseMenu.equipSlot);
        const Character& member = party.Members()[pauseMenu.memberIndex];

        EquipmentId current = EquipmentId::None;
        if (slot == EquipSlot::Weapon) current = member.weapon;
        else if (slot == EquipSlot::Armor) current = member.armor;
        else current = member.accessory;

        std::vector<EquipmentId> options = EquipmentOptions(member, slot, inventory);

        pauseMenu.equipChoice = 0;

        for (int i = 0; i < static_cast<int>(options.size()); i++)
        {
            if (options[i] == current)
            {
                pauseMenu.equipChoice = i;
                break;
            }
        }
    };

    while (!quitRequested && !WindowShouldClose())
    {
        ui.Update(GetFrameTime()); //tunng

        if (battleMusicActive)
            UpdateMusicStream(battleMusic);

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
                if (pauseMenu.page == PausePage::Root)
                {
                    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                        pauseMenu.menuIndex = (pauseMenu.menuIndex + 5) % 6;

                    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                        pauseMenu.menuIndex = (pauseMenu.menuIndex + 1) % 6;

                    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                        IsKeyPressed(KEY_J))
                    {
                        pauseMenu.page = static_cast<PausePage>(
                            static_cast<int>(PausePage::Items) + pauseMenu.menuIndex);
                        pauseMenu.message.clear();
                        pauseMenu.itemTargeting = false;
                        pauseMenu.equipStep = 0;
                    }

                    if (IsKeyPressed(KEY_K) || IsKeyPressed(KEY_ESCAPE) ||
                        IsKeyPressed(KEY_P) || IsKeyPressed(KEY_I) ||
                        IsKeyPressed(KEY_TAB))
                        pauseOpen = false;
                }
                else
                {
                    if (IsKeyPressed(KEY_K) || IsKeyPressed(KEY_ESCAPE))
                    {
                        if (pauseMenu.page == PausePage::Items &&
                            pauseMenu.itemTargeting)
                        {
                            pauseMenu.itemTargeting = false;
                            pauseMenu.message.clear();
                        }
                        else if (pauseMenu.page == PausePage::Equip &&
                            pauseMenu.equipStep > 0)
                        {
                            pauseMenu.equipStep--;
                            pauseMenu.message.clear();
                        }
                        else
                        {
                            pauseMenu.page = PausePage::Root;
                            pauseMenu.message.clear();
                        }
                    }
                    else if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_TAB))
                    {
                        pauseOpen = false;
                    }
                    else if (pauseMenu.page == PausePage::Items)
                    {
                        if (pauseMenu.itemTargeting)
                        {
                            const int memberCount =
                                static_cast<int>(party.Members().size());

                            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                            {
                                for (int i = 0; i < memberCount; i++)
                                {
                                    pauseMenu.targetIndex = (pauseMenu.targetIndex +
                                        memberCount - 1) % memberCount;

                                    if (party.Members()[pauseMenu.targetIndex].hp > 0)
                                        break;
                                }
                            }

                            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                            {
                                for (int i = 0; i < memberCount; i++)
                                {
                                    pauseMenu.targetIndex = (pauseMenu.targetIndex + 1) %
                                        memberCount;

                                    if (party.Members()[pauseMenu.targetIndex].hp > 0)
                                        break;
                                }
                            }

                            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                                IsKeyPressed(KEY_J))
                                usePauseItem(pauseMenu.targetIndex);
                        }
                        else
                        {
                            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
                            {
                                pauseMenu.itemCategory =
                                    (pauseMenu.itemCategory + 3) % 4;
                                pauseMenu.itemIndex = 0;
                                pauseMenu.message.clear();
                            }

                            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
                            {
                                pauseMenu.itemCategory =
                                    (pauseMenu.itemCategory + 1) % 4;
                                pauseMenu.itemIndex = 0;
                                pauseMenu.message.clear();
                            }

                            const ItemCategory category = static_cast<ItemCategory>(
                                pauseMenu.itemCategory);
                            std::vector<const ItemStack*> items =
                                ItemStacksInCategory(inventory, category);
                            const int itemCount = static_cast<int>(items.size());

                            if (itemCount > 0)
                            {
                                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                                    pauseMenu.itemIndex = (pauseMenu.itemIndex +
                                        itemCount - 1) % itemCount;

                                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                                    pauseMenu.itemIndex = (pauseMenu.itemIndex + 1) %
                                        itemCount;
                            }
                            else
                            {
                                pauseMenu.itemIndex = 0;
                            }

                            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                                IsKeyPressed(KEY_J))
                            {
                                if (itemCount == 0)
                                {
                                    pauseMenu.message = "No items in this category.";
                                }
                                else if (category != ItemCategory::Healing)
                                {
                                    pauseMenu.message = "Only healing items can be used here.";
                                }
                                else
                                {
                                    pauseMenu.itemTargeting = true;
                                    pauseMenu.targetIndex = pauseMenu.memberIndex;

                                    if (party.Members()[pauseMenu.targetIndex].hp <= 0)
                                    {
                                        const int memberCount = static_cast<int>(
                                            party.Members().size());
                                        for (int i = 0; i < memberCount; i++)
                                        {
                                            if (party.Members()[i].hp > 0)
                                            {
                                                pauseMenu.targetIndex = i;
                                                break;
                                            }
                                        }
                                    }

                                    pauseMenu.message = "Choose an ally.";
                                }
                            }
                        }
                    }
                    else if (pauseMenu.page == PausePage::Equip)
                    {
                        const int memberCount = static_cast<int>(party.Members().size());

                        if (pauseMenu.equipStep == 0)
                        {
                            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                                pauseMenu.memberIndex = (pauseMenu.memberIndex +
                                    memberCount - 1) % memberCount;

                            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                                pauseMenu.memberIndex = (pauseMenu.memberIndex + 1) %
                                    memberCount;

                            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                                IsKeyPressed(KEY_J))
                            {
                                pauseMenu.equipStep = 1;
                                pauseMenu.equipSlot = 0;
                                pauseMenu.message.clear();
                            }
                        }
                        else if (pauseMenu.equipStep == 1)
                        {
                            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                                pauseMenu.equipSlot = (pauseMenu.equipSlot + 2) % 3;

                            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                                pauseMenu.equipSlot = (pauseMenu.equipSlot + 1) % 3;

                            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                                IsKeyPressed(KEY_J))
                            {
                                pauseMenu.equipStep = 2;
                                centerEquipmentChoice();
                                pauseMenu.message.clear();
                            }
                        }
                        else
                        {
                            const EquipSlot slot = static_cast<EquipSlot>(
                                pauseMenu.equipSlot);
                            std::vector<EquipmentId> options = EquipmentOptions(
                                party.Members()[pauseMenu.memberIndex], slot,
                                inventory);
                            const int optionCount = static_cast<int>(options.size());

                            if (optionCount > 0)
                            {
                                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                                    pauseMenu.equipChoice = (pauseMenu.equipChoice +
                                        optionCount - 1) % optionCount;

                                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                                    pauseMenu.equipChoice = (pauseMenu.equipChoice + 1) %
                                        optionCount;
                            }
                            else
                            {
                                pauseMenu.equipChoice = 0;
                            }

                            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                                IsKeyPressed(KEY_J))
                                applyEquipment();
                        }
                    }
                    else if (pauseMenu.page == PausePage::Member ||
                        pauseMenu.page == PausePage::Skills)
                    {
                        const int memberCount = static_cast<int>(party.Members().size());

                        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                            pauseMenu.memberIndex = (pauseMenu.memberIndex +
                                memberCount - 1) % memberCount;

                        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                            pauseMenu.memberIndex = (pauseMenu.memberIndex + 1) %
                                memberCount;
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

                    if (battleMusic.frameCount > 0)
                    {
                        SeekMusicStream(battleMusic, 0.0f);
                        PlayMusicStream(battleMusic);
                        battleMusicActive = true;
                    }
                }
            }
            else
            {
                if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE) ||
                    IsKeyPressed(KEY_TAB))
                {
                    pauseOpen = true;
                    pauseMenu.page = PausePage::Root;
                    pauseMenu.menuIndex = 0;
                    pauseMenu.message.clear();
                }
                else if (IsKeyPressed(KEY_I))
                {
                    pauseOpen = true;
                    pauseMenu.page = PausePage::Items;
                    pauseMenu.itemIndex = 0;
                    pauseMenu.itemTargeting = false;
                    pauseMenu.message.clear();
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

                    party.ReviveFallenOneHp();
                }
                else
                {
                    party.RestoreAll();
                }

                battle.reset();

                if (battleMusicActive)
                {
                    StopMusicStream(battleMusic);
                    battleMusicActive = false;
                }

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

        if (state == AppState::Playing)
        {
            updateOverworldCamera(GetFrameTime());
            snapCamera = false;
        }

        BeginDrawing();

        if (!battle)
        {
            ui.DrawOverworld(ball, enemy, party, map, pendingBattle, dungeonFloor,
                !pauseOpen, overworldCamera);

            if (pauseOpen)
                ui.DrawPauseMenu(party, inventory, pauseMenu, dungeonFloor);
        }
        else
        {
            ui.DrawBattle(*battle);
        }

        EndDrawing();
    }

    if (battleMusicActive)
        StopMusicStream(battleMusic);

    if (battleMusic.frameCount > 0)
        UnloadMusicStream(battleMusic);

    CloseAudioDevice();

    CloseWindow();
}

