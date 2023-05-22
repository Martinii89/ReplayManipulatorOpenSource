#pragma once
#include <unordered_set>
#include <unordered_map>
#include <bakkesmod/core/loadout_structs.h>


struct SlotData
{
    static SlotData FromSlotWrapper(ProductSlotWrapper& slot_wrapper);

    int slot_id = -1;
    pluginsdk::Equipslot equip_slot = pluginsdk::Equipslot::MAX;
    std::string label;
    //UTexture* thumbnail = nullptr;
};

struct ProductItemData
{
    std::string name;
    int id = 0;
    SlotData slot;
    bool paintable = false;
    bool can_equip = false;
    std::unordered_set<int> compatible_bodies;
    bool body_restrictions = false;
    bool item_can_take_esport_attribute = false;
};


class Items
{
public:
    explicit Items(std::shared_ptr<GameWrapper> game_wrapper);

    void InitSpecialEditionNames();
    void InitProductItemsCache();
    const ProductItemData& GetProductItemData(int item_id);
    void InitSlotsAndDefaultItems();
    [[nodiscard]] bool IsValidProductId(int product_id) const;
    [[nodiscard]] const SlotData& GetSlot(pluginsdk::Equipslot equip_slot) const;
    [[nodiscard]] const ProductItemData& GetDefaultItem(pluginsdk::Equipslot slot) const;
    [[nodiscard]] const ProductItemData& GetItemOrDefaultData(int product_id, pluginsdk::Equipslot slot) const;
    [[nodiscard]] const std::vector<ProductItemData>& GetSlotItems(pluginsdk::Equipslot slot);


    static inline const std::vector<pluginsdk::Equipslot> loadout_slots_ =
    {
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
    };

private:
    [[nodiscard]] ProductItemData GetItemDataImpl(ProductWrapper& prod) const;

    std::unordered_map<int, std::string> special_edition_names_;
    std::unordered_map<int, ProductItemData> all_items_;
    std::unordered_map<pluginsdk::Equipslot, std::vector<ProductItemData>> all_items_by_slot_;
    std::unordered_map<pluginsdk::Equipslot, ProductItemData> default_items_;
    std::unordered_map<pluginsdk::Equipslot, SlotData> equip_slots_;
    const ProductItemData null_item_{};

    std::shared_ptr<GameWrapper> gw_;
};
