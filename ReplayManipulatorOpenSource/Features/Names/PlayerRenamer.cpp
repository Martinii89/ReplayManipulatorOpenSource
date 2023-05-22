#include "pch.h"
#include "PlayerRenamer.h"

PlayerRenamer::PlayerRenamer(std::shared_ptr<GameWrapper> gw)
    : gw_(std::move(gw))
{
    gw_->HookEventPost("Function TAGame.GFxNameplatesManager_TA.Update", [this](...) {
        if (name_cache_.empty())
            return;

        if (auto replay = gw_->GetGameEventAsReplay())
        {
            for (auto pri : replay.GetPRIs())
            {
                OnNameChange(pri);
            }
        }
    });
}


void PlayerRenamer::DoRename(PriWrapper pri, const std::string& new_name)
{
    pri.ChangeNameForScoreboardAndNameplateInReplay(new_name);
}

void PlayerRenamer::Rename(PriWrapper pri, const std::string& new_name)
{
    if (!CanRename())
    {
        LOG("player renamer is disabled");
        return;
    }
    if (!pri)
    {
        LOG("Pri is null when changing name to {}", new_name);
        return;
    }

    if (const auto id = PriUid(pri); IsInRenameCache(id))
    {
        name_cache_[id].new_name = new_name;
    }
    else
    {
        const auto original = pri.GetPlayerName().ToString();
        name_cache_[id] = {original, new_name};
    }

    DoRename(pri, new_name);
}


void PlayerRenamer::Restore(PriWrapper pri)
{
    if (!CanRename())
    {
        LOG("player renamer is disabled");
        return;
    }
    const auto id = PriUid(pri);
    if (const auto p = name_cache_.find(id); p != name_cache_.end())
    {
        DEBUGLOG("Restoring the name for {}", p->second.original);
        const auto original_name = p->second.original;
        name_cache_.erase(p);
        DoRename(pri, original_name);
    }
}


bool PlayerRenamer::IsInRenameCache(const PriUid& pri_id) const
{
    return name_cache_.contains(pri_id);
}


void PlayerRenamer::ResetNameOverrides()
{
    name_cache_.clear();
}


bool PlayerRenamer::CanRename() const
{
    return enabled_ && gw_->IsInReplay();
}


void PlayerRenamer::OnNameChange(PriWrapper& pri)
{
    if (!CanRename())
    {
        LOG("can't rename");
        return;
    }
    const auto id = PriUid(pri);
    auto old_name = pri.GetPlayerName().ToString();
    if (const auto p = name_cache_.find(id); p != name_cache_.end())
    {
        pri.ChangeNameForScoreboardAndNameplateInReplay(p->second.new_name);
    }
}
