#ifndef GAME_H
#define GAME_H

#include "box2d/box2d.h"
#include "raylib.h"

#include "include/cc_slist.h"

#include "helpers.h"
#include "settings.h"

#include <assert.h>
#include <time.h>
#include <stdio.h>

enum entityType
{
    STANDARD_WALL_ENTITY,
    BOUNCY_WALL_ENTITY,
    DEATH_WALL_ENTITY,
    WEAPON_PICKUP_ENTITY,
    PROJECTILE_ENTITY,
    DRONE_ENTITY,
};

enum shapeCategory
{
    WALL_SHAPE = 0x1,
    PROJECTILE_SHAPE = 0x2,
    DRONE_SHAPE = 0x4,
};

typedef struct entity
{
    enum entityType type;
    void *entity;
} entity;

typedef struct wallEntity
{
    b2BodyId bodyID;
    b2ShapeId *shapeID;
    b2Vec2 extent;
    bool isFloating;
    enum entityType type;
} wallEntity;

typedef struct weaponPickupEntity
{
    b2BodyId bodyID;
    b2ShapeId *shapeID;
    enum weaponType weapon;
} weaponPickupEntity;

typedef struct droneEntity droneEntity;

typedef struct projectileEntity
{
    b2BodyId bodyID;
    b2ShapeId *shapeID;
    enum weaponType type;
    b2Vec2 lastPos;
    float distance;
    uint8_t bounces;
    uint8_t bouncyWallBounces;
} projectileEntity;

typedef struct droneEntity
{
    b2BodyId bodyID;
    b2ShapeId *shapeID;
    enum weaponType weapon;
    int8_t ammo;
    float weaponCooldown;
    bool shotThisStep;
    uint16_t triggerHeldSteps;
    b2Vec2 lastAim;
} droneEntity;

wallEntity *createWall(const b2WorldId worldID, const float posX, const float posY, const float width, const float height, const enum entityType type, bool floating)
{
    assert(type != DRONE_ENTITY);
    assert(type != PROJECTILE_ENTITY);

    b2BodyDef wallBodyDef = b2DefaultBodyDef();
    wallBodyDef.position = createb2Vec(posX, posY);
    if (floating)
    {
        wallBodyDef.type = b2_dynamicBody;
    }
    b2BodyId wallBodyID = b2CreateBody(worldID, &wallBodyDef);
    b2Vec2 extent = {.x = width / 2.0f, .y = height / 2.0f};
    b2Polygon bouncyWall = b2MakeBox(extent.x, extent.y);
    b2ShapeDef wallShapeDef = b2DefaultShapeDef();
    wallShapeDef.density = WALL_DENSITY;
    wallShapeDef.restitution = 0.0f;
    if (type == BOUNCY_WALL_ENTITY)
    {
        wallShapeDef.restitution = BOUNCY_WALL_RESTITUTION;
    }
    wallShapeDef.filter.categoryBits = WALL_SHAPE;
    wallShapeDef.filter.maskBits = WALL_SHAPE | PROJECTILE_SHAPE | DRONE_SHAPE;

    wallEntity *wall = (wallEntity *)calloc(1, sizeof(wallEntity));
    wall->bodyID = wallBodyID;
    wall->shapeID = (b2ShapeId *)calloc(1, sizeof(b2ShapeId));
    wall->extent = extent;
    wall->isFloating = floating;
    wall->type = type;

    entity *e = (entity *)calloc(1, sizeof(entity));
    e->type = type;
    e->entity = wall;

    wallShapeDef.userData = e;
    *wall->shapeID = b2CreatePolygonShape(wallBodyID, &wallShapeDef, &bouncyWall);

    return wall;
}

weaponPickupEntity *createWeaponPickup(const b2WorldId worldID, const float posX, const float posY, const enum weaponType type)
{
    assert(type != STANDARD_WEAPON);

    b2BodyDef pickupBodyDef = b2DefaultBodyDef();
    pickupBodyDef.position = createb2Vec(posX, posY);
    b2BodyId pickupBodyID = b2CreateBody(worldID, &pickupBodyDef);
    b2ShapeDef pickupShapeDef = b2DefaultShapeDef();
    pickupShapeDef.filter.categoryBits = WEAPON_PICKUP_ENTITY;
    pickupShapeDef.filter.maskBits = DRONE_SHAPE;
    pickupShapeDef.isSensor = true;
    b2Circle pickupCircle = {.center = {.x = 0.0f, .y = 0.0f}, .radius = 0.9f};

    weaponPickupEntity *pickup = (weaponPickupEntity *)calloc(1, sizeof(weaponPickupEntity));
    pickup->bodyID = pickupBodyID;
    pickup->shapeID = (b2ShapeId *)calloc(1, sizeof(b2ShapeId));
    pickup->weapon = type;

    entity *e = (entity *)calloc(1, sizeof(entity));
    e->type = WEAPON_PICKUP_ENTITY;
    e->entity = pickup;

    // TODO: make square
    pickupShapeDef.userData = e;
    *pickup->shapeID = b2CreateCircleShape(pickupBodyID, &pickupShapeDef, &pickupCircle);

    return pickup;
}

void destroyWeaponPickup(const b2WorldId worldID, weaponPickupEntity *pickup)
{
    assert(pickup != NULL);

    entity *e = (entity *)b2Shape_GetUserData(*pickup->shapeID);
    free(e);

    b2DestroyBody(pickup->bodyID);
    free(pickup->shapeID);
    free(pickup);
}

droneEntity *createDrone(const b2WorldId worldID, const float posX, const float posY)
{
    b2BodyDef droneBodyDef = b2DefaultBodyDef();
    droneBodyDef.type = b2_dynamicBody;
    droneBodyDef.position = createb2Vec(posX, posY);
    b2BodyId droneBodyID = b2CreateBody(worldID, &droneBodyDef);
    b2ShapeDef droneShapeDef = b2DefaultShapeDef();
    droneShapeDef.density = DRONE_DENSITY;
    droneShapeDef.friction = 0.0f;
    droneShapeDef.restitution = 0.3f;
    droneShapeDef.filter.categoryBits = DRONE_SHAPE;
    droneShapeDef.filter.maskBits = WALL_SHAPE | PROJECTILE_SHAPE | DRONE_SHAPE;
    droneShapeDef.enableContactEvents = true;
    droneShapeDef.enableSensorEvents = true;
    b2Circle droneCircle = {.center = {.x = 0.0f, .y = 0.0f}, .radius = DRONE_RADIUS};

    droneEntity *drone = (droneEntity *)calloc(1, sizeof(droneEntity));
    drone->bodyID = droneBodyID;
    drone->shapeID = (b2ShapeId *)calloc(1, sizeof(b2ShapeId));
    drone->weapon = DRONE_DEFAULT_WEAPON;
    drone->ammo = weaponAmmo(drone->weapon);
    drone->shotThisStep = false;
    drone->lastAim = (b2Vec2){.x = 0.0f, .y = -1.0f};

    entity *e = (entity *)calloc(1, sizeof(entity));
    e->type = DRONE_ENTITY;
    e->entity = drone;

    droneShapeDef.userData = e;
    *drone->shapeID = b2CreateCircleShape(droneBodyID, &droneShapeDef, &droneCircle);

    return drone;
}

void destroyDrone(droneEntity *drone)
{
    assert(drone != NULL);

    entity *e = (entity *)b2Shape_GetUserData(*drone->shapeID);
    free(e);

    b2DestroyBody(drone->bodyID);
    free(drone->shapeID);
    free(drone);
}

void droneMove(const droneEntity *drone, const b2Vec2 direction)
{
    assert(drone != NULL);
    ASSERT_VEC(direction, -1.0f, 1.0f);

    b2Vec2 force = b2MulSV(DRONE_MOVE_MAGNITUDE, direction);
    b2Body_ApplyForceToCenter(drone->bodyID, force, true);
}

void createProjectile(const b2WorldId worldID, CC_SList *projectiles, droneEntity *drone, const b2Vec2 normAim, const b2Vec2 aimRecoil)
{
    assert(drone != NULL);
    ASSERT_VEC(normAim, -1.0f, 1.0f);

    b2BodyDef projectileBodyDef = b2DefaultBodyDef();
    projectileBodyDef.type = b2_dynamicBody;
    // TODO: make fixed rotation?
    projectileBodyDef.rotation = b2MakeRot(b2Atan2(normAim.x, normAim.y));
    b2Vec2 dronePos = b2Body_GetPosition(drone->bodyID);
    float radius = weaponRadius(drone->weapon);
    projectileBodyDef.position = b2Add(dronePos, b2MulSV(1.0f + radius * 1.5f, normAim));
    b2BodyId projectileBodyID = b2CreateBody(worldID, &projectileBodyDef);
    b2ShapeDef projectileShapeDef = b2DefaultShapeDef();
    projectileShapeDef.enableContactEvents = true;
    projectileShapeDef.density = weaponDensity(drone->weapon);
    projectileShapeDef.friction = 0.0f;
    projectileShapeDef.restitution = 1.0f;
    projectileShapeDef.filter.categoryBits = PROJECTILE_SHAPE;
    projectileShapeDef.filter.maskBits = WALL_SHAPE | PROJECTILE_SHAPE | DRONE_SHAPE;
    b2Circle projectileCircle = {.center = {.x = 0.0f, .y = 0.0f}, .radius = radius};

    b2ShapeId *projectileShapeID = (b2ShapeId *)calloc(1, sizeof(b2ShapeId));
    *projectileShapeID = b2CreateCircleShape(projectileBodyID, &projectileShapeDef, &projectileCircle);

    // add lateral drone velocity to projectile
    b2Vec2 droneVel = b2Body_GetLinearVelocity(drone->bodyID);
    b2Vec2 forwardVel = b2MulSV(b2Dot(droneVel, normAim), normAim);
    b2Vec2 lateralVel = b2Sub(droneVel, forwardVel);
    lateralVel = b2MulSV(weaponDroneMoveCoef(drone->weapon), lateralVel);
    b2Vec2 fire = b2MulAdd(lateralVel, weaponFire(drone->weapon), normAim);
    fire = b2Add(fire, aimRecoil);
    b2Body_ApplyLinearImpulseToCenter(projectileBodyID, fire, true);

    projectileEntity *projectile = (projectileEntity *)calloc(1, sizeof(projectileEntity));
    projectile->bodyID = projectileBodyID;
    projectile->shapeID = projectileShapeID;
    projectile->type = drone->weapon;
    projectile->lastPos = projectileBodyDef.position;
    cc_slist_add(projectiles, projectile);

    entity *e = (entity *)calloc(1, sizeof(entity));
    e->type = PROJECTILE_ENTITY;
    e->entity = projectile;

    b2Shape_SetUserData(*projectile->shapeID, e);
}

void destroyProjectile(CC_SList *projectiles, projectileEntity *projectile, bool full)
{
    assert(projectile != NULL);

    entity *e = (entity *)b2Shape_GetUserData(*projectile->shapeID);
    free(e);

    if (full)
    {
        cc_slist_remove(projectiles, projectile, NULL);
        b2DestroyBody(projectile->bodyID);
    }

    free(projectile->shapeID);
    free(projectile);
}

void destroyAllProjectiles(CC_SList *projectiles)
{
    for (SNode *cur = projectiles->head; cur != NULL; cur = cur->next)
    {
        projectileEntity *projectile = (projectileEntity *)cur->data;
        destroyProjectile(projectiles, projectile, false);
    }
}

void droneChangeWeapon(droneEntity *drone, enum weaponType weapon)
{
    drone->weapon = weapon;
    drone->ammo = weaponAmmo(weapon);
    drone->weaponCooldown = 0.0f;
    drone->triggerHeldSteps = 0;
}

void droneShoot(const b2WorldId worldID, CC_SList *projectiles, droneEntity *drone, const b2Vec2 aim)
{
    assert(drone != NULL);
    assert(drone->ammo != 0);

    drone->shotThisStep = true;
    drone->triggerHeldSteps++;
    if (drone->weaponCooldown != 0.0f)
    {
        return;
    }
    if (drone->weapon != STANDARD_WEAPON && drone->ammo != INFINITE_AMMO)
    {
        drone->ammo--;
    }
    drone->weaponCooldown = weaponCoolDown(drone->weapon);

    b2Vec2 normAim = drone->lastAim;
    if (!b2VecEqual(aim, b2Vec2_zero))
    {
        normAim = b2Normalize(aim);
    }
    b2Vec2 aimRecoil = weaponAimRecoil(drone->weapon);
    b2Vec2 recoil = b2MulAdd(aimRecoil, -weaponRecoil(drone->weapon), normAim);
    b2Body_ApplyLinearImpulseToCenter(drone->bodyID, recoil, true);

    createProjectile(worldID, projectiles, drone, normAim, aimRecoil);

    if (drone->ammo == 0)
    {
        droneChangeWeapon(drone, STANDARD_WEAPON);
    }
}

void droneStep(droneEntity *drone, const float frameTime)
{
    assert(drone != NULL);

    drone->weaponCooldown = b2MaxFloat(drone->weaponCooldown - frameTime, 0.0f);
    if (!drone->shotThisStep)
    {
        drone->triggerHeldSteps = 0;
    }
    else
    {
        drone->shotThisStep = false;
    }
    assert(drone->shotThisStep == false);

    b2Vec2 velocity = b2Body_GetLinearVelocity(drone->bodyID);
    if (!b2VecEqual(velocity, b2Vec2_zero))
    {
        b2Vec2 dragForce = {
            .x = -DRONE_DRAG_COEFFICIENT * velocity.x,
            .y = -DRONE_DRAG_COEFFICIENT * velocity.y,
        };
        b2Body_ApplyForceToCenter(drone->bodyID, dragForce, false);
    }
}

void projectilesStep(CC_SList *projectiles)
{
    CC_SListIter iter;
    cc_slist_iter_init(&iter, projectiles);
    projectileEntity *projectile;
    while (cc_slist_iter_next(&iter, (void **)&projectile) != CC_ITER_END)
    {
        b2Vec2 pos = b2Body_GetPosition(projectile->bodyID);
        b2Vec2 distance = b2Sub(pos, projectile->lastPos);
        projectile->distance += b2Length(distance);
        if (projectile->distance >= weaponMaxDistance(projectile->type))
        {
            cc_slist_iter_remove(&iter, NULL);
            destroyProjectile(projectiles, projectile, true);
            continue;
        }

        projectile->lastPos = pos;
    }
}

bool handleProjectileBeginContact(CC_SList *projectiles, const entity *e1, const entity *e2)
{
    assert(e1 != NULL);
    assert(e2 != NULL);

    projectileEntity *projectile = (projectileEntity *)e1->entity;
    if (e2->type != BOUNCY_WALL_ENTITY)
    {
        projectile->bounces++;
    }
    else
    {
        projectile->bouncyWallBounces++;
    }
    uint8_t maxBounces = weaponBounce(projectile->type);
    if (projectile->bounces == maxBounces || projectile->bouncyWallBounces == maxBounces * 3)
    {
        destroyProjectile(projectiles, projectile, true);
        return true;
    }

    return false;
}

bool handleProjectileEndContact(CC_SList *projectiles, const entity *p, const entity *e)
{
    assert(p != NULL);

    projectileEntity *projectile = (projectileEntity *)p->entity;
    // e (shape B in the collision) will be NULL if it's another
    // projectile that was just destroyed
    if (e == NULL || e->type != BOUNCY_WALL_ENTITY)
    {
        projectile->bounces++;
    }
    else
    {
        projectile->bouncyWallBounces++;
    }
    uint8_t maxBounces = weaponBounce(projectile->type);
    if (projectile->bounces == maxBounces || projectile->bouncyWallBounces == maxBounces * 3)
    {
        destroyProjectile(projectiles, projectile, true);
        return true;
    }

    b2Vec2 velocity = b2Body_GetLinearVelocity(projectile->bodyID);
    b2Vec2 newVel = b2MulSV(weaponFire(projectile->type) * weaponInvMass(projectile->type), b2Normalize(velocity));
    b2Body_SetLinearVelocity(projectile->bodyID, newVel);

    return false;
}

void handleContactEvents(const b2WorldId worldID, CC_SList *projectiles)
{
    b2ContactEvents events = b2World_GetContactEvents(worldID);
    for (int i = 0; i < events.endCount; ++i)
    {
        const b2ContactEndTouchEvent *event = events.endEvents + i;
        entity *e1 = NULL;
        entity *e2 = NULL;

        if (b2Shape_IsValid(event->shapeIdA))
        {
            e1 = (entity *)b2Shape_GetUserData(event->shapeIdA);
            assert(e1 != NULL);
        }
        if (b2Shape_IsValid(event->shapeIdB))
        {
            e2 = (entity *)b2Shape_GetUserData(event->shapeIdB);
            assert(e2 != NULL);
        }

        if (e1 != NULL && e1->type == PROJECTILE_ENTITY)
        {
            if (handleProjectileEndContact(projectiles, e1, e2))
            {
                e1 = NULL;
            }
        }
        if (e2 != NULL && e2->type == PROJECTILE_ENTITY)
        {
            handleProjectileEndContact(projectiles, e2, e1);
        }
    }
}

void handleDroneBeginTouch(const b2WorldId worldID, const entity *s, entity *v)
{
    weaponPickupEntity *pickup = (weaponPickupEntity *)s->entity;
    droneEntity *drone = (droneEntity *)v->entity;
    droneChangeWeapon(drone, pickup->weapon);
    // TODO: disable/hide for a bit, then enable in a new spot instead of destroying
    // destroyWeaponPickup(worldID, pickup);
}

void handleSensorEvents(const b2WorldId worldID)
{
    b2SensorEvents events = b2World_GetSensorEvents(worldID);
    for (int i = 0; i < events.beginCount; ++i)
    {
        const b2SensorBeginTouchEvent *event = events.beginEvents + i;
        if (!b2Shape_IsValid(event->sensorShapeId))
        {
            ERROR("could not find sensor shape for begin touch event");
        }
        entity *s = (entity *)b2Shape_GetUserData(event->sensorShapeId);
        assert(s != NULL);
        assert(s->type == WEAPON_PICKUP_ENTITY);

        if (!b2Shape_IsValid(event->visitorShapeId))
        {
            ERROR("could not find visitor shape for begin touch event");
        }
        entity *v = (entity *)b2Shape_GetUserData(event->visitorShapeId);
        assert(v != NULL);
        assert(v->type == DRONE_ENTITY);

        handleDroneBeginTouch(worldID, s, v);
    }
}

#endif