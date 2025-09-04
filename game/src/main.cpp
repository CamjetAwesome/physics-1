#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"

const unsigned int TARGET_FPS = 50;
float time = 0;
float dt = 0;
float x = 500;
float y = 500;
float frequency = 1.0f;
float amplitude = 70;

int minuteCounter = 0;
float secondCounter = 0;

void Update() 
{
    dt = 1.0f / TARGET_FPS;
    time += dt;

    x = x + (-sin(time * frequency)) * frequency * amplitude * dt;
    y = y + (cos(time * frequency)) * frequency * amplitude * dt;

    minuteCounter = time / 60;
    secondCounter = time - (minuteCounter * 60);
}

void Draw()
{
    BeginDrawing();
    ClearBackground(BLUE);
    //DrawText("Hello world!", 10, 10, 20, LIGHTGRAY);

    GuiSliderBar(Rectangle{ 60, 5, 1000, 10 }, "Time", TextFormat("%.2f", time), &time, 0, 240);
    DrawText(TextFormat("Time: %.2f", time), GetScreenWidth() - 200, 20, 20, LIGHTGRAY);

    DrawCircle(x, y, 60, RED);
    DrawCircle(GetScreenWidth() / 2 + cos(time * frequency) * amplitude, GetScreenHeight() / 2, 60, GREEN);
    DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2 + sin(time * frequency) * amplitude, 60, DARKPURPLE);

    EndDrawing();
}

int main()
{
    InitWindow(InitialWidth, InitialHeight, "Cameron's Very Own Physics Engine");
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }

    CloseWindow();
    return 0;
}