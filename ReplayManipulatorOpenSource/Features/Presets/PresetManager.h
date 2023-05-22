#pragma once
#include "LoadoutPresetIO.h"
#include "Data/PriData.h"
#include "Features/Items/Items.h"
#include "bakkesmod/core/loadout_structs.h"
#include "Features/Items/ItemPaints.h"

struct PresetVm
{
    NamedLoadout loadout;
    PriData pri_data;
    std::map<pluginsdk::Equipslot, bool> slot_selected;

    PresetVm()
        : pri_data({loadout.loadout, ""})
    {
        for (auto slot : Items::loadout_slots_)
        {
            slot_selected[slot] = true;
        }
    }
};

class PresetManager
{
public:
    explicit PresetManager(LoadoutPresetIO loadout_preset_io, std::shared_ptr<Items> items, std::function<void(PriData&)> on_loadout_changed);

    void RefreshPresets();

    static Loadout ToLoadout(const PriData& pri_data);
    static PriData FromLoadout(const Loadout& loadout);
    static PriData FromLoadout(const Loadout& loadout, const std::string& name);

    bool StoreReplayLoadout(const std::vector<PriData>& replay_players, const std::string& replay_id,
                            const std::string& replay_name);
    bool StoreReplayLoadout(const ReplayLoadout& replay_loadout);

    bool StoreNamedLoadout(const NamedLoadout& loadout);

    static void LoadReplayLoadout(const ReplayLoadout& replay_loadout, std::vector<PriData>& replay_players);
    bool DrawPresetTabContent(const std::string& current_replay_id, const std::string& current_replay_name,
                              std::vector<PriData>& replay_players);
    void DrawPriPresetsWidgets(PriData& current_pri_data);
    void DrawPresetsModal(PriData& current_pri_data);
    void DrawPresetVm(PresetVm& vm) const;

    const ReplayLoadout& GetReplayLoadout(const std::string& replay_id);
    const NamedLoadout& GetPresetLoadout(const std::string& name);

    static void DrawPriData(const PriData& pri_data, Items* items_manager, std::map<pluginsdk::Equipslot, bool>* slot_selection = nullptr);
    static void DrawItem(const ProductItemData& item, const ItemPaints::ItemPaint& paint);

private:
    void ValidateAndReplaceBadItems(Loadout& loadout) const;

    std::vector<NamedLoadout> named_loadouts_;
    std::vector<ReplayLoadout> replay_loadouts_;
    std::map<std::string, PriData> named_loadouts_data_;
    std::map<std::string, std::pair<std::string, std::vector<PriData>>> replay_loadouts_data_;


    LoadoutPresetIO loadout_preset_io_;
    std::filesystem::path presets_subfolder_ = "players";
    std::filesystem::path replays_subfolder_ = "replays";
    std::shared_ptr<Items> items_manager_;

    PresetVm preset_view_model_;

    std::function<void(PriData& pri_updated)> on_loadout_changed_;

};
