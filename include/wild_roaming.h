#ifndef GUARD_WILD_ROAMING_H
#define GUARD_WILD_ROAMING_H

#if WILD_ROAMING == TRUE

#include "constants/trainer_types.h"
#include "constants/event_objects.h"
#include "constants/metatile_labels.h"
#include "event_object_movement.h"
#include "graphics.h"

#define WILD_VALID_METATILES 1

void TrySetupWildRoamingPokemon(void);
bool8 CheckForWildPokemonToBattle(void);

static const int sValidSpawnMetatiles[WILD_VALID_METATILES] = 
{
    METATILE_General_TallGrass
};

static const struct ObjectEventTemplate sWildPokemonObjectEventTemplates[NUM_SPECIES] =
{
    [SPECIES_POOCHYENA] = 
    {
        .graphicsId = OBJ_EVENT_GFX_EXAMPLE,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_LOOK_AROUND,
        .movementRangeX = 1,
        .movementRangeY = 1,
        .trainerType = TRAINER_TYPE_NORMAL,
    },
};

#endif

#endif /* GUARD_WILD_ROAMING_H */