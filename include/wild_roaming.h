#ifndef GUARD_WILD_ROAMING_H
#define GUARD_WILD_ROAMING_H

#if WILD_ROAMING == TRUE

#include "constants/trainer_types.h"
#include "constants/event_objects.h"
#include "constants/metatile_labels.h"
#include "event_object_movement.h"
#include "graphics.h"

#define WILD_VALID_METATILES 2

void TrySetupWildRoamingPokemon(void);
bool8 CheckForWildPokemonToBattle(void);
u8 Step_CreateWildRoamingPokemon(void);

EWRAM_DATA static u8 numActivePokemon = 0;

static const int sValidSpawnMetatiles[NUM_SPECIES][WILD_VALID_METATILES] = 
{
    [SPECIES_BULBASAUR] = {METATILE_General_TallGrass},
    [SPECIES_IVYSAUR] = {METATILE_General_TallGrass},
    [SPECIES_POOCHYENA] = {METATILE_General_TallGrass}
};

static const struct ObjectEventTemplate sWildPokemonObjectEventTemplates[NUM_SPECIES] =
{
    [SPECIES_BULBASAUR] = 
    {
        .graphicsId = OBJ_EVENT_GFX_BULBASAUR,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_LOOK_AROUND,
        .movementRangeX = 1,
        .movementRangeY = 1,
        .trainerType = TRAINER_TYPE_NORMAL,
    },
    [SPECIES_IVYSAUR] = 
    {
        .graphicsId = OBJ_EVENT_GFX_IVYSAUR,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_LOOK_AROUND,
        .movementRangeX = 1,
        .movementRangeY = 1,
        .trainerType = TRAINER_TYPE_NORMAL,
    },
    [SPECIES_POOCHYENA] = 
    {
        .graphicsId = OBJ_EVENT_GFX_POOCHYENA,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_WALK_SEQUENCE_UP_LEFT_DOWN_RIGHT,
        .movementRangeX = 3,
        .movementRangeY = 3,
        .trainerType = TRAINER_TYPE_NORMAL,
    },
};

#endif

#endif /* GUARD_WILD_ROAMING_H */