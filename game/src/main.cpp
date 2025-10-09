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

enum PhysicsShape {
    CIRCLE,
    HALFSPACE
};

void CheckCollision();

class PhysicsObject
{
public:
    bool isStatic = false;
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float mass = 1; // kg
    float radius = 15; // circular radius in pixels
    std::string name = "object";
    Color color = RED;

    virtual void Draw()
    {
        
    }

    virtual PhysicsShape GetShape() = 0;
};

class PhysicsCircle : public PhysicsObject
{
public:
    float radius = 15;

    void Draw() override
    {
        DrawCircle(position.x, position.y, radius, color);
        DrawText(TextFormat("Size: %0.0f", radius), position.x, position.y, 20, WHITE);
    }

    PhysicsShape GetShape() override
    {
        return CIRCLE;
    }
};

class PhysicsHalfSpace : public PhysicsObject
{
private:
    float rotation = 0;
    Vector2 normal = {0,-1};

public:
#pragma region Get/Set Methods

    float GetRotation()
    {
        return rotation;
    }

    Vector2 GetNormal()
    {
        return normal;
    }

    void SetRotationInDegrees(float rotationInDegrees)
    {
        rotation = rotationInDegrees;
        normal = Vector2Rotate({ 0, -1 }, rotation * DEG2RAD);
    }

    PhysicsShape GetShape() override
    {
        return HALFSPACE;
    }

#pragma endregion

    void Draw() override
    {
        // draw arbitrary point on the line
        DrawCircle(position.x, position.y, 8, RED);

        // draw normal vector, perpendicular to the surface
        DrawLineEx(position, position + normal, 1, color);
        
        // draw the line/surface
        Vector2 ParallelToSurface = Vector2Rotate(normal, PI * 0.5f);
        DrawLineEx(position - ParallelToSurface * 4000, position + ParallelToSurface * 4000, 1, color);

    }    
};

class PhysicsWorld
{
private:
    unsigned int objectCount;
public:
    std::vector<PhysicsObject*> objects;
    Vector2 accelerationGravity = { 0, 9 };

    void Update()
    {
        for (int i = 0; i < objects.size(); i++)
        {
            PhysicsObject* object = objects[i];
            //velocity = change in position over time, therefore: change in position = velocity * time

            if (object->isStatic) continue;

            object->position += object->velocity * deltatime;
            object->velocity += accelerationGravity * deltatime;
            DrawLineEx(object->position, object->position + object->velocity, 5, object->color);
            object->Draw();
        }

        CheckCollision();
    }

    void add(PhysicsObject* newObject)
    {
        objects.push_back(newObject);
        objectCount++;
    }    
};

PhysicsWorld world;
PhysicsHalfSpace halfspace;
float halfspaceRotation = halfspace.GetRotation();

int minuteCounter = 0;
float secondCounter = 0;
string timer;

bool CheckCircleCircleOverlap(PhysicsCircle* A, PhysicsCircle* B)
{
    Vector2 displacement = B->position - A->position;
    float distance = Vector2Length(displacement); // Use pythagorean theorem to get magnitude
    float sumOfRadii = A->radius + B->radius;
    return sumOfRadii > distance;
}

bool CheckCircleHalfspaceOverlap(PhysicsCircle* Circle, PhysicsHalfSpace* Halfspace)
{
    // Not finished!
    Vector2 displacement = Halfspace->position - Circle->position;
    float distance = Vector2Length(displacement);

    DrawLineEx(Circle->position, Halfspace->position, 1, GRAY);
    Vector2 midpoint = Halfspace->position + displacement * 0.5f;
    //DrawText("D: ", midpoint)

    return false;
}

void CheckCollision()
{
    for (int i = 0; i < world.objects.size(); i++)
    {
        world.objects[i]->color = GREEN;
    }

    for (int i = 0; i < world.objects.size(); i++)
    {
        for (int j = i + 1; j < world.objects.size(); j++) 
        {
            PhysicsObject* ObjectA = world.objects[i];
            PhysicsObject* ObjectB = world.objects[j];

            PhysicsShape ShapeA = ObjectA->GetShape();
            PhysicsShape ShapeB = ObjectB->GetShape();

            bool overlap = false;

            //Check which collision function to use
            // Circle - Circle Collision
            if (ShapeA == CIRCLE && ShapeB == CIRCLE)
            {
                overlap = CheckCircleCircleOverlap((PhysicsCircle*)ObjectA, (PhysicsCircle*)ObjectB);
            }
            // Circle - Halfspace Collision
            else if (ShapeA == CIRCLE && ShapeB == HALFSPACE)
            {
                overlap = CheckCircleHalfspaceOverlap((PhysicsCircle*)ObjectA, (PhysicsHalfSpace*)ObjectB);
            }
            else if (ShapeA == HALFSPACE && ShapeB == CIRCLE)
            {
                overlap = CheckCircleHalfspaceOverlap((PhysicsCircle*)ObjectB, (PhysicsHalfSpace*)ObjectA);
            }

            if (overlap)
            {
                ObjectA->color = RED;
                ObjectB->color = RED;
            }
        }
    }
}

void Cleanup()
{
    for (int i = 0; i < world.objects.size(); i++)
    {
        PhysicsObject* object = world.objects[i];
        //remove all objects out of bounds
        if (object->position.y > GetScreenHeight()
            || object->position.y < 0
            || object->position.x > GetScreenWidth()
            || object->position.x < 0)
        {
            // destroy
            std::vector<PhysicsObject*>::iterator iterator = (world.objects.begin() + 1);
            PhysicsObject* pointerToObj = *iterator;
            delete pointerToObj;
            world.objects.erase(iterator);
            i--;
        }
    }
}

void Update()
{
    deltatime = 1.0f / TARGET_FPS;
    time += deltatime;

//#pragma region Timer
//    
//    minuteCounter = (int)time / 60;
//    secondCounter = time - (minuteCounter * 60);
//    timer = "Time: ";
//    if (minuteCounter < 10) {
//        timer += "0";
//    }
//    timer += (string)TextFormat("%d", minuteCounter) + ":";
//    if (secondCounter < 10) {
//        timer += "0";
//    }
//    timer += (string)TextFormat("%0.2f", secondCounter);
//
//#pragma endregion

    if (IsKeyPressed(KEY_SPACE))
    {
        PhysicsCircle* newObject = new PhysicsCircle();
        newObject->position = position;
        newObject->velocity = { speed * (float)cos(angle * DEG2RAD), -speed * (float)sin(angle * DEG2RAD) };

        // rand() % N produces random number from 0 to N - 1
        newObject->radius = (rand() & 11) + 5; // between 5 to 15

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

    halfspace.Draw();

    

    // halfspace controls
    GuiSliderBar(Rectangle{ 100, 135, 500, 25 }, "Halfspace X", TextFormat("%.0f", halfspace.position.x), &halfspace.position.x, 0, GetScreenWidth());
    GuiSliderBar(Rectangle{ 100, 165, 500, 25 }, "Halfspace Y", TextFormat("%.0f", halfspace.position.y), &halfspace.position.y, 0, GetScreenHeight());
    GuiSliderBar(Rectangle{ 100, 195, 500, 25 }, "Halfspace Rotation", TextFormat("%.0f", halfspaceRotation), &halfspaceRotation, -360, 360);
    halfspace.SetRotationInDegrees(halfspaceRotation);
    halfspace.isStatic = true;

    Cleanup();
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

    halfspace.position = { 500,700 };
    world.add(&halfspace);

    while (!WindowShouldClose())
    {   
        Update();
        Draw();
    }

    CloseWindow();
    return 0;
}