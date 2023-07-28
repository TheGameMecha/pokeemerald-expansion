
#if WILD_ROAMING == TRUE

#include "global.h"
#include "global.fieldmap.h"
#include "wild_encounter.h"
#include "wild_roaming.h"
#include "event_object_movement.h"
#include "graphics.h"
#include "constants/event_objects.h"
#include "fieldmap.h"
#include "script.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "battle_setup.h"

EWRAM_DATA static u8 sLocation[2] = {0}; // represents the current location
EWRAM_DATA struct Pokemon gWildPokemonInstances[MAX_ACTIVE_PKMN] = {0};
EWRAM_DATA static u8 wildPokemonObjectEventIds[MAX_ACTIVE_PKMN] = {0};

enum
{
    MAP_GRP, // map group
    MAP_NUM, // map number
};

static void StartWildBattle(u8 index)
{
    gEnemyParty[0] = gWildPokemonInstances[index];
    BattleSetup_StartWildBattle();
}

static u8 GetInverseDirection(u8 direction)
{
    if(direction == DIR_SOUTH)
        return DIR_NORTH;
    else if (direction == DIR_NORTH)
        return DIR_SOUTH;
    else if (direction == DIR_WEST)
        return DIR_EAST;
    else if (direction == DIR_EAST)
        return DIR_WEST;
}

// called from field_control_avatar.c every time we take a step
bool8 CheckForWildPokemonToBattle()
{
    struct ObjectEvent *objectEvent;
    struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    for (u8 i = 0; i < MAX_ACTIVE_PKMN; i++)
    {
        u8 index = wildPokemonObjectEventIds[i];
        if(index == 0) // in this case, index 0 would be the player. We will always collide with ourself...
        {
            return FALSE;
        }
        objectEvent = &gWildPokemonObjects[index];
        u8 collisionA = GetCollisionInDirection(objectEvent, objectEvent->facingDirection);
        u8 collisionB = GetCollisionInDirection(objectEvent, GetInverseDirection(playerObjEvent->movementDirection));
        if(collisionA == COLLISION_WILD_POKEMON || collisionB == COLLISION_OBJECT_EVENT)
        {
            StartWildBattle(i);
            RemoveObjectEvent(objectEvent);
            wildPokemonObjectEventIds[i] = 0;
            return TRUE;
        }
    }

    return FALSE;
}

void TrySetupWildRoamingPokemon()
{
    sLocation[MAP_GRP] = gSaveBlock1Ptr->location.mapGroup;
    sLocation[MAP_NUM] = gSaveBlock1Ptr->location.mapNum;

    CreateWildWalkingMons(gWildPokemonInstances, MAX_ACTIVE_PKMN);

    for (u8 i = 0; i < 1; i++)
    {
        u8 obj = SpawnSpecialObjectEventParameterized(
            OBJ_EVENT_GFX_EXAMPLE,
            MOVEMENT_TYPE_FACE_RIGHT,
            240 - i,
            gSaveBlock1Ptr->pos.x + i + 2 + MAP_OFFSET,
            gSaveBlock1Ptr->pos.y + i + MAP_OFFSET,
            3,
            OBJ_KIND_WILD_POKEMON);
        wildPokemonObjectEventIds[i] = obj;
        struct ObjectEvent *objectEvent;
        objectEvent = &gWildPokemonObjects[obj];
        objectEvent->isPlayer = FALSE;
    }
}

#endif