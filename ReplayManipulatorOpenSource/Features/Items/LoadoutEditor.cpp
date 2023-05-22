#include "pch.h"
#include "LoadoutEditor.h"

#include "ImguiUtils.h"
#include "ItemPaints.h"
#include "Items.h"
#include "PaintFinishColors.h"

size_t FindCaseInsensitive(std::string data, std::string to_search, const size_t pos = 0)
{
    // Convert complete given String to lower case
    std::ranges::transform(data, data.begin(), ::tolower);
    // Convert complete given Sub String to lower case
    std::ranges::transform(to_search, to_search.begin(), ::tolower);
    // Find sub string in given string
    return data.find(to_search, pos);
}

void RemoveAttribute(pluginsdk::ItemData item, pluginsdk::ItemAttribute::AttributeType type)
{
    const auto it = std::ranges::find_if(item.attributes, [type](const pluginsdk::ItemAttribute& attribute) {
        return attribute.type == type;
    });

    if (it != item.attributes.end())
    {
        item.attributes.erase(it);
    }
}

pluginsdk::ItemAttribute* GetAttributeOrNull(pluginsdk::ItemData& item, pluginsdk::ItemAttribute::AttributeType type)
{
    const auto it = std::ranges::find_if(item.attributes, [type](const pluginsdk::ItemAttribute& attribute) {
        return attribute.type == type;
    });
    if (it != item.attributes.end())
    {
        return &(*it);
    }
    return nullptr;
}

void SetOrAddAttribute(pluginsdk::ItemData& item, pluginsdk::ItemAttribute attribute)
{
    if (const auto att = GetAttributeOrNull(item, attribute.type))
    {
        *att = attribute;
        return;
    }
    item.attributes.emplace_back(attribute);
}

LoadoutEditor::LoadoutEditor(std::shared_ptr<Items> items,
                             std::shared_ptr<PaintFinishColors> paint_finish_colors,
                             std::shared_ptr<ItemPaints> itempaints)
    : items_(std::move(items)),
      paint_finish_colors_(std::move(paint_finish_colors)),
      itempaints_(std::move(itempaints)) {}

bool LoadoutEditor::DrawLoadoutEditor(pluginsdk::Loadout& loadout, int team_index) const
{
    bool changed = false;
    const auto current_body = loadout.items[pluginsdk::Equipslot::BODY].product_id;
    for (auto& [equipSlot, item] : loadout.items)
    {
        if (!editable_slots_.contains(equipSlot))
            continue;
        const SlotData& slot_data = items_->GetSlot(equipSlot);
        auto item_data = items_->GetItemOrDefaultData(item.product_id, equipSlot);
        changed |= DrawItemSlotSelector(slot_data, item_data, item, current_body);
    }
    changed |= DrawCarColorsEditor(loadout.paint_finish, team_index);
    return changed;
}

bool LoadoutEditor::DrawItemSlotSelector(const SlotData& slot, ProductItemData& item_data,
                                         pluginsdk::ItemData& current_item, const int current_body) const
{
    bool changed = false;
    char input_buffer[64] = "";
    const auto equip_slot = slot.equip_slot;

    bool incompatible = false;
    if (equip_slot == pluginsdk::Equipslot::DECAL && item_data.body_restrictions && !item_data.compatible_bodies.
                                                                                               contains(current_body))
    {
        incompatible = true;
    }
    auto push_red = [&] {
        if (incompatible)
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
    };
    auto pop_red = [&] {
        if (incompatible)
            ImGui::PopStyleColor();
    };

    if (auto* icon = GetSlotIconPtr(slot.equip_slot))
    {
        ImGui::Image(icon, {20, 20});
        ImGui::SameLine();
    }
    ImGui::PushItemWidth(200);

    push_red();
    const auto open = ImGui::BeginSearchableCombo(slot.label.c_str(), item_data.name.c_str(), input_buffer, 64,
                                                  "type to filter list");
    pop_red();

    if (open)
    {
        const std::string filter_str(input_buffer);

        const auto slot_to_fetch = (equip_slot == pluginsdk::Equipslot::CUSTOMFINISH)
                                       ? pluginsdk::Equipslot::PAINTFINISH
                                       : equip_slot;
        for (const auto& the_item : items_->GetSlotItems(slot_to_fetch))
        {
            if (FindCaseInsensitive(the_item.name, filter_str, 0) == std::string::npos)
                continue;

            if (equip_slot != pluginsdk::Equipslot::BODY && the_item.body_restrictions && !the_item.compatible_bodies.
                                                                                                    contains(current_body))
            {
                // The item only fits on certain bodies. and that body is not equipped. Skip this.
                continue;
            }
            const ImGui::ScopeId item_scope{the_item.id};
            const auto selected = item_data.id == the_item.id;
            if (ImGui::Selectable(the_item.name.c_str(), selected))
            {
                item_data = the_item;
                current_item.product_id = the_item.id;
                changed = true;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndSearchableCombo();
    }

    ImGui::SameLine();
    changed |= DrawPaintSelector(item_data, current_item);
    return changed;
}

bool LoadoutEditor::DrawPaintSelector(const ProductItemData& item_data, pluginsdk::ItemData& current_item) const
{
    bool changed = false;
    const ImGui::ScopeId id(item_data.slot.slot_id);
    const ImGui::Disable disable_non_paintable(!item_data.paintable);

    auto& paints = itempaints_->GetItemPaints();
    if (paints.empty())
        return changed;
    const auto* current_paint_attribute = GetAttributeOrNull(current_item,
                                                             pluginsdk::ItemAttribute::AttributeType::PAINT);

    int current_paint_id = current_paint_attribute == nullptr ? 0 : current_paint_attribute->value;

    if (!item_data.paintable && current_paint_attribute)
    {
        RemoveAttribute(current_item, pluginsdk::ItemAttribute::AttributeType::PAINT);
        changed = true;
    }

    if (current_paint_id < 0 || !paints.contains(current_paint_id))
    {
        RemoveAttribute(current_item, pluginsdk::ItemAttribute::AttributeType::PAINT);
        current_paint_id = 0;
        changed = true;
    }

    if (!item_data.paintable)
    {
        ImGui::TextUnformatted("Unpaintable");
        return changed;
    }

    // This is safe since we set it to zero if contains return false;
    const auto& current_paint = paints.at(current_paint_id);

    auto color_button_label = [](const LinearColor& color, const std::string& label,
                                 const ImGuiColorEditFlags flag = 0) {
        ImGui::BeginGroup();
        const auto clicked = ImGui::ColorButton(std::format("##{}", label).c_str(), color, flag);
        ImGui::SameLine();
        ImGui::TextUnformatted(label.c_str());
        ImGui::EndGroup();
        return clicked;
    };

    if (color_button_label(current_paint.colors[0], current_paint.name, ImGuiColorEditFlags_NoTooltip))
    {
        ImGui::OpenPopup("item_paint_selector");
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Click to change");
    }

    if (ImGui::BeginPopup("item_paint_selector"))
    {
        ImGui::Text("Select a Color");
        ImGui::Separator();
        const auto cols = paints.size() / 3 - 1;
        for (const auto& [paint_id, paint] : paints)
        {
            if (ImGui::ColorButton(paint.name.c_str(), paint.colors[0]))
            {
                changed = true;
                SetOrAddAttribute(current_item,
                                  {.type = pluginsdk::ItemAttribute::AttributeType::PAINT, .value = paint_id});
                ImGui::CloseCurrentPopup();
            }
            if (paint.is_custom && ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                itempaints_->SetUpdatingColor(paint.name, paint.colors[0]);
                ImGui::OpenPopup("item_paint_update_custom");
            }

            if ((paint_id + 1) % cols != 0)
                ImGui::SameLine();
        }
        if (ImGui::Button("Add custom"))
        {
            ImGui::OpenPopup("item_paint_add_custom");
        }
        ImGui::SameLine();
        HelpMarker("Right click custom color to update");
        if (ImGui::BeginPopup("item_paint_add_custom"))
        {
            if (itempaints_->DrawAddCustomPaintColorWidget())
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopup("item_paint_update_custom"))
        {
            if (itempaints_->DrawUpdateCustomPaintColorWidget())
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::EndPopup();
    }

    return changed;
}

const std::vector<pluginsdk::PaintFinishColor>& LoadoutEditor::GetTeamColorSet(const int team_id) const
{
    return team_id == 0
               ? paint_finish_colors_->GetBlueColorSet()
               : paint_finish_colors_->GetOrangeColorSet();
}

bool LoadoutEditor::DrawCarColorsEditor(pluginsdk::CarColors& current_paint_finish, const int team_id) const
{
    static LinearColor const default_color{0.0f, 0.0f, 0.0f, 1.0f};
    bool changed = false;
    {

        if (auto team_color_maybe = GetTeamColorFromCarColors(current_paint_finish); !team_color_maybe)
        {
            ImGui::TextUnformatted("Missing team color");
        }
        else
        {
            if (ImGui::ColorButton("Primary", *team_color_maybe))
            {
                ImGui::OpenPopup("team_paintfinish_select");
            }
            ImGui::SameLine();
            ImGui::TextUnformatted("Primary paint finish color");

            if (ImGui::BeginPopup("team_paintfinish_select"))
            {
                auto& primary_color_set = GetTeamColorSet(team_id);
                auto [selected, color] = PaintFinishColors::DrawColorSelectGrid(primary_color_set);
                if (selected)
                {
                    current_paint_finish.team_paint->team_color_id = static_cast<unsigned char>(color.index);
                    current_paint_finish.team_color_override = std::nullopt;
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
                LinearColor current_custom = current_paint_finish.team_color_override.value_or(pluginsdk::CarColors::default_color);
                if (ImGui::ColorEdit3("Custom Primary Color", &current_custom.R, ImGuiColorEditFlags_NoInputs))
                {
                    changed = true;
                    current_paint_finish.team_color_override = current_custom;
                }
                ImGui::EndPopup();
            }
        }

    }
    {
        if (auto custom_color_maybe = GetCustomColorFromCarColors(current_paint_finish); !custom_color_maybe)
        {
            ImGui::TextUnformatted("Missing custom color");
        }
        else
        {
            if (ImGui::ColorButton("Secondary", *custom_color_maybe))
            {
                ImGui::OpenPopup("custom_paintfinish_select");
            }
            ImGui::SameLine();
            ImGui::TextUnformatted("Secondary paint finish color");

            if (ImGui::BeginPopup("custom_paintfinish_select"))
            {
                auto& secondary_color_set = paint_finish_colors_->GetCustomColorSet();
                auto [selected, color] = PaintFinishColors::DrawColorSelectGrid(secondary_color_set);
                if (selected)
                {
                    current_paint_finish.team_paint->custom_color_id = static_cast<unsigned char>(color.index);
                    current_paint_finish.custom_color_override = std::nullopt;
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
                LinearColor current_custom = current_paint_finish.custom_color_override.value_or(pluginsdk::CarColors::default_color);
                if (ImGui::ColorEdit3("Custom Secondary Color", &current_custom.R, ImGuiColorEditFlags_NoInputs))
                {
                    changed = true;
                    current_paint_finish.custom_color_override = current_custom;
                }
                ImGui::EndPopup();
            }
        }

    }

    return changed;
}

void LoadoutEditor::LoadProductIcons(const std::filesystem::path& slot_icons_folder)
{
    slot_icons_[pluginsdk::Equipslot::BODY] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "Body.png", false, true);
    slot_icons_[pluginsdk::Equipslot::DECAL] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "Skin.png", false, true);
    slot_icons_[pluginsdk::Equipslot::WHEELS] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "Wheels.png", false, true);
    slot_icons_[pluginsdk::Equipslot::ROCKETBOOST] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "Boost.png", false, true);
    slot_icons_[pluginsdk::Equipslot::ANTENNA] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "Antenna.png", false, true);
    slot_icons_[pluginsdk::Equipslot::TOPPER] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "Hat.png", false, true);
    slot_icons_[pluginsdk::Equipslot::PAINTFINISH] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "Paint.png", false, true);
    slot_icons_[pluginsdk::Equipslot::CUSTOMFINISH] = slot_icons_[pluginsdk::Equipslot::PAINTFINISH];
    slot_icons_[pluginsdk::Equipslot::ENGINEAUDIO] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "EngineAudio.png", false, true);
    slot_icons_[pluginsdk::Equipslot::TRAIL] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "Trail.png", false, true);
    slot_icons_[pluginsdk::Equipslot::GOALEXPLOSION] = std::make_shared<ImageWrapper>(
        slot_icons_folder / "GoalFX.png", false, true);

    slot_icon_default_ = std::make_shared<ImageWrapper>(slot_icons_folder / "Missing.png", false, true);

}

std::optional<LinearColor> LoadoutEditor::GetTeamColorFromCarColors(const pluginsdk::CarColors& paint_finish) const
{
    if (paint_finish.team_color_override)
    {
        return *paint_finish.team_color_override;
    }
    if (paint_finish.team_paint)
    {
        auto index = static_cast<int>(paint_finish.team_paint->team_color_id);
        auto color_set = GetTeamColorSet(paint_finish.team_paint->team);
        if (color_set.size() > index)
        {
            return color_set[index].color;
        }
    }
    return {};
}

std::optional<LinearColor> LoadoutEditor::GetCustomColorFromCarColors(const pluginsdk::CarColors& paint_finish) const
{
    if (paint_finish.custom_color_override)
    {
        return *paint_finish.custom_color_override;
    }
    if (paint_finish.team_paint)
    {
        auto index = static_cast<int>(paint_finish.team_paint->custom_color_id);
        auto color_set = paint_finish_colors_->GetCustomColorSet();
        if (color_set.size() > index)
        {
            return color_set[index].color;
        }
    }
    return {};
}

void* LoadoutEditor::GetSlotIconPtr(const pluginsdk::Equipslot slot) const
{
    if (const auto it = slot_icons_.find(slot); it != slot_icons_.end())
    {
        return it->second->GetImGuiTex();
    }
    if (slot_icon_default_)
    {
        return slot_icon_default_->GetImGuiTex();
    }
    return nullptr;
}
