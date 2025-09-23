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

float speed = 100;
float angle = 0;

int minuteCounter = 0;
float secondCounter = 0;
string timer;

void Update()
{
    deltatime = 1.0f / TARGET_FPS;
    time += deltatime;

    minuteCounter = (int)time / 60;
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
    
    GuiSliderBar(Rectangle{ 75, 5, 500, 25 }, "Time", TextFormat("%.2f", time), &time, 0, 600);
    DrawText(timer.c_str(), 25, 110, 30, LIGHTGRAY);

    GuiSliderBar(Rectangle{ 75, 25, 500, 25 }, "Angle", TextFormat("Angle: %.0f Degrees", angle), &angle, -180, 180);
    GuiSliderBar(Rectangle{ 75, 45, 500, 25 }, "Speed", TextFormat("Speed: ", speed), &speed, 10, 200);

    Vector2 startPos = { 300, 500 };
    Vector2 velocity = { speed * cos(angle * DEG2RAD), speed * sin(angle * DEG2RAD)};

    DrawLineEx({ startPos.x - speed, startPos.y }, { startPos.x + speed, startPos.y}, 7, BLACK);
    DrawLineEx(startPos, startPos + velocity, 5, RED);

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