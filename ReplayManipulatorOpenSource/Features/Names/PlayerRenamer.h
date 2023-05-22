#pragma once
#include "bakkesmod/wrappers/gamewrapper.h"
#include "Data/PriUid.h"


struct PlayerNameOverride
{
    std::string original;
    std::string new_name;
};

class PlayerRenamer
{
public:
    explicit PlayerRenamer(std::shared_ptr<GameWrapper> gw);

    void Rename(PriWrapper pri, const std::string& new_name);
    void Restore(PriWrapper pri);
    void ResetNameOverrides();
    [[nodiscard]] bool IsInRenameCache(const PriUid& pri_id) const;
    [[nodiscard]] bool CanRename() const;

private:
    void OnNameChange(PriWrapper& pri);
    static void DoRename(PriWrapper pri, const std::string& new_name);

    std::map<PriUid, PlayerNameOverride> name_cache_;
    bool enabled_ = true;
    std::shared_ptr<GameWrapper> gw_;
};
