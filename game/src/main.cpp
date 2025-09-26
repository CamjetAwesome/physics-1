#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"
using namespace std;
#include <string>
#include <vector>

const unsigned int TARGET_FPS = 50;
float time = 0;
float deltatime = 0;

Vector2 position = { 300, 600 };
float speed = 100;
float angle = 0;

class PhysicsObject
{
public:
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float mass = 1; // kg
    float radius = 15; // circular radius in pixels
    std::string name = "object";
    Color color = RED;

    void Draw()
    {
        DrawCircle(position.x, position.y, radius, color);
        DrawText(TextFormat("Size: %0.0f", radius), position.x, position.y, 20, WHITE);
    }
};

class PhysicsWorld
{
private:
    unsigned int objectCount;
public:
    std::vector<PhysicsObject> objects;
    Vector2 accelerationGravity = { 0, 9 };

    void Update()
    {
        for (int i = 0; i < objects.size(); i++)
        {
            //velocity = change in position over time, therefore: change in position = velocity * time
            objects[i].position += objects[i].velocity * deltatime;
            objects[i].velocity += accelerationGravity * deltatime;
            DrawLineEx(objects[i].position, objects[i].position + objects[i].velocity, 5, objects[i].color);
            objects[i].Draw();
        }
    }

    void add(PhysicsObject newObject)
    {
        objects.push_back(newObject);
        objectCount++;
    }    
};

PhysicsWorld world;

int minuteCounter = 0;
float secondCounter = 0;
string timer;

void Cleanup()
{
    for (int i = 0; i < world.objects.size(); i++)
    {
        //remove all objects out of bounds
    }
}

void Update()
{
    deltatime = 1.0f / TARGET_FPS;
    time += deltatime;

#pragma region Timer
    
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

#pragma endregion

    if (IsKeyPressed(KEY_SPACE))
    {
        PhysicsObject newObject;
        newObject.position = position;
        newObject.velocity = { speed * (float)cos(angle * DEG2RAD), -speed * (float)sin(angle * DEG2RAD) };

        // rand() % N produces random number from 0 to N - 1
        newObject.radius = (rand() & 11) + 5; // between 5 to 15

        world.add(newObject);
    }

    world.Update();
}

void Draw()
{
    BeginDrawing();
    ClearBackground(BLUE);
    
    GuiSliderBar(Rectangle{ 100, 5, 500, 25 }, "Time", TextFormat("%.2f", time), &time, 0, 600);
    DrawText(timer.c_str(), 25, 130, 30, LIGHTGRAY);

    GuiSliderBar(Rectangle{ 100, 30, 500, 25 }, "Angle", TextFormat("Angle: %.0f Degrees", angle), &angle, -180, 180);
    GuiSliderBar(Rectangle{ 100, 65, 500, 25 }, "Speed", TextFormat("Speed: %0.f", speed), &speed, 10, 500);
    GuiSliderBar(Rectangle{ 100, 100, 500, 25 }, "Gravity", TextFormat("Gravity: %0.f Pixels/Sec^2", world.accelerationGravity.y), &world.accelerationGravity.y, -100, 100);

    Vector2 startPos = position;
    Vector2 velocity = { speed * cos(angle * DEG2RAD), -speed * sin(angle * DEG2RAD)};

    DrawLineEx({ startPos.x - speed, startPos.y }, { startPos.x + speed, startPos.y}, 7, BLACK);
    DrawLineEx(startPos, startPos + velocity, 5, RED);

    EndDrawing();
}

int main()
{
    InitWindow(InitialWidth, InitialHeight, "Cameron's Very Own Physics Engine");
    SetTargetFPS(TARGET_FPS);

    Font figtree = LoadFontEx("Resources/figtree-regular.ttf", 32, 0, 250);
    GuiSetFont(figtree);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xFFFFFFFF);

    while (!WindowShouldClose())
    {   
        Update();
        Draw();
    }

    CloseWindow();
    return 0;
}