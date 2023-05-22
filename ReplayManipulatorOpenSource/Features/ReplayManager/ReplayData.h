#pragma once

struct ReplayData
{
    explicit ReplayData(ReplaySoccarWrapper& replay_wrapper);

    std::string name;
    std::string id;
    std::string map;
    std::string date;
    int team0_score;
    int team1_score;
    int team_size;
    std::string file_path;
    float record_fps;
    std::string match_type;

    std::vector<ScoredGoal> goals;
    std::vector<Highlight> highlights;
    std::vector<ReplayPlayerStats> player_stats;
};
