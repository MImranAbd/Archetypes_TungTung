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
    Texture2D PartyIdleSheets[4] = {};
    Texture2D PartyWalkSheets[4] = {};
    Texture2D PartyAttackSheets[4] = {};
    Texture2D Slime;
    bool SpritesLoaded = false;

    const char* const PartyFolders[4] = { "ELIZA", "ARES", "LEO", "TROY" };

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

    for (int i = 0; i < 4; i++)
    {
        const std::string base = std::string("res/draw/Character/")
            + PartyFolders[i] + "/16x32 ";

        PartyIdleSheets[i] = LoadTexture((base + "Idle-Sheet.png").c_str());
        PartyWalkSheets[i] = LoadTexture((base + "Walk-Sheet.png").c_str());
        PartyAttackSheets[i] = LoadTexture((base + "Attack-Sheet.png").c_str());
    }

    Slime = LoadTexture("res/draw/Slime.png");

    SpritesLoaded = Slime.id > 0;

    for (int i = 0; i < 4; i++)
    {
        SpritesLoaded = SpritesLoaded &&
            PartyIdleSheets[i].id > 0 &&
            PartyWalkSheets[i].id > 0 &&
            PartyAttackSheets[i].id > 0;
    }

    for (int i = 0; i < 4; i++)
    {
        if (PartyIdleSheets[i].id > 0) SetTextureFilter(PartyIdleSheets[i], TEXTURE_FILTER_POINT);
        if (PartyWalkSheets[i].id > 0) SetTextureFilter(PartyWalkSheets[i], TEXTURE_FILTER_POINT);
        if (PartyAttackSheets[i].id > 0) SetTextureFilter(PartyAttackSheets[i], TEXTURE_FILTER_POINT);
    }

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
