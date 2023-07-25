
#if WILD_ROAMING == TRUE

#include "global.h"
#include "global.fieldmap.h"
#include "event_data.h"
#include "util.h"
#include "overworld.h"
#include "random.h"
#include "sprite.h"
#include "pokemon.h"
#include "wild_encounter.h"
#include "wild_roaming.h"
#include "event_object_movement.h"
#include "graphics.h"
#include "constants/species.h"
#include "constants/event_objects.h"
#include "fieldmap.h"
#include "event_scripts.h"

#define MAX_ACTIVE_PKMN 10

EWRAM_DATA static u8 sLocation[2] = {0}; //represents the current location
EWRAM_DATA struct Pokemon gWildPokemonInstances[MAX_ACTIVE_PKMN] = {0};

enum
{
    MAP_GRP, // map group
    MAP_NUM, // map number
};

// called from field_control_avatar.c
bool8 CheckForWildPokemonToBattle()
{
    return FALSE;
}

void TrySetupWildRoamingPokemon()
{
    sLocation[MAP_GRP] = gSaveBlock1Ptr->location.mapGroup;
    sLocation[MAP_NUM] = gSaveBlock1Ptr->location.mapNum;

    CreateWildWalkingMons(gWildPokemonInstances, MAX_ACTIVE_PKMN);

    for(u8 i=0;i < MAX_ACTIVE_PKMN;i++)
    {
        u8 obj = SpawnSpecialObjectEventParameterized(
                                            OBJ_EVENT_GFX_MAN_1, 
                                            MOVEMENT_TYPE_FACE_DOWN, 
                                            LOCALID_WILD_ROAMING_1 + i,
                                            gSaveBlock1Ptr->pos.x + i + MAP_OFFSET, 
                                            gSaveBlock1Ptr->pos.y + i + MAP_OFFSET,
                                            3);

        OverrideObjectEventTemplateScript(&gObjectEvents[obj], BerryTreeScript);
    }
}

#endif