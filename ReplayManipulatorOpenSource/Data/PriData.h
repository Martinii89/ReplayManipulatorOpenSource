#pragma once
#include "PriUid.h"
#include "Features/CustomTextures/CustomTextures.h"
#include "bakkesmod/core/loadout_structs.h"

struct Loadout;

struct PriData
{
    PriData(PriWrapper& pri_wrapper, const pluginsdk::Loadout& loadout);

    bool operator==(const PriData& rhs) const;
    bool operator!=(const PriData& rhs) const;
    bool operator==(PriWrapper& rhs) const;
    bool operator!=(PriWrapper& rhs) const;

    void Update(PriWrapper& pri_wrapper, const pluginsdk::Loadout& updated_loadout);

    //// The player
    std::string player_name;
    PriUid uid;
    bool hidden = false;
    bool spectating = false;
    int team = 0;

    pluginsdk::Loadout loadout;
    CustomDecal custom_decal;
};
