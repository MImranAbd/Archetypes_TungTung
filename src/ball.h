#pragma once

class GameMap;

class Ball
{
public:
    Ball();
    void Update(const GameMap& map, int blockedX = -1, int blockedY = -1);
    void UpdateVisual();
    void Draw() const;

    int GetGridX() const;
    int GetGridY() const;
    float GetPixelX() const;
    float GetPixelY() const;
    int GetFacingX() const;
    int GetFacingY() const;
    bool IsMoving() const;
    bool IsRunning() const;

    void Dash(int cells, const GameMap& map);
    void SetGridPosition(int gridX, int gridY);
    bool ConsumeExit(int& dirX, int& dirY);

    static constexpr int cellSize = 40;
    static constexpr float repeatDelay = 0.10f;

private:
    int gridX;
    int gridY;
    float x;
    float y;
    float repeatTimer;
    int facingX;
    int facingY;
    bool moving;
    bool running;
    int exitDirX;
    int exitDirY;
    int radius;
};
