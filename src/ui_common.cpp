#include "ui.h"
#include "ui_internal.h"
#include "theme.h"

#include <raylib.h>
#include "raygui.h"
#include <string>

using namespace UiInternal;

namespace UiInternal
{
    Font FontRegular;
    Font FontBold;
    Texture2D ElizaStand = {};
    Texture2D ElizaWalkDown[4] = {};
    Texture2D ElizaWalkSide[4] = {};
    Texture2D ElizaWalkUp[4] = {};
    Texture2D PartyBattlePoses[4] = {};
    Texture2D PartyAttackPoses[4] = {};
    Texture2D MenuPortrait = {};
    Texture2D MenuPolygon = {};
    Texture2D MenuFace = {};
    Texture2D MenuTitles[6] = {};
    Texture2D Slime;

    Texture2D LoadWhiteMenuTitle(const char* path)
    {
        Image image = LoadImage(path);

        if (image.data == nullptr)
            return Texture2D{};

        unsigned char* pixels = static_cast<unsigned char*>(image.data);
        const int count = image.width * image.height;

        for (int i = 0; i < count; i++)
        {
            if (pixels[i * 4 + 3] == 0)
                continue;

            pixels[i * 4 + 0] = 255;
            pixels[i * 4 + 1] = 255;
            pixels[i * 4 + 2] = 255;
        }

        Texture2D texture = LoadTextureFromImage(image);
        UnloadImage(image);

        if (texture.id > 0)
            SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);

        return texture;
    }
    bool SpritesLoaded = false;

    const Color PartyTints[4] = {
        WHITE,
        { 185, 205, 245, 255 },
        { 245, 225, 170, 255 },
        { 235, 190, 165, 255 }
    };

    void UiDrawText(const char* text, int x, int y, int size, Color color, bool bold)
    {
        Font font = bold ? FontBold : FontRegular;
        if (font.texture.id == 0) font = GetFontDefault();

        DrawTextEx(font, text, { static_cast<float>(x), static_cast<float>(y) },
            static_cast<float>(size), 1.0f, color);
    }

    int UiMeasureText(const char* text, int size, bool bold)
    {
        Font font = bold ? FontBold : FontRegular;
        if (font.texture.id == 0) font = GetFontDefault();

        return static_cast<int>(MeasureTextEx(font, text,
            static_cast<float>(size), 1.0f).x);
    }

    void DrawPageFrame()
    {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();

        DrawRectangleLinesEx(
            { 14.0f, 14.0f, static_cast<float>(width) - 28.0f, static_cast<float>(height) - 28.0f },
            1.0f, Theme::Ink);

        DrawRectangleLinesEx(
            { 20.0f, 20.0f, static_cast<float>(width) - 40.0f, static_cast<float>(height) - 40.0f },
            1.0f, Fade(Theme::Ink, 0.25f));
    }

    void DrawBackdrop()
    {
        ClearBackground(Theme::Parchment);
        DrawPageFrame();
    }

    void DrawCentered(const char* text, int centerX, int y, int size, Color color, bool bold)
    {
        UiDrawText(text, centerX - UiMeasureText(text, size, bold) / 2, y, size, color, bold);
    }

    void DrawHpBar(Rectangle bar, int value, int maxValue, Color fill)
    {
        GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, ColorToInt(fill));

        float current = static_cast<float>(value);
        GuiProgressBar(bar, nullptr, nullptr, &current, 0.0f, static_cast<float>(maxValue));
    }
}

UI::UI()
    : slashTimer(0.0f)
    , slashCellX(0)
    , slashCellY(0)
{
    GuiLoadStyleDefault();

    FontRegular = LoadFontEx("C:/Windows/Fonts/georgia.ttf", 64, nullptr, 0);
    FontBold = LoadFontEx("C:/Windows/Fonts/georgiab.ttf", 64, nullptr, 0);
    if (FontRegular.texture.id == 0) FontRegular = GetFontDefault();
    if (FontBold.texture.id == 0) FontBold = FontRegular;

    SetTextureFilter(FontRegular.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(FontBold.texture, TEXTURE_FILTER_BILINEAR);
    GuiSetFont(FontRegular);

    ElizaStand = LoadTexture("res/draw/Character/ELIZA/Stand.png");

    for (int i = 0; i < 4; i++)
    {
        const std::string number = std::to_string(i + 1);

        ElizaWalkDown[i] = LoadTexture(("res/draw/Character/ELIZA/ElizaWalkDown"
            + number + ".png").c_str());
        ElizaWalkSide[i] = LoadTexture(("res/draw/Character/ELIZA/ElizaWalkSide"
            + number + ".png").c_str());
        ElizaWalkUp[i] = LoadTexture(("res/draw/Character/ELIZA/ElizaWalkUp"
            + number + ".png").c_str());
    }

    const char* const battlePaths[4] = {
        "res/draw/Character/ELIZA/Battle_Stance.png",
        "res/draw/Character/ARES/BattleAres.png",
        "res/draw/Character/LEO/BattleLeo.png",
        "res/draw/Character/TROY/BattleTroy.png"
    };

    const char* const attackPaths[4] = {
        "res/draw/Character/ELIZA/16x32 Attack.png",
        "res/draw/Character/ARES/AttackAres.png",
        "res/draw/Character/LEO/AttackLeo.png",
        "res/draw/Character/TROY/AttackTroy.png"
    };

    for (int i = 0; i < 4; i++)
    {
        PartyBattlePoses[i] = LoadTexture(battlePaths[i]);
        PartyAttackPoses[i] = LoadTexture(attackPaths[i]);
    }

    Slime = LoadTexture("res/draw/Slime.png");

    MenuPortrait = LoadTexture("res/draw/Menu/image 4.png");
    MenuPolygon = LoadTexture("res/draw/Menu/Polygon 37.png");
    MenuFace = LoadTexture("res/draw/Menu/TesIdle 4.png");

    const char* menuTitlePaths[6] = {
        "res/draw/Menu/ITEM-1.png",
        "res/draw/Menu/EQUIPMENT.png",
        "res/draw/Menu/SKILLS.png",
        "res/draw/Menu/QUESTS.png",
        "res/draw/Menu/MEMBER-1.png",
        "res/draw/Menu/SYSTEMS.png"
    };

    for (int i = 0; i < 6; i++)
        MenuTitles[i] = LoadWhiteMenuTitle(menuTitlePaths[i]);

    if (MenuPortrait.id > 0) SetTextureFilter(MenuPortrait, TEXTURE_FILTER_POINT);
    if (MenuPolygon.id > 0) SetTextureFilter(MenuPolygon, TEXTURE_FILTER_POINT);
    if (MenuFace.id > 0) SetTextureFilter(MenuFace, TEXTURE_FILTER_POINT);

    SpritesLoaded = Slime.id > 0 && ElizaStand.id > 0;

    for (int i = 0; i < 4; i++)
    {
        SpritesLoaded = SpritesLoaded &&
            ElizaWalkDown[i].id > 0 &&
            ElizaWalkSide[i].id > 0 &&
            ElizaWalkUp[i].id > 0 &&
            PartyBattlePoses[i].id > 0 &&
            PartyAttackPoses[i].id > 0;

        if (ElizaWalkDown[i].id > 0) SetTextureFilter(ElizaWalkDown[i], TEXTURE_FILTER_POINT);
        if (ElizaWalkSide[i].id > 0) SetTextureFilter(ElizaWalkSide[i], TEXTURE_FILTER_POINT);
        if (ElizaWalkUp[i].id > 0) SetTextureFilter(ElizaWalkUp[i], TEXTURE_FILTER_POINT);
        if (PartyBattlePoses[i].id > 0) SetTextureFilter(PartyBattlePoses[i], TEXTURE_FILTER_POINT);
        if (PartyAttackPoses[i].id > 0) SetTextureFilter(PartyAttackPoses[i], TEXTURE_FILTER_POINT);
    }

    if (ElizaStand.id > 0) SetTextureFilter(ElizaStand, TEXTURE_FILTER_POINT);
    if (Slime.id > 0) SetTextureFilter(Slime, TEXTURE_FILTER_POINT);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(DEFAULT, TEXT_PADDING, 8);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(Theme::Parchment));
    GuiSetStyle(DEFAULT, LINE_COLOR, ColorToInt(Theme::Ink));
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(Theme::Ink));
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(Theme::Parchment));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(Theme::Ink));

    GuiSetStyle(PROGRESSBAR, BORDER_COLOR_NORMAL, ColorToInt(Theme::Ink));
    GuiSetStyle(PROGRESSBAR, BORDER_COLOR_FOCUSED, ColorToInt(Theme::Gold));
    GuiSetStyle(PROGRESSBAR, BASE_COLOR_NORMAL, ColorToInt(Theme::ParchmentDark));
    GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, ColorToInt(Theme::Forest));
    GuiSetStyle(PROGRESSBAR, BASE_COLOR_FOCUSED, ColorToInt(Theme::ParchmentDark));
    GuiSetStyle(PROGRESSBAR, PROGRESS_PADDING, 0);
}

void UI::Update(float deltaTime)
{
    if (slashTimer > 0.0f)
        slashTimer -= deltaTime;
}

void UI::StartSlash(int cellX, int cellY)
{
    slashTimer = slashDuration;
    slashCellX = cellX;
    slashCellY = cellY;
}
