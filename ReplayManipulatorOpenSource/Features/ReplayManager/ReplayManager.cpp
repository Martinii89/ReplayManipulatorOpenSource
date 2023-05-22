#include "pch.h"
#include "ReplayManager.h"
#include "ImguiUtils.h"

#include "bakkesmod/wrappers/GameObject/ReplayManagerWrapper.h"

size_t FindCaseInsensitive(std::string data, std::string to_search, const size_t pos = 0);

void ReplayManager::LoadReplays(const ReplayManagerWrapper& replay_manager) const
{
    if (replay_manager.LoadReplaysFromDemoFolder())
    {
        LOG("ReplayManager: Loading replays");
    }
    else
    {
        LOG("ReplayManager: Error loading replays");
    }
}

void ReplayManager::GetOrStarLoadingReplays()
{
    const auto replay_manager = gw_->GetReplayManagerWrapper();
    if (!replay_manager)
    {
        LOG("no replay manager");
        return;
    }

    auto loaded_replays = replay_manager.GetLoadedReplays();
    if (loaded_replays.empty())
    {
        LOG("starting to load replays");
        LoadReplays(replay_manager);
        return;
    }
    SetLoadedReplays(loaded_replays);
}

ReplayManager::ReplayManager(std::shared_ptr<GameWrapper> gw, std::shared_ptr<CVarManagerWrapper> cv)
    : GuiFeatureBase{std::move(gw), "Replay Manager", DefaultVisibility::kSettings},
      cv_(std::move(cv))
{
    gw_->HookEventPost("Function TAGame.ReplayManager_TA.Tick", [this](...) {
        const auto replay_manager = gw_->GetReplayManagerWrapper();

        if (!replay_manager || replay_manager.IsLoadingReplayHeaders())
        {
            return;
        }

        auto loaded_replays = replay_manager.GetLoadedReplays();
        if (!loaded_replays.empty())
            SetLoadedReplays(loaded_replays);
    });
}


void ReplayManager::PlayReplay(const std::filesystem::path& file, float time) const
{
    gw_->Execute([file, time](GameWrapper* gw) {
        LOG(L"Play {} from time {}", file.wstring(), time);
        gw->PlayReplayFromTime(file.wstring(), time);
    });
}

void ReplayManager::SetLoadedReplays(std::vector<ReplaySoccarWrapper>& replay_wrappers)
{
    std::vector<ReplayData> new_replays;
    for (ReplaySoccarWrapper& replay_wrapper : replay_wrappers)
    {
        if (!replay_wrapper)
            continue;
        new_replays.emplace_back(replay_wrapper);
    }
    {
        std::scoped_lock const lock(replays_mutex_);
        replays_ = std::move(new_replays);
    }
}


//void ReplayManager::DrawTeamTableForTooltip(const ParsedReplayData::TeamData& team_to_draw) const
//{
//	ImGui::Columns(5);
//	ImGui::SetColumnWidth(0, 150);
//	{
//		ImGui::Text("%i %s", team_to_draw.goals, team_to_draw.name.c_str());
//		ImGui::NextColumn();
//
//
//		ImGui::TextUnformatted("Score");
//		ImGui::NextColumn();
//		ImGui::TextUnformatted("Assists");
//		ImGui::NextColumn();
//		ImGui::TextUnformatted("Saves");
//		ImGui::NextColumn();
//		ImGui::TextUnformatted("Shots");
//		ImGui::NextColumn();
//		ImGui::Separator();
//		for (const auto& [name, team, score, goals, assists, saves, shots, is_bot] : team_to_draw.players)
//		{
//			ImGui::TextUnformatted(name.c_str());
//			ImGui::NextColumn();
//
//			ImGui::Text("%i", score);
//			ImGui::NextColumn();
//			ImGui::Text("%i", assists);
//			ImGui::NextColumn();
//			ImGui::Text("%i", saves);
//			ImGui::NextColumn();
//			ImGui::Text("%i", shots);
//			ImGui::NextColumn();
//		}
//	}
//	ImGui::Columns(1);
//}

void ReplayManager::Render()
{
    if (replays_.empty())
    {
        if (ImGui::Button("Load demo folder"))
        {
            gw_->Execute([this](...) {
                GetOrStarLoadingReplays();
            });
        }
        return;
    }
    std::scoped_lock const lock(replays_mutex_);

    static std::string search;
    ImGui::InputText("Filter by name", &search);

    ImGui::Columns(4);
    for (auto& [name, id, map, date, team0_score, team1_score, team_size, file_path, record_fps, match_type, goals,
             highlights, player_stats] : replays_)
    {
        if (!search.empty() && FindCaseInsensitive(name, search, 0) == std::string::npos)
            continue;
        ImGui::ScopeId const scope_id(id + name);
        std::string label;
        if (name.empty())
        {
            label = std::format("{} ({}-{})", id, team0_score, team1_score);
        }
        else
        {
            label = std::format("{} ({}-{})", name, team0_score, team1_score);
        }
        ImGui::Selectable(label.c_str(), false,
                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);
        //const bool hovered = ImGui::IsItemHovered();
        ImGui::NextColumn();

        ImGui::TextUnformatted(date.c_str());
        ImGui::NextColumn();

        ImGui::TextUnformatted(match_type.c_str());
        ImGui::NextColumn();

        if (ImGui::SmallButton("Play"))
        {
            ImGui::OpenPopup("Play replay");
        }
        ImGui::NextColumn();
        //if (hovered)
        //{
        //	const auto line_height = ImGui::GetTextLineHeightWithSpacing();
        //	// 7 actually. but make some extra space
        //	const auto lines = player_stats.size() + goals.size() + 8;
        //	ImGui::SetNextWindowContentSize({500, static_cast<float>(lines) * line_height});
        //	ImGui::BeginTooltip();
        //	ImGui::Text("ReplayId: %s", id.c_str());
        //	DrawTeamTableForTooltip(teams[0]);
        //	ImGui::NewLine();
        //	ImGui::NewLine();
        //	DrawTeamTableForTooltip(teams[1]);
        //	ImGui::NewLine();
        //	ImGui::TextUnformatted("Goals");
        //	for (const auto& [frame, player_name, team] : goals)
        //	{
        //		ImGui::Text("Frame[%i]: Goal scored by %s", frame, player_name.c_str());
        //	}
        //	ImGui::EndTooltip();
        //}
        if (ImGui::BeginPopup("Play replay"))
        {
            if (ImGui::Button("Play from start"))
            {
                PlayReplay(file_path, 0);
            }
            for (const auto& [frame, player_name, team] : goals)
            {
                if (ImGui::Button(std::format("Play from Frame {} ({})", frame, player_name).c_str()))
                {
                    const auto goal_time = static_cast<float>(frame) / record_fps;
                    PlayReplay(file_path, goal_time - 4); // 4 seconds seems like a ok pre-load time.
                }
            }
            ImGui::EndPopup();
        }
    }

    ImGui::Columns(1);
}
