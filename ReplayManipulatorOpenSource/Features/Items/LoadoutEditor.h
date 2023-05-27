#pragma once
#include <set>
#include <bakkesmod/core/loadout_structs.h>

class ItemPaints;
class PaintFinishColors;
struct ProductItemData;
struct SlotData;
class Items;

class LoadoutEditor
{
public:
    LoadoutEditor(std::shared_ptr<Items> items,
                  std::shared_ptr<PaintFinishColors> paint_finish_colors, std::shared_ptr<ItemPaints> itempaints);

    void LoadProductIcons(const std::filesystem::path& slot_icons_folder);

    [[nodiscard]] std::optional<LinearColor> GetTeamColorFromCarColors(const pluginsdk::CarColors& paint_finish) const;
    [[nodiscard]] std::optional<LinearColor> GetCustomColorFromCarColors(const pluginsdk::CarColors& paint_finish) const;

    [[nodiscard]] bool DrawLoadoutEditor(pluginsdk::Loadout& loadout, int team_index) const;
    [[nodiscard]] bool DrawItemSlotSelector(const SlotData& slot, ProductItemData& item_data,
                                            pluginsdk::ItemData& current_item, int current_body) const;
    [[nodiscard]] bool DrawPaintSelector(const ProductItemData& item_data, pluginsdk::ItemData& current_item) const;
    [[nodiscard]] const std::vector<pluginsdk::PaintFinishColor>& GetTeamColorSet(int team_id) const;
    [[nodiscard]] bool DrawCarColorsEditor(pluginsdk::CarColors& current_paint_finish, int team_id) const;

private:
    [[nodiscard]] void* GetSlotIconPtr(pluginsdk::Equipslot slot) const;

    std::shared_ptr<Items> items_;
    std::shared_ptr<PaintFinishColors> paint_finish_colors_;
    std::shared_ptr<ItemPaints> itempaints_;
    std::unordered_map<pluginsdk::Equipslot, std::shared_ptr<ImageWrapper>> slot_icons_;
    std::shared_ptr<ImageWrapper> slot_icon_default_;

    std::set<pluginsdk::Equipslot> editable_slots_ = {
        pluginsdk::Equipslot::BODY,
        pluginsdk::Equipslot::DECAL,
        pluginsdk::Equipslot::WHEELS,
        pluginsdk::Equipslot::ROCKETBOOST,
        pluginsdk::Equipslot::TOPPER,
        pluginsdk::Equipslot::ANTENNA,
        pluginsdk::Equipslot::TRAIL,
        pluginsdk::Equipslot::PAINTFINISH,
        pluginsdk::Equipslot::CUSTOMFINISH,
        pluginsdk::Equipslot::ENGINEAUDIO,
        pluginsdk::Equipslot::GOALEXPLOSION,
    };
};
