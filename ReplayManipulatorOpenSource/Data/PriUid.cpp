#include "pch.h"
#include "PriUid.h"

PriUid::PriUid(PriWrapper& pri)
    : pri_id_string(GetPriIdString(pri)) {}

PriUid::PriUid(std::string id_string)
    : pri_id_string(std::move(id_string)) {}

std::string PriUid::GetPriIdString(PriWrapper& pri)
{
    if (pri.GetbBot())
    {
        return pri.GetPlayerName().ToString();
    }
    return pri.GetUniqueIdWrapper().GetIdString();
}
