#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include "bakkesmod/core/loadout_structs.h"

struct Item
{
    int id = 0;
    int paint_id = 0;
};


void to_json(json& j, const Item& obj);
void from_json(const json& j, Item& obj);

void to_json(json& j, const LinearColor& obj);
void from_json(const json& j, LinearColor& obj);


struct PaintOverride
{
    bool enabled = false;
    LinearColor primary;
    LinearColor secondary;
};

void to_json(json& j, const PaintOverride& obj);
void from_json(const json& j, PaintOverride& obj);

struct Loadout
{
    std::string id_string; // Not sure if this should be here or not.
    std::map<pluginsdk::Equipslot, Item> items;
    PaintOverride paint_override;
    int esport_wheel_id = -1;
    std::string custom_decal_name;
};

void to_json(json& j, const Loadout& obj);
void from_json(const json& j, Loadout& obj);

struct NamedLoadout
{
    std::string name; //player name for replays and the "preset name" for a preset
    Loadout loadout;
};

void to_json(json& j, const NamedLoadout& obj);
void from_json(const json& j, NamedLoadout& obj);

struct ReplayLoadout
{
    std::string replay_id;
    std::string replay_name;
    std::map<std::string, NamedLoadout> loadouts;
};

void to_json(json& j, const ReplayLoadout& obj);
void from_json(const json& j, ReplayLoadout& obj);


class LoadoutPresetIO
{
public:
    explicit LoadoutPresetIO(std::filesystem::path loadout_path);
    explicit LoadoutPresetIO(std::filesystem::path loadout_path, std::string file_extension);


    [[nodiscard]] std::vector<NamedLoadout> ReadLoadouts(const std::filesystem::path& sub_folder) const;
    [[nodiscard]] std::vector<ReplayLoadout> ReadReplayLoadouts(const std::filesystem::path& sub_folder) const;

    [[nodiscard]] bool WriteLoadout(const NamedLoadout& loadout, const std::filesystem::path& sub_folder,
                                    const std::string& file_name = "") const;
    [[nodiscard]] bool WriteReplayLoadout(const ReplayLoadout& loadout, const std::filesystem::path& sub_folder,
                                          const std::string& file_name = "") const;

private:
    const std::filesystem::path loadout_path_;
    const std::string file_extension_ = ".preset";

    void CreateFolder(const std::filesystem::path& path_to_create) const;
    [[nodiscard]] bool WriteJsonToFile(const std::filesystem::path& path, const json& j) const;

    template <typename T>
    [[nodiscard]] T ReadFromJsonFile(const std::filesystem::path& path) const;

    template <typename T>
    [[nodiscard]] std::vector<T> ReadFromFolder(const std::filesystem::path& sub_folder) const;
};

template <typename T>
T LoadoutPresetIO::ReadFromJsonFile(const std::filesystem::path& path) const
{
    std::ifstream i(path);
    if (!i.good())
    {
        throw std::invalid_argument("Failed to open file");
    }

    json j;
    i >> j;
    auto obj = j.get<T>();
    return obj;
}

template <typename T>
std::vector<T> LoadoutPresetIO::ReadFromFolder(const std::filesystem::path& sub_folder) const
{
    std::vector<T> loadouts;
    const auto folder = loadout_path_ / sub_folder;
    if (!exists(folder))
    {
        LOG("loadout folder does not exist {}", folder.string());
        return loadouts;
    }

    for (const auto& f : std::filesystem::recursive_directory_iterator(folder))
    {
        if (!is_regular_file(f))
            continue;
        const auto& p = f.path();
        if (p.has_extension() && p.extension() == file_extension_)
        {
            LOG("Deserializing the file {}", p.string());
            try
            {
                loadouts.emplace_back(ReadFromJsonFile<T>(p));
            }
            catch (...)
            {
                LOG("Failed to deserialize the loadout");
            }
        }
    }
    return loadouts;
}
