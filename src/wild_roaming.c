
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
#include "task.h"
#include "palette.h"
#include "sound.h"
#include "malloc.h"
#include "trainer_see.h"
#include "random.h"
#include "constants/event_objects.h"
#include "constants/trainer_types.h"
#include "constants/field_effects.h"
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
    u8 initiator; // 0 for player, 1 for pokemon
};

enum
{
    MAP_GRP, // map group
    MAP_NUM, // map number
};

static void ResetWildPokemonInstances()
{
    struct Pokemon empty;
    for(u8 i = 0; i < ARRAY_COUNT(gWildPokemonInstances); i++)
    {
        gWildPokemonInstances[i] = empty;
    }
}

static void StartWildBattle(u8 index)
{
    gEnemyParty[0] = gWildPokemonInstances[index];
    BattleSetup_StartWildBattle();
    numActivePokemon = 0;
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
    struct ObjectEvent *pokemonObj;
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];
    pokemonObj = &gWildPokemonObjects[currentWildMonIndex];
    sWildBattle->timer++;
    switch (sWildBattle->state)
    {   
    case 0:
        ClearObjectEventMovement(pokemonObj, &gSprites[pokemonObj->spriteId]);

        if(sWildBattle->initiator == 0)
            ObjectEventSetHeldMovement(pokemonObj, GetFaceDirectionMovementAction(GetOppositeDirection(playerObj->facingDirection)));
        else
            ObjectEventSetHeldMovement(playerObj, GetFaceDirectionMovementAction(GetOppositeDirection(pokemonObj->facingDirection)));
        sWildBattle->timer = 0;
        sWildBattle->state++;
        break;
    case 1:
        PlaySE(SE_PIN);
        DoTrainerApproach();

        sWildBattle->timer = 0;
        sWildBattle->state++;
        break;
    case 2:
        if (FieldEffectActiveListContains(FLDEFF_EXCLAMATION_MARK_ICON) == FALSE)
        {
            sWildBattle->timer = 0;
            sWildBattle->state++;
        }
        break;
    case 3:
        StartWildBattle(currentWildMonIndex);
        wildPokemonObjectEventIds[currentWildMonIndex] = 0;
        DestroyTask(taskId);
        break;
    }
}

// called from field_control_avatar.c every time we take a step
bool8 CheckForWildPokemonToBattle()
{
    struct ObjectEvent *pokemonObj;
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];

    for (u8 i = 0; i < MAX_ACTIVE_PKMN; i++)
    {
        u8 index = wildPokemonObjectEventIds[i];
        pokemonObj = &gWildPokemonObjects[index];

        s16 x, y;
        // Check collision from the pokemon's front
        u8 direction = pokemonObj->facingDirection;
        x = pokemonObj->currentCoords.x;
        y = pokemonObj->currentCoords.y;
        MoveCoords(direction, &x, &y);
        u8 collision = GetCollisionAtCoords(pokemonObj, x, y, direction);

        // Check collision from the player's direction
        direction = GetOppositeDirection(playerObj->movementDirection);
        x = pokemonObj->currentCoords.x;
        y = pokemonObj->currentCoords.y;
        MoveCoords(direction, &x, &y);
        u8 collisionB = GetCollisionAtCoords(pokemonObj, x, y, direction);
        if(collision == COLLISION_OBJECT_EVENT || collisionB == COLLISION_OBJECT_EVENT)
        {
            currentWildMonIndex = i;
            sWildBattle = AllocZeroed(sizeof(*sWildBattle));
            sWildBattle->initiator = collisionB == COLLISION_OBJECT_EVENT ? 0 : 1;
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

static bool8 GetRandCoords(s16 *x, s16 *y, u16 metatileId)
{
    const struct MapLayout *mapLayout = gMapHeader.mapLayout;
    s16 i, j;
    i = Random() % mapLayout->width;
    j = Random() %  mapLayout->height;

    if ((mapLayout->map[j * mapLayout->width + i] & MAPGRID_METATILE_ID_MASK) == metatileId)
    {
        if(IsMapCoordOccupied(i, j) == FALSE)
        {
            *x = i;
            *y = j;
            return TRUE;
        }
    }
    DebugPrintf("Failed on coords at: %d, %d", i,j);
    return FALSE;
}

static bool8 FindSpawnLocation(struct Coords16 *location, u32 species)
{
    location->x = gSaveBlock1Ptr->pos.x;
    location->y = gSaveBlock1Ptr->pos.y;

    bool8 isGood = FALSE;
    for(u8 i = 0; i < WILD_VALID_METATILES; i++)
    {
        bool8 result = GetRandCoords(&location->x, &location->y, sValidSpawnMetatiles[species][i]);
        if (result == TRUE)
            isGood = TRUE;
    }

    return isGood;
}

static bool8 SetupWildPokemon(u8 i)
{
    u32 species = GetMonData(&gWildPokemonInstances[i], MON_DATA_SPECIES);
    struct ObjectEventTemplate wildPokemonObjEventTemplate = sWildPokemonObjectEventTemplates[species];
    u8 objectEventId;
    struct ObjectEvent *objectEvent;

    struct Coords16 spawnLocation;
    if(FindSpawnLocation(&spawnLocation, species) == FALSE)
    {
        return FALSE;
    }

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

    DebugPrintf("Spawning Pokemon Species ID %d at (%d,%d)", species,spawnLocation.x, spawnLocation.y);

    return TRUE;
}

void TrySetupWildRoamingPokemon()
{
    sLocation[MAP_GRP] = gSaveBlock1Ptr->location.mapGroup;
    sLocation[MAP_NUM] = gSaveBlock1Ptr->location.mapNum;

    CreateWildWalkingMons(gWildPokemonInstances, MAX_ACTIVE_PKMN);

    for (u8 i = 0; i < MAX_ACTIVE_PKMN; i++)
    {
        SetupWildPokemon(i);
    }
}

// called in field_control_avatar.c whenever we take a step
u8 Step_CreateWildRoamingPokemon()
{
    if(numActivePokemon >= MAX_ACTIVE_PKMN)
    {
        DebugPrintf("Can't spawn wild pokemon, at max");
        return MAX_ACTIVE_PKMN;
    }

    gWildPokemonInstances[numActivePokemon] = CreateWildWalkingMon();
    if(SetupWildPokemon(numActivePokemon) == TRUE)
        numActivePokemon++;

    return numActivePokemon;
}

#endif