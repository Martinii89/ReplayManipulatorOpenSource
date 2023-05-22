#include "pch.h"
#include "PriData.h"


PriData::PriData(PriWrapper& pri_wrapper, const pluginsdk::Loadout& loadout)
    : uid(pri_wrapper)
{
    Update(pri_wrapper, loadout);
}


void PriData::Update(PriWrapper& pri_wrapper, const pluginsdk::Loadout& updated_loadout)
{
    if (*this != pri_wrapper)
    {
        return;
    }

    player_name = pri_wrapper.GetPlayerName().ToString();
    auto car = pri_wrapper.GetCar();
    spectating = car.IsNull();
    hidden = car.IsNull() ? true : static_cast<bool>(car.GetbHidden());
    team = pri_wrapper.GetTeamNum2();
    // too lazy
    if (team > 1)
    {
        team = 0;
    }

    loadout = updated_loadout;
}

bool PriData::operator==(const PriData& rhs) const
{
    return uid == rhs.uid;
}

bool PriData::operator!=(const PriData& rhs) const
{
    return !(*this == rhs);
}

bool PriData::operator==(PriWrapper& rhs) const
{
    return uid == PriUid(rhs);
}

bool PriData::operator!=(PriWrapper& rhs) const
{
    return !(*this == rhs);
}
