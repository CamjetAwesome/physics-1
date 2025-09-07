#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"
using namespace std;
#include <string>

const unsigned int TARGET_FPS = 50;
float time = 0;
float deltatime = 0;
float x = 900;
float y = 300;
float frequency = 1.0f;
float amplitude = 70;

int minuteCounter = 0;
float secondCounter = 0;
string timer;

void Update()
{
    deltatime = 1.0f / TARGET_FPS;
    time += deltatime;

    x = x + (sin(time * frequency)) * frequency * amplitude * deltatime;
    y = y + (cos(time * frequency)) * frequency * amplitude * deltatime;

    minuteCounter = time / 60;
    secondCounter = time - (minuteCounter * 60);
    timer = "Time: ";
    if (minuteCounter < 10) {
        timer += "0";
    }
    timer += (string)TextFormat("%d", minuteCounter) + ":";
    if (secondCounter < 10) {
        timer += "0";
    }
    timer += (string)TextFormat("%0.2f", secondCounter);
}

void Draw()
{
    BeginDrawing();
    ClearBackground(BLUE);
    //DrawText("Hello world!", 10, 10, 20, LIGHTGRAY);

    GuiSliderBar(Rectangle{ 75, 5, 500, 25 }, "Time", TextFormat("%.2f", time), &time, 0, 600);
    GuiSliderBar(Rectangle{ 75, 35, 100, 25 }, "Frequency", TextFormat("%.2f", frequency), &frequency, 0, 2);
    GuiSliderBar(Rectangle{ 75, 65, 100, 25 }, "Amplitude", TextFormat("%.2f", amplitude), &amplitude, 0, 140);
    DrawText(timer.c_str(), 25, 110, 30, LIGHTGRAY);
    DrawText(TextFormat("Frequency: %0.2f", frequency), 25, 140, 30, LIGHTGRAY);
    DrawText(TextFormat("Amplitude: %0.2f", amplitude), 25, 170, 30, LIGHTGRAY);
    
    DrawCircle(x, y, 60, RED);
    DrawCircle(GetScreenWidth() / 2 + cos(time * frequency) * amplitude, GetScreenHeight() / 2, 60, GREEN);
    DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2 + sin(time * frequency) * amplitude, 60, DARKPURPLE);
    DrawCircle(GetScreenWidth() / 2 + cos(time * frequency) * amplitude, GetScreenHeight() / 2 + sin(time * frequency) * amplitude, 60, ORANGE);

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