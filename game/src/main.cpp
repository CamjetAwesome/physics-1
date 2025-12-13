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
float bounciness = 1.0f;

Vector2 position = { 300, 600 };
float speed = 100;
float angle = 0;

enum PhysicsShape {
    CIRCLE,
    HALFSPACE,
    AABB
};

enum SlingshotState
{
    SLING_IDLE,
    SLING_DRAG
};

void CheckCollisions();

class PhysicsObject
{
public:
    bool isStatic = false;
    Vector2 position = { 0, 0 };
    Vector2 velocity = { 0, 0 };
    float mass = 5; // kg
    float radius = 15; // circular radius in pixels
    std::string name = "object";
    Color color = GREEN;
    Vector2 netForce = { 0, 0 };
    float grippiness = 0.1f;
    float bounciness = 0.9f;

    virtual void Draw()
    {

    }

    virtual PhysicsShape GetShape() = 0;
};

class PhysicsWorld
{
private:
    unsigned int objectCount;
public:
    std::vector<PhysicsObject*> objects;
    Vector2 accelerationGravity = { 0, 9 };

    void Draw()
    {
        for (int i = 0; i < objects.size(); i++)
        {
            PhysicsObject* object = objects[i];

            DrawLineEx(object->position, object->position + object->velocity, 5, object->color);
            object->Draw();
        }
    }

    void ResetNetForces()
    {
        for (int i = 0; i < objects.size(); i++)
        {
            PhysicsObject* object = objects[i];

            object->netForce = { 0,0 };
        }
    }

    void ApplyGravity()
    {
        for (int i = 0; i < objects.size(); i++)
        {
            PhysicsObject* object = objects[i];

            if (object->isStatic) continue;

            Vector2 Fgravity = accelerationGravity * object->mass;
            object->netForce += Fgravity;
            Vector2 FgravityLine = { object->position.x + Fgravity.x, object->position.y + Fgravity.y };
            DrawLineEx(object->position, FgravityLine, 6, PURPLE);
        }
    }

    void ApplyKinematics()
    {
        for (int i = 0; i < objects.size(); i++)
        {
            PhysicsObject* object = objects[i];

            if (object->isStatic) continue;

            //Acceleration should be based on net force on an object. F = M*A; A = F/M
            Vector2 acceleration = object->netForce / object->mass;

            object->velocity += acceleration * deltatime;
            object->position += object->velocity * deltatime;
        }
    }

    void Update()
    {
        ResetNetForces();
        ApplyGravity();
        CheckCollisions();
        ApplyKinematics();
    }

    void add(PhysicsObject* newObject)
    {
        objects.push_back(newObject);
        objectCount++;
    }
};

PhysicsWorld world;

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

class PhysicsBox : public PhysicsObject
{
public:
    Vector2 size = { 0, 0 };
    float mass = 10;

    void Draw() override
    {
        float minX = position.x - size.x * 0.5f;
        float minY = position.y - size.y * 0.5f;
        float maxX = position.x + size.x * 0.5f;
        float maxY = position.y + size.y * 0.5f;


        DrawRectangle(minX, minY, size.x, size.y, color);
    }

    PhysicsShape GetShape() override
    {
        return AABB;
    }
};

class PhysicsHalfSpace : public PhysicsObject
{
private:
    float rotation = 0;
    Vector2 normal = { 0,-1 };

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

PhysicsHalfSpace halfspace;
PhysicsHalfSpace halfspace2;
float halfspaceRotation = halfspace.GetRotation();

int minuteCounter = 0;
float secondCounter = 0;
string timer;

//bool CheckCircleCircleOverlap(PhysicsCircle* A, PhysicsCircle* B)
//{
//    Vector2 displacement = B->position - A->position;
//    float distance = Vector2Length(displacement); // Use pythagorean theorem to get magnitude
//    float sumOfRadii = A->radius + B->radius;
//    return sumOfRadii > distance;
//}

bool CircleCircleCollisionResponse(PhysicsCircle* A, PhysicsCircle* B)
{
    Vector2 displacement = B->position - A->position;
    float distance = Vector2Length(displacement); // Use pythagorean theorem to get magnitude
    float sumOfRadii = A->radius + B->radius;

    float overlap = sumOfRadii - distance;
    Vector2 normal = displacement / distance;
    Vector2 mtv = normal * overlap; // minimum translation vector (to move to have objects not overlap

    if (overlap >= 0)
    {
        A->position -= mtv * 0.5f;
        B->position += mtv * 0.5f;

        // From the perspective of A
        Vector2 velocityBRelativeToA = B->velocity - A->velocity;
        float closingVelocity1D = Vector2DotProduct(velocityBRelativeToA, normal);

        if (closingVelocity1D >= 0) return true;

        float restitution = A->bounciness * B->bounciness;

        float totalMass = A->mass + B->mass;
        float impulseMagnitude = ((1.0f * restitution) * closingVelocity1D * A->mass * B->mass / totalMass);

        Vector2 impulseA = normal * impulseMagnitude;
        Vector2 impulseB = normal * -impulseMagnitude;

        A->velocity += impulseA / A->mass;
        B->velocity += impulseB / B->mass;

        return true;
    }

    return false;
}

bool BoxBoxOverlap(PhysicsBox* A, PhysicsBox* B)
{
    float aExtentsX = A->size.x * 0.5f; // half width
    float aExtentsY = A->size.y * 0.5f; // half height
    float bExtentsX = B->size.x * 0.5f;
    float bExtentsY = B->size.y * 0.5f;

    // A min and max points
    float aMinX = A->position.x - aExtentsX; // min (furthest left)
    float aMaxX = A->position.x + aExtentsX; // max (furthest right)

    float aMinY = A->position.y - aExtentsY; // min (furthest up)
    float aMaxY = A->position.y + aExtentsY; // max (furthest down)

    // B min and max points
    float bMinX = B->position.x - bExtentsX;
    float bMaxX = B->position.x + bExtentsX;

    float bMinY = B->position.y - bExtentsY;
    float bMaxY = B->position.y + bExtentsY;

    float distanceX = 0;
    float distanceY = 0;

    // to calculate overlap, we check if any of B's min or max points are within A's min and max points
    bool aXOverlapsB = (aMinX < bMinX && bMinX < aMaxX) || (aMinX < bMaxX && bMaxX < aMaxX);
    bool aYOverlapsB = (aMinY < bMinY && bMinY < aMaxY) || (aMinY < bMaxY && bMaxY < aMaxY);

    //if (((aMinX <= bMinX && bMinX <= aMaxX) || (aMinX <= bMaxX && bMaxX <= aMaxX)) && ((aMinY <= bMinY && bMinY <= aMaxY) || (aMaxY <= bMaxY && bMaxY <= aMaxY)))

    if (aXOverlapsB && aYOverlapsB)
    {
        distanceX = A->position.x - B->position.x;
        distanceY = A->position.y - B->position.y;

        if (distanceX > distanceY)
        {
            A->position.x += distanceX * 0.5f;
            B->position.x -= distanceX * 0.5f;
        }
        else
        {
            float overlap;
            if (aMinY < bMinY)
            {
                overlap = aMaxY - bMinY;
                A->position.y -= overlap;

            }
            else
            {
                overlap = bMaxY - aMinY;
                B->position.y -= overlap;
            }
        }

		A->netForce = { 0,0 };
		B->netForce = { 0,0 };

        return true;
    }

    return false;
}

//bool CheckCircleHalfspaceOverlap(PhysicsCircle* Circle, PhysicsHalfSpace* Halfspace)
//{
//    // Get Displacement
//    Vector2 displacement = Circle->position - Halfspace->position;
//
//    // Take the dot product
//    float dot = Vector2DotProduct(displacement, Halfspace->GetNormal());
//    Vector2 vectorProjection = Halfspace->GetNormal() * dot;
//
//    float distance = Vector2Length(displacement);
//
//    DrawLineEx(Circle->position, Circle->position - vectorProjection, 3, GRAY);
//
//    //Vector2 midpoint = vectorProjection * displacement * 0.5f;
//    Vector2 midpoint = Circle->position - vectorProjection * 0.5f;
//    DrawText(TextFormat("D: %6", dot), midpoint.x, midpoint.y, 20, GRAY);
//
//    float overlap = Circle->radius - dot;
//
//    return dot < Circle->radius && dot > -Circle->radius;
//}

bool CircleHalfspaceCollisionResponse(PhysicsCircle* Circle, PhysicsHalfSpace* Halfspace)
{
    // Get Displacement
    Vector2 displacement = Circle->position - Halfspace->position;

    // Take the dot product
    float dot = Vector2DotProduct(displacement, Halfspace->GetNormal());
    Vector2 vectorProjection = Halfspace->GetNormal() * dot;

    float distance = Vector2Length(displacement);

    DrawLineEx(Circle->position, Circle->position - vectorProjection, 3, GRAY);

    //Vector2 midpoint = vectorProjection * displacement * 0.5f;
    Vector2 midpoint = Circle->position - vectorProjection * 0.5f;
    DrawText(TextFormat("D: %6", dot), midpoint.x, midpoint.y, 20, GRAY);

    float overlap = Circle->radius - dot;

    if (overlap > 0)
    {
        // Move
        Vector2 mtv = Halfspace->GetNormal() * overlap;
        Circle->position += mtv;

        // Get Gravity
        Vector2 Fgravity = world.accelerationGravity * Circle->mass;

        // Apply Normal Force
        Vector2 FgPerpindicular = Halfspace->GetNormal() * Vector2DotProduct(Fgravity, Halfspace->GetNormal());
        Vector2 Fnormal = FgPerpindicular * -1;
        //DrawLine(Circle->position.x, Circle->position.y, Circle->position.x + Fnormal.x, Circle->position.y + Fnormal.y, GREEN);

        // Friction
        float coefficientOfFriction = Clamp(Circle->grippiness * Halfspace->grippiness, 0.0f, 1.0f);
        float FfrictionMagnitude = coefficientOfFriction * Vector2Length(Fnormal);

        Vector2 FgParallel = Fgravity - FgPerpindicular;
        Vector2 FfrictionDirection = Vector2Normalize(FgParallel * -1);

        float FgParallelMagnitude = Vector2Length(FgParallel);
        float clampedFriction = min(FfrictionMagnitude, FgParallelMagnitude);

        Vector2 Ffriction = FfrictionDirection * clampedFriction;
        float frictionForceLength = Vector2Length(Ffriction);

        Circle->netForce += Ffriction;

        //Bouncing
        float closingVelocity1D = Vector2DotProduct(Circle->velocity, Halfspace->GetNormal());

        if (closingVelocity1D >= 0) return true;

        float restitution = Circle->bounciness * Halfspace->bounciness;
        Circle->velocity += Halfspace->GetNormal() * closingVelocity1D * -(1.0f + restitution);

        return true;
    }

    return false;
}

void CheckCollisions()
{
    /*for (int i = 0; i < world.objects.size(); i++)
    {
        world.objects[i]->color = GREEN;
    }*/

    for (int i = 0; i < world.objects.size(); i++)
    {
        for (int j = i + 1; j < world.objects.size(); j++)
        {
            PhysicsObject* ObjectA = world.objects[i];
            PhysicsObject* ObjectB = world.objects[j];

            PhysicsShape ShapeA = ObjectA->GetShape();
            PhysicsShape ShapeB = ObjectB->GetShape();

            bool isOverlapping = false;

            //Check which collision function to use
            // Circle - Circle Collision
            if (ShapeA == CIRCLE && ShapeB == CIRCLE)
            {
                isOverlapping = CircleCircleCollisionResponse((PhysicsCircle*)ObjectA, (PhysicsCircle*)ObjectB);
            }
            // Circle - Halfspace Collision
            else if (ShapeA == CIRCLE && ShapeB == HALFSPACE)
            {
                isOverlapping = CircleHalfspaceCollisionResponse((PhysicsCircle*)ObjectA, (PhysicsHalfSpace*)ObjectB);
            }
            else if (ShapeA == HALFSPACE && ShapeB == CIRCLE)
            {
                isOverlapping = CircleHalfspaceCollisionResponse((PhysicsCircle*)ObjectB, (PhysicsHalfSpace*)ObjectA);
            }
            else if (ShapeA == AABB && ShapeB == AABB)
            {
                isOverlapping = BoxBoxOverlap((PhysicsBox*)ObjectA, (PhysicsBox*)ObjectB);
            }

            /*if (isOverlapping)
            {
                ObjectA->color = RED;
                ObjectB->color = RED;
            }*/
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
            // take the object to be deleted
            std::vector<PhysicsObject*>::iterator iterator = (world.objects.begin() + i);
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

    //if (IsKeyPressed(KEY_SPACE))
    //{
    //    PhysicsCircle* newObject = new PhysicsCircle();
    //    newObject->position = position;
    //    newObject->velocity = { speed * (float)cos(angle * DEG2RAD), -speed * (float)sin(angle * DEG2RAD) };

    //    // rand() % N produces random number from 0 to N - 1
    //    newObject->radius = (rand() & 11) + 5; // between 5 to 15
    //    newObject->bounciness = bounciness;

    //    world.add(newObject);
    //}

    const float slingshot_radius = 10.0f;
    const Vector2 slingshot_position = position;
    Vector2 bird_position = slingshot_position;
    SlingshotState slingshot_state = SLING_IDLE;
    Vector2 mouse_position = GetMousePosition();

    Vector2 startPos = {200, 600};
	Vector2 groundPos = { 200, 700 };
    Vector2 velocity = { speed * cos(angle * DEG2RAD), -speed * sin(angle * DEG2RAD) };

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
		slingshot_state = SLING_DRAG;
    }

    if (slingshot_state == SLING_DRAG)
    {
		bool isMouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

        if (isMouseDown)
        {
            bird_position = mouse_position;
            DrawLineEx(startPos, bird_position, 5, BLACK);
        }
        if (!isMouseDown || IsKeyPressed(KEY_SPACE))
        {
            // Launch bird (TODO -- launch bird based on direction of sling rather than hard-coded velocity)
            PhysicsCircle* bird = new PhysicsCircle();
            bird->position = mouse_position;
            bird->velocity = startPos - mouse_position; //Vector2Rotate(Vector2UnitX, -45.0f * DEG2RAD) * 100.0f;
            bird->radius = 10;
			bird->color = YELLOW;
            world.add(bird);

            slingshot_state = SLING_IDLE;
        }

        if (slingshot_state == SLING_IDLE)
            DrawCircleV(slingshot_position, slingshot_radius, GREEN);
        else
            DrawCircleV(bird_position, slingshot_radius, ORANGE);
    }

	DrawLineEx(startPos, groundPos, 10, ORANGE);
    

	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        position = GetMousePosition();
    }



    world.Update();
}

void Draw()
{
    BeginDrawing();
    ClearBackground(BLUE);

    GuiSliderBar(Rectangle{ 100, 5, 200, 25 }, "Time", TextFormat("%.2f", time), &time, 0, 600);
    DrawText(timer.c_str(), 25, 130, 30, LIGHTGRAY);

    //GuiSliderBar(Rectangle{ 100, 35, 200, 25 }, "Angle", TextFormat("Angle: %.0f Degrees", angle), &angle, -180, 180);
    //GuiSliderBar(Rectangle{ 100, 65, 200, 25 }, "Speed", TextFormat("Speed: %0.f", speed), &speed, 10, 100);
    //GuiSliderBar(Rectangle{ 100, 95, 200, 25 }, "Gravity", TextFormat("Gravity: %0.f Pixels/Sec^2", world.accelerationGravity.y), &world.accelerationGravity.y, -100, 100);
    //GuiSliderBar(Rectangle{ 200, 125, 200, 25 }, "Bounciness", TextFormat("%.0f", bounciness), &bounciness, 0, 10);


    

    world.Draw();

    // halfspace controls
    //GuiSliderBar(Rectangle{ 200, 155, 300, 25 }, "Halfspace X", TextFormat("%.0f", halfspace.position.x), &halfspace.position.x, 0, GetScreenWidth());
    //GuiSliderBar(Rectangle{ 200, 185, 300, 25 }, "Halfspace Y", TextFormat("%.0f", halfspace.position.y), &halfspace.position.y, 0, GetScreenHeight());
    //GuiSliderBar(Rectangle{ 300, 215, 200, 25 }, "Halfspace Rotation", TextFormat("%.0f", halfspaceRotation), &halfspaceRotation, -180, 180);
    //GuiSliderBar(Rectangle{ 300, 245, 200, 25 }, "Halfspace Grippiness", TextFormat("%.0f", halfspace.grippiness), &halfspace.grippiness, 0, 10);

    halfspace.SetRotationInDegrees(halfspaceRotation);
    halfspace.isStatic = true;
    halfspace2.isStatic = true;

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

    halfspace.position = { 500, 700 };
    halfspace.SetRotationInDegrees(15);
    world.add(&halfspace);

    halfspace2.position = { 400, 700 };
    halfspace2.SetRotationInDegrees(-15);
    //world.add(&halfspace2);

    PhysicsBox* floorBox = new PhysicsBox;
    floorBox->position = { 600, 750 };
    floorBox->size = { 3200, 100 };
    floorBox->isStatic = true;
    floorBox->color = GREEN;
    world.add(floorBox);

    PhysicsBox* box1 = new PhysicsBox;
    box1->position = { 600, 690 };
    box1->size = { 100, 60 };
    box1->color = RED;
    world.add(box1);

    PhysicsBox* box2 = new PhysicsBox;
    box2->position = { 601, 630 };
    box2->size = { 100, 60 };
    box2->color = PURPLE;
    world.add(box2);

    PhysicsBox* box3 = new PhysicsBox;
    box3->position = { 602, 570 };
    box3->size = { 100, 60 };
    box3->color = DARKBLUE;
    world.add(box3);

	PhysicsBox* box4 = new PhysicsBox;
	box4->position = { 753, 510 };
	box4->size = { 300, 60 };
    box4->color = GREEN;
	world.add(box4);

    PhysicsBox* box5 = new PhysicsBox;
    box5->position = { 900, 690 };
    box5->size = { 100, 60 };
    box5->color = RED;
    world.add(box5);

    PhysicsBox* box6 = new PhysicsBox;
    box6->position = { 901, 630 };
    box6->size = { 100, 60 };
    box6->color = PURPLE;
    world.add(box6);

    PhysicsBox* box7 = new PhysicsBox;
    box7->position = { 902, 570 };
    box7->size = { 100, 60 };
    box7->color = DARKBLUE;
    world.add(box7);



    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }

    CloseWindow();
    return 0;
}