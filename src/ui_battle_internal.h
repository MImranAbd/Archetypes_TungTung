#pragma once

#include <raylib.h>
#include <string>
#include "battle.h"
#include "party.h"

namespace BattleUi
{
    extern const Vector2 PartyPos[4];
    extern const Vector2 EnemyPos[5];
    extern const float EnemySizes[5];

    const char* CommandLabel(int index);
    const char* CommandDescription(int index);
    const char* ItemDescription(ItemType type);

    void DrawWrappedText(const char* text, int x, int y, int maxWidth, int fontSize,
        Color color, bool bold = false);
    void DrawTurnSquares(int points, int halfIcons, int maxIcons, int x, int y,
        Color fill);
    void DrawTargetCaret(float x, float y, Color color);
    void DrawHitFlash(int x, int y, float scale, float alpha);
    void DrawPartyActor(int index, Vector2 pos, int activeIndex, int attackingIndex,
        const Party& party, Color effectTint = BLANK);
    void DrawEnemyActor(int index, Vector2 pos, bool selected, const Enemy& enemy,
        Color effectTint = BLANK, bool acting = false, float sizeScale = 1.0f);
    void DrawPartyCards(const Battle& battle, int x, int y, int width);
    void DrawBattleVictory(const Battle& battle);
    void DrawBattleDefeat();
}

