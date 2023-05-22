#include "pch.h"
#include "PresetManager.h"

#include "ImguiUtils.h"
#include "IMGUI/imgui_stdlib.h"


PresetManager::PresetManager(LoadoutPresetIO loadout_preset_io, std::shared_ptr<Items> items,
                             std::function<void(PriData&)> on_loadout_changed)
    : loadout_preset_io_(std::move(loadout_preset_io)),
      items_manager_(std::move(items)),
      on_loadout_changed_(std::move(on_loadout_changed))

{
    RefreshPresets();
}

void PresetManager::RefreshPresets()
{
    named_loadouts_ = loadout_preset_io_.ReadLoadouts("players");
    replay_loadouts_ = loadout_preset_io_.ReadReplayLoadouts("replays");
    named_loadouts_data_.clear();
    replay_loadouts_data_.clear();

    for (auto& [name, loadout] : named_loadouts_)
    {
        ValidateAndReplaceBadItems(loadout);
        named_loadouts_data_.emplace(name, FromLoadout(loadout, name));
    }

    for (auto& [replay_id, replay_name, loadouts] : replay_loadouts_)
    {
        replay_loadouts_data_[replay_id] = {replay_name, {}};
        for (auto& [name, loadout] : loadouts | std::views::values)
        {
            ValidateAndReplaceBadItems(loadout);
            replay_loadouts_data_[replay_id].second.push_back(FromLoadout(loadout, name));
        }
    }
}

Loadout PresetManager::ToLoadout(const PriData& pri_data)
{
    Loadout loadout{};
    loadout.id_string = pri_data.uid.pri_id_string;

    loadout.custom_decal_name = pri_data.custom_decal.name;

    if (const auto att = pri_data.loadout.items.at(pluginsdk::Equipslot::WHEELS).GetAttributeOrNull(pluginsdk::ItemAttribute::AttributeType::ESPORT))
    {
        loadout.esport_wheel_id = att->value;
    }

    loadout.paint_override.enabled = pri_data.loadout.paint_finish_colors.override_color;
    const auto& p = pri_data.loadout.paint_finish_colors.team_color_override;
    loadout.paint_override.primary = {p.R, p.G, p.B, p.A};
    const auto& s = pri_data.loadout.paint_finish_colors.custom_color_override;
    loadout.paint_override.secondary = {s.R, s.G, s.B, s.A};

    for (const auto& [slot_id, item] : pri_data.loadout.items)
    {
        const auto slot_search = std::ranges::find(Items::loadout_slots_, slot_id);
        if (slot_search == Items::loadout_slots_.end())
            continue;
        loadout.items[slot_id].id = item.product_id;
        if (const auto paint_att = item.GetAttributeOrNull(pluginsdk::ItemAttribute::AttributeType::PAINT))
        {
            loadout.items[slot_id].paint_id = paint_att->value;
        }

    }

    return loadout;
}

PriData PresetManager::FromLoadout(const Loadout& loadout)
{
    return FromLoadout(loadout, "Unknown");
}

PriData PresetManager::FromLoadout(const Loadout& loadout, const std::string& name)
{
    return {loadout, name};
}

bool PresetManager::StoreReplayLoadout(const std::vector<PriData>& replay_players,
                                       const std::string& replay_id, const std::string& replay_name)
{
    ReplayLoadout replay_loadout;
    replay_loadout.replay_id = replay_id;
    replay_loadout.replay_name = replay_name;
    for (const auto& p : replay_players)
    {
        auto loadout = ToLoadout(p);
        replay_loadout.loadouts[loadout.id_string] = {p.player_name, loadout};
    }
    const auto res = loadout_preset_io_.WriteReplayLoadout(replay_loadout, replays_subfolder_);
    RefreshPresets();
    return res;
}

bool PresetManager::StoreReplayLoadout(const ReplayLoadout& replay_loadout)
{
    const auto res = loadout_preset_io_.WriteReplayLoadout(replay_loadout, replays_subfolder_);
    RefreshPresets();
    return res;
}

bool PresetManager::StoreNamedLoadout(const NamedLoadout& loadout)
{
    const auto res = loadout_preset_io_.WriteLoadout(loadout, presets_subfolder_);
    RefreshPresets();
    return res;
}

void PresetManager::LoadReplayLoadout(const ReplayLoadout& replay_loadout,
                                      std::vector<PriData>& replay_players)
{
    DEBUGLOG("Load replay Loadout");
    for (const auto& [id, loadout] : replay_loadout.loadouts)
    {
        for (auto& player : replay_players)
        {
            if (player.uid.pri_id_string == id)
            {
                DEBUGLOG("Load loadout for {}", player.player_name);
                player.LoadLoadout(loadout.loadout);
                break;
            }
        }
    }
}

bool PresetManager::DrawPresetTabContent(const std::string& current_replay_id,
                                         const std::string& current_replay_name,
                                         std::vector<PriData>& replay_players)
{
    auto replay_players_modified = false;
    if (ImGui::Button("Save current replay loadout"))
    {
        StoreReplayLoadout(replay_players, current_replay_id, current_replay_name);
    }

    for (const auto& [replay_id, replay_data] : replay_loadouts_data_)
    {
        ImGui::ScopeId const replay_scope{replay_id};
        const auto open = ImGui::TreeNode(std::format("{} ({})", replay_data.first, replay_id).c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("Load"))
        {
            LoadReplayLoadout(GetReplayLoadout(replay_id), replay_players);
            replay_players_modified = true;
        }
        if (open)
        {
            for (const auto& pri_data : replay_data.second)
            {
                ImGui::ScopeId const id_scope{pri_data.uid.pri_id_string};
                if (ImGui::TreeNode(pri_data.player_name.c_str()))
                {
                    DrawPriData(pri_data, items_manager_.get());
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::Button("Refresh"))
    {
        RefreshPresets();
    }
    return replay_players_modified;
}

void PresetManager::DrawPriPresetsWidgets(PriData& current_pri_data)
{
    ImGui::ScopeId const pri_scope{current_pri_data.uid.pri_id_string};

    if (ImGui::Button("Presets"))
    {
        ImGui::OpenPopup("pri_presets_modal");
    }
    ImGui::SameLine();
    if (ImGui::BeginPopup("pri_presets_modal"))
    {
        DrawPresetsModal(current_pri_data);
        ImGui::EndPopup();
    }
}

void PresetManager::DrawPresetsModal(PriData& current_pri_data)
{
    static std::string preset_name;
    ImGui::BeginChild("PresetsModal", {450, 250});
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("Name", &preset_name);
    ImGui::SameLine();
    HelpMarker("Save the current preset");
    ImGui::SameLine();
    {
        ImGui::Disable const disable_if_no_name{preset_name.empty()};
        if (ImGui::Button("Save"))
        {
            const auto loadout = ToLoadout(current_pri_data);
            const NamedLoadout named_loadout{preset_name, loadout};
            StoreNamedLoadout(named_loadout);
        }
    }
    ImGui::Separator();

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 200);
    ImGui::BeginChild("ThePresetsTable", {-1, -1});

    for (const auto& [loadout_name, loadout_pri_data] : named_loadouts_data_)
    {
        const auto selected = preset_view_model_.loadout.name == loadout_name;
        if (ImGui::Selectable(loadout_name.c_str(), selected))
        {
            preset_view_model_ = PresetVm{};
            preset_view_model_.loadout = GetPresetLoadout(loadout_name);
            preset_view_model_.pri_data = loadout_pri_data;
        }
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    ImGui::BeginChild("ThePresetVm", {-1, -35});
    if (ImGui::Button("Toggle all"))
    {
        auto val = preset_view_model_.slot_selected[pluginsdk::Equipslot::BODY];
        for (auto& selected : preset_view_model_.slot_selected | std::views::values)
        {
            selected = !val;
        }
    }
    DrawPresetVm(preset_view_model_);
    ImGui::EndChild();
    if (ImGui::Button("Load"))
    {
        Loadout filtered_loadout = preset_view_model_.loadout.loadout;
        for (auto& s : preset_view_model_.slot_selected)
        {
            if (!s.second)
            {
                filtered_loadout.items.erase(s.first);
            }
        }
        current_pri_data.LoadLoadout(filtered_loadout);
        on_loadout_changed_(current_pri_data);
    }

    ImGui::EndChild();
}

void PresetManager::DrawPresetVm(PresetVm& vm) const
{
    DrawPriData(vm.pri_data, items_manager_.get(), &vm.slot_selected);
}

void PresetManager::DrawItem(const ProductItemData& item, const ItemPaints::ItemPaint& paint)
{
    ImGui::TextUnformatted(item.name.c_str());

    if (paint.attribute_id != 0)
    {
        ImGui::SameLine();
        ImGui::ColorButton("##CurrentColor", paint.colors[0], ImGuiColorEditFlags_NoTooltip);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", paint.name.c_str());
        }
    }
}

void PresetManager::ValidateAndReplaceBadItems(Loadout& loadout) const
{
    for (auto& [slot, item] : loadout.items)
    {
        if (!items_manager_->IsValidProductId(item.id))
        {
            item.id = items_manager_->GetDefaultItem(slot).id;
            item.paint_id = 0;
        }
    }
}

void PresetManager::DrawPriData(const PriData& pri_data, Items* items_manager, std::map<pluginsdk::Equipslot, bool>* slot_selection)
{
    //TODO: Use loadout editor
}

const ReplayLoadout& PresetManager::GetReplayLoadout(const std::string& replay_id)
{
    const auto it = std::find_if(replay_loadouts_.begin(), replay_loadouts_.end(),
                                 [replay_id](const ReplayLoadout& replay_loadout) {
                                     return replay_loadout.replay_id == replay_id;
                                 });
    if (it != replay_loadouts_.end())
    {
        return *it;
    }
    throw std::invalid_argument("replay loadout not found");
}

const NamedLoadout& PresetManager::GetPresetLoadout(const std::string& name)
{
    const auto it = std::find_if(named_loadouts_.begin(), named_loadouts_.end(),
                                 [name](const NamedLoadout& replay_loadout) {
                                     return replay_loadout.name == name;
                                 });
    if (it != named_loadouts_.end())
    {
        return *it;
    }
    throw std::invalid_argument("preset loadout not found");
}
