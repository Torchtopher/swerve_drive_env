#ifndef RENDER_H
#define RENDER_H

#include "helpers.h"
#include "game.h"

const Color barolo = {.r = 165, .b = 8, .g = 37, .a = 255};

const float halfDroneRadius = DRONE_RADIUS / 2.0f;
const float aimGuideWidthExtent = 2.0f * DRONE_RADIUS;
const float aimGuideHeightExtent = 0.1f * DRONE_RADIUS;

void renderWall(const wallEntity *wall)
{
    Color color = {0};
    switch (wall->type)
    {
    case STANDARD_WALL_ENTITY:
        color = BLUE;
        break;
    case BOUNCY_WALL_ENTITY:
        color = YELLOW;
        break;
    case DEATH_WALL_ENTITY:
        color = RED;
        break;
    default:
        ERRORF("unknown wall type %d", wall->type);
    }

    b2Vec2 wallPos = b2Body_GetPosition(wall->bodyID);
    Vector2 pos = b2VecToRayVec(wallPos);
    Rectangle rec = {
        .x = pos.x - wall->extent.x * scale,
        .y = pos.y - wall->extent.y * scale,
        .width = wall->extent.x * scale * 2.0f,
        .height = wall->extent.y * scale * 2.0f,
    };
    DrawRectanglePro(rec, (Vector2){.x = 0.0f, .y = 0.0f}, 0.0f, color);
}

void renderWeaponPickup(const weaponPickupEntity *pickup)
{
    b2Vec2 pos = b2Body_GetPosition(pickup->bodyID);
    DrawCircleV(b2VecToRayVec(pos), DRONE_RADIUS * scale, LIME);
}

void renderDrone(const droneEntity *drone, b2Vec2 move, b2Vec2 aim)
{
    if (b2VecEqual(aim, b2Vec2_zero))
    {
        aim = drone->lastAim;
    }

    b2Vec2 pos = b2Body_GetPosition(drone->bodyID);

    if (!b2VecEqual(move, b2Vec2_zero))
    {
        float moveMagnitude = b2Length(move);
        float moveRot = RAD2DEG * b2Rot_GetAngle(b2MakeRot(b2Atan2(-move.y, -move.x)));
        Rectangle moveGuide = {
            .x = (pos.x) * scale,
            .y = (pos.y) * scale,
            .width = ((halfDroneRadius * moveMagnitude) + halfDroneRadius) * scale * 2.0f,
            .height = halfDroneRadius * scale * 2.0f,
        };
        DrawRectanglePro(moveGuide, (Vector2){.x = 0.0f, .y = 0.5f * scale}, moveRot, RED);
    }

    float aimRot = RAD2DEG * b2Rot_GetAngle(b2MakeRot(b2Atan2(aim.y, aim.x)));
    Rectangle aimGuide = {
        .x = (pos.x) * scale,
        .y = (pos.y) * scale,
        .width = aimGuideWidthExtent * scale * 2.0f,
        .height = aimGuideHeightExtent * scale * 2.0f,
    };
    DrawRectanglePro(aimGuide, (Vector2){.x = 0.0f, .y = aimGuideHeightExtent * scale}, aimRot, RED);

    DrawCircleV(b2VecToRayVec(pos), DRONE_RADIUS * scale, barolo);
}

void renderProjectiles(CC_SList *projectiles)
{
    for (SNode *cur = projectiles->head; cur != NULL; cur = cur->next)
    {
        projectileEntity *projectile = (projectileEntity *)cur->data;
        b2Vec2 projectilePos = b2Body_GetPosition(projectile->bodyID);
        DrawCircleV(b2VecToRayVec(projectilePos), scale * weaponRadius(projectile->type), PURPLE);
    }
}

#endif