#include "pch.h"
#include "ReplayData.h"

ReplayData::ReplayData(ReplaySoccarWrapper& replay_wrapper)
    : name(replay_wrapper.GetReplayName().ToString()),
      id(replay_wrapper.GetId().ToString()),
      map(replay_wrapper.GetMapName()),
      date(replay_wrapper.GetDate().ToString()),
      team0_score(replay_wrapper.GetTeam0Score()),
      team1_score(replay_wrapper.GetTeam1Score()),
      team_size(replay_wrapper.GetTeamSize()),
      file_path(replay_wrapper.GetFilePath().ToString()),
      record_fps(replay_wrapper.GetRecordFPS()),
      match_type(replay_wrapper.GetMatchType()),
      goals(replay_wrapper.GetGoals()),
      highlights(replay_wrapper.GetHighlights()),
      player_stats(replay_wrapper.GetPlayerStats()) {}
