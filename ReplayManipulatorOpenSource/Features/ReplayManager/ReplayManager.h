#pragma once
#include <filesystem>
#include "ReplayData.h"
#include "Framework/GuiFeatureBase.h"

struct ReplayData;

class ReplayManager final : public GuiFeatureBase
{
public:
    void LoadReplays(const ReplayManagerWrapper& replay_manager) const;
    ReplayManager(std::shared_ptr<GameWrapper> gw, std::shared_ptr<CVarManagerWrapper> cv);

    void Render() override;

private:
    void PlayReplay(const std::filesystem::path& file, float time) const;
    void SetLoadedReplays(std::vector<ReplaySoccarWrapper>& replay_wrappers);
    void GetOrStarLoadingReplays();

    //void DrawTeamTableForTooltip(const ParsedReplayData::TeamData& team_to_draw) const;
    std::shared_ptr<CVarManagerWrapper> cv_;
    std::mutex replays_mutex_;
    std::vector<ReplayData> replays_;
};
