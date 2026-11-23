#pragma once

#include <raylib.h>
#include "enemies.h"

namespace UiInternal
{
    struct EnemyAnimData
    {
        Texture2D texture = {};
        int frameCount = 0;
        int frameWidth = 0;
        int frameHeight = 0;
        int attackFrame = 0;
    };

    extern Font FontRegular;
    extern Font FontBold;
    extern Texture2D ElizaStand;
    extern Texture2D ElizaWalkDown[4];
    extern Texture2D ElizaWalkSide[4];
    extern Texture2D ElizaWalkUp[4];
    extern Texture2D PartyBattlePoses[4];
    extern Texture2D PartyAttackPoses[4];
    extern Texture2D MenuPortrait;
    extern Texture2D MenuPolygon;
    extern Texture2D MenuFace;
    extern Texture2D MenuTitles[6];
    extern EnemyAnimData EnemyAnims[EnemyKindCount];
    extern bool SpritesLoaded;
    extern const Color PartyTints[4];

    void UiDrawText(const char* text, int x, int y, int size, Color color, bool bold = false);
    int UiMeasureText(const char* text, int size, bool bold = false);
    void DrawPageFrame();
    void DrawBackdrop();
    void DrawCentered(const char* text, int centerX, int y, int size, Color color, bool bold = false);
    void DrawHpBar(Rectangle bar, int value, int maxValue, Color fill);
}
