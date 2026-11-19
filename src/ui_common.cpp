#include "ui.h"
#include "ui_internal.h"
#include "theme.h"

#include <raylib.h>
#include "raygui.h"

using namespace UiInternal;

namespace UiInternal
{
    Font FontRegular;
    Font FontBold;
    Texture2D PartyIdle;
    Texture2D PartyAttack;
    Texture2D PartyPortrait;
    Texture2D Slime;
    bool SpritesLoaded = false;

    const Color PartyTints[4] = {
        WHITE,
        { 205, 160, 220, 255 },
        { 160, 205, 170, 255 },
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

    PartyIdle = LoadTexture("res/draw/Side_stand.png");
    PartyAttack = LoadTexture("res/draw/Punch.png");
    PartyPortrait = LoadTexture("res/draw/Stand.png");
    Slime = LoadTexture("res/draw/Slime.png");

    SpritesLoaded = PartyIdle.id > 0 && PartyAttack.id > 0 &&
        PartyPortrait.id > 0 && Slime.id > 0;

    if (PartyIdle.id > 0) SetTextureFilter(PartyIdle, TEXTURE_FILTER_POINT);
    if (PartyAttack.id > 0) SetTextureFilter(PartyAttack, TEXTURE_FILTER_POINT);
    if (PartyPortrait.id > 0) SetTextureFilter(PartyPortrait, TEXTURE_FILTER_POINT);
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
