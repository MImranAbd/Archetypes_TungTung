#pragma once

#include <raylib.h>

namespace UiInternal
{
    extern Font FontRegular;
    extern Font FontBold;
    extern Texture2D PartyIdle;
    extern Texture2D PartyAttack;
    extern Texture2D PartyPortrait;
    extern Texture2D Slime;
    extern bool SpritesLoaded;
    extern const Color PartyTints[4];

    void UiDrawText(const char* text, int x, int y, int size, Color color, bool bold = false);
    int UiMeasureText(const char* text, int size, bool bold = false);
    void DrawPageFrame();
    void DrawBackdrop();
    void DrawCentered(const char* text, int centerX, int y, int size, Color color, bool bold = false);
    void DrawHpBar(Rectangle bar, int value, int maxValue, Color fill);
}
