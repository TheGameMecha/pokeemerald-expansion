
#if WILD_ROAMING == TRUE

#include "global.h"
#include "global.fieldmap.h"
#include "wild_encounter.h"
#include "wild_roaming.h"
#include "event_object_movement.h"
#include "graphics.h"
#include "fieldmap.h"
#include "field_effect.h"
#include "script.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "battle_setup.h"
#include "constants/event_objects.h"
#include "constants/trainer_types.h"
#include "constants/field_effects.h"
#include "task.h"
#include "palette.h"
#include "sound.h"
#include "malloc.h"
#include "trainer_see.h"
#include "constants/songs.h"

EWRAM_DATA static u8 sLocation[2] = {0}; // represents the current location
EWRAM_DATA struct Pokemon gWildPokemonInstances[MAX_ACTIVE_PKMN] = {0};
EWRAM_DATA static u8 wildPokemonObjectEventIds[MAX_ACTIVE_PKMN] = {0};
EWRAM_DATA static u8 currentWildMonIndex;
EWRAM_DATA static struct WildBattleSetup *sWildBattle = NULL;

struct WildBattleSetup 
{
    u16 timer;
    u8 state;
};

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

static void CB2_WildRoamingStartBattle(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
    MapMusicMain();
}

static void Task_StartWildBattle(u8 taskId)
{   
    struct ObjectEvent *objectEvent;
    objectEvent = &gWildPokemonObjects[currentWildMonIndex];
    sWildBattle->timer++;
    switch (sWildBattle->state)
    {   
    case 0:
        struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
        u8 direction = GetFaceDirectionMovementAction(GetInverseDirection(playerObjEvent->facingDirection));
        ObjectEventSetHeldMovement(objectEvent, direction);
        sWildBattle->timer = 0;
        sWildBattle->state++;
        break;
    case 1:
        if (sWildBattle->timer > 30)
        {
            sWildBattle->timer = 0;
            sWildBattle->state++;
        }
        break;
    case 2:
        InitTrainerApproachTask(objectEvent, 1);
        PlaySE(SE_PIN);
        DoTrainerApproach();
        sWildBattle->timer = 0;
        sWildBattle->state++;
        break;
    case 3:
        if (FieldEffectActiveListContains(FLDEFF_EXCLAMATION_MARK_ICON) == FALSE)
        {
            sWildBattle->timer = 0;
            sWildBattle->state++;
        }
        break;
    case 4:
        StartWildBattle(currentWildMonIndex);
        wildPokemonObjectEventIds[currentWildMonIndex] = 0;
        DestroyTask(taskId);
        break;
    }
}

// called from field_control_avatar.c every time we take a step
bool8 CheckForWildPokemonToBattle()
{
    struct ObjectEvent *objectEvent;
    struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    for (u8 i = 0; i < 2; i++)
    {
        u8 index = wildPokemonObjectEventIds[i];
        // if(index == 0) // in this case, index 0 would be the player. We will always collide with ourself...
        // {
        //     return FALSE;
        // }
        objectEvent = &gWildPokemonObjects[index];

        s16 x, y;
        // Check collision from the pokemon's front
        u8 direction = objectEvent->facingDirection;
        x = objectEvent->currentCoords.x;
        y = objectEvent->currentCoords.y;
        MoveCoords(direction, &x, &y);
        u8 collision = GetCollisionAtCoords(objectEvent, x, y, direction);

        // Check collision from the player's direction
        direction = GetInverseDirection(playerObjEvent->movementDirection);
        x = objectEvent->currentCoords.x;
        y = objectEvent->currentCoords.y;
        MoveCoords(direction, &x, &y);
        u8 collisionB = GetCollisionAtCoords(objectEvent, x, y, direction);
        if(collision == COLLISION_OBJECT_EVENT || collisionB == COLLISION_OBJECT_EVENT)
        {
            currentWildMonIndex = i;
            sWildBattle = AllocZeroed(sizeof(*sWildBattle));
            SetMainCallback2(CB2_WildRoamingStartBattle);
            CreateTask(Task_StartWildBattle, 0);
            return TRUE;
        }
    }

    return FALSE;
}

static bool8 IsMapCoordOccupied(s16 x, s16 y)
{
    x+=MAP_OFFSET;
    y+=MAP_OFFSET;

    DebugPrintf("Testing Coords: %d, %d", x,y);
    struct ObjectEvent *objectEvent;
    for (u8 i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        objectEvent = &gObjectEvents[i];
        DebugPrintf("Checking Against Coords: %d, %d", objectEvent->currentCoords.x,objectEvent->currentCoords.y);
        if(objectEvent->currentCoords.x == x && objectEvent->currentCoords.y == y)
            return TRUE;
    }

    for (u8 i = 0; i < MAX_ACTIVE_PKMN; i++)
    {
        objectEvent = &gWildPokemonObjects[i];
        DebugPrintf("Checking Against Coords: %d, %d", objectEvent->currentCoords.x,objectEvent->currentCoords.y);
        if(objectEvent->currentCoords.x == x && objectEvent->currentCoords.y == y)
            return TRUE;
    }

    return FALSE;

}

static void FindMetatileIdMapCoords(s16 *x, s16 *y, u16 metatileId)
{
    s16 i, j;
    const struct MapLayout *mapLayout = gMapHeader.mapLayout;

    for (j = 0; j < mapLayout->height; j++)
    {
        for (i = 0; i < mapLayout->width; i++)
        {
            if ((mapLayout->map[j * mapLayout->width + i] & MAPGRID_METATILE_ID_MASK) == metatileId)
            {
                if(IsMapCoordOccupied(i, j) == FALSE)
                {
                    *x = i;
                    *y = j;
                    return;
                }
            }
        }
    }
}

static struct Coords16 FindSpawnLocation()
{
    struct Coords16 location;
    location.x = gSaveBlock1Ptr->pos.x;
    location.y = gSaveBlock1Ptr->pos.y;

    for(u8 i = 0; i < WILD_VALID_METATILES; i++)
    {
        FindMetatileIdMapCoords(&location.x, &location.y, sValidSpawnMetatiles[i]);
    }

    return location;
}

void TrySetupWildRoamingPokemon()
{
    sLocation[MAP_GRP] = gSaveBlock1Ptr->location.mapGroup;
    sLocation[MAP_NUM] = gSaveBlock1Ptr->location.mapNum;

    CreateWildWalkingMons(gWildPokemonInstances, MAX_ACTIVE_PKMN);

    for (u8 i = 0; i < 2; i++)
    {
        u32 species = GetMonData(&gWildPokemonInstances[i], MON_DATA_SPECIES);
        struct ObjectEventTemplate wildPokemonObjEventTemplate = sWildPokemonObjectEventTemplates[species];
        u8 objectEventId;
        struct ObjectEvent *objectEvent;

        struct Coords16 spawnLocation = FindSpawnLocation();

        wildPokemonObjEventTemplate.localId =  240 - i;
        wildPokemonObjEventTemplate.x = spawnLocation.x; //gSaveBlock1Ptr->pos.x + i + 2;
        wildPokemonObjEventTemplate.y =  spawnLocation.y; //gSaveBlock1Ptr->pos.y + i;
        wildPokemonObjEventTemplate.elevation = 3;
        wildPokemonObjEventTemplate.flagId = 0;
        wildPokemonObjEventTemplate.kind = OBJ_KIND_WILD_POKEMON;
        objectEventId = SpawnSpecialObjectEvent(&wildPokemonObjEventTemplate);

        wildPokemonObjectEventIds[i] = objectEventId;

        objectEvent = &gWildPokemonObjects[objectEventId];
        objectEvent->isPlayer = FALSE;
    }
}

#endif