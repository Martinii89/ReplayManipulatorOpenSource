#include "pch.h"
#include "LoadoutPresetIO.h"

#include <utility>
#include <fstream>


LoadoutPresetIO::LoadoutPresetIO(std::filesystem::path loadout_path)
    : loadout_path_(std::move(loadout_path)) {}

LoadoutPresetIO::LoadoutPresetIO(std::filesystem::path loadout_path, std::string file_extension)
    : loadout_path_(std::move(loadout_path)),
      file_extension_(std::move(file_extension)) {}

std::vector<NamedLoadout> LoadoutPresetIO::ReadLoadouts(const std::filesystem::path& sub_folder) const
{
    return ReadFromFolder<NamedLoadout>(sub_folder);
}

std::vector<ReplayLoadout> LoadoutPresetIO::ReadReplayLoadouts(
    const std::filesystem::path& sub_folder) const
{
    return ReadFromFolder<ReplayLoadout>(sub_folder);
}

void LoadoutPresetIO::CreateFolder(const std::filesystem::path& path_to_create) const
{
    if (create_directories(path_to_create))
    {
        LOG("Created the directory for the output file");
    }
}

bool LoadoutPresetIO::WriteJsonToFile(const std::filesystem::path& path, const json& j) const
{
    CreateFolder(path.parent_path());
    std::ofstream o(path);
    if (!o.good())
    {
        LOG("Failed to open the output file when serializing the loadout");
        return false;
    }
    o << j.dump(2);
    return true;
}

bool LoadoutPresetIO::WriteLoadout(const NamedLoadout& loadout, const std::filesystem::path& sub_folder,
                                   const std::string& file_name) const
{
    if (file_name.empty() && loadout.name.empty())
    {
        LOG("Can't save a loadout without a name!");
        return false;
    }
    const auto final_file_name = (file_name.empty() ? loadout.name : file_name) + file_extension_;
    const auto path = loadout_path_ / sub_folder / final_file_name;

    try
    {
        LOG("Saving loadout to {}", absolute(path).string());
        const json j = loadout;
        return WriteJsonToFile(path, j);
    }
    catch (std::exception& e)
    {
        LOG("Something went wrong when serializing the loadout: {}", e.what());
        return false;
    }
    catch (...)
    {
        LOG("Something went wrong when serializing the loadout: UNKNOWN EXCEPTION");
        return false;
    }
}

bool LoadoutPresetIO::WriteReplayLoadout(const ReplayLoadout& loadout, const std::filesystem::path& sub_folder,
                                         const std::string& file_name) const
{
    if (file_name.empty() && loadout.replay_id.empty())
    {
        LOG("Can't save a loadout without a name!");
        return false;
    }
    const auto final_file_name = (file_name.empty() ? loadout.replay_id : file_name) + file_extension_;
    const auto path = loadout_path_ / sub_folder / final_file_name;

    try
    {
        LOG("Saving loadout to {}", absolute(path).string());
        const json j = loadout;
        return WriteJsonToFile(path, j);
    }
    catch (std::exception& e)
    {
        LOG("Something went wrong when serializing the loadout: {}", e.what());
        return false;
    }
    catch (...)
    {
        LOG("Something went wrong when serializing the loadout: UNKNOWN EXCEPTION");
        return false;
    }
}
