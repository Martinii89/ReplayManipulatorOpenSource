#include "pch.h"
#include "Items.h"

#include <ranges>

SlotData SlotData::FromSlotWrapper(ProductSlotWrapper& slot_wrapper)
{
    return {slot_wrapper.GetSlotIndex(), static_cast<pluginsdk::Equipslot>(slot_wrapper.GetSlotIndex()),
            slot_wrapper.GetOnlineLabel().ToString()};
}

Items::Items(std::shared_ptr<GameWrapper> game_wrapper)
    : gw_(std::move(game_wrapper))
{
    InitSpecialEditionNames();
    InitProductItemsCache();
    InitSlotsAndDefaultItems();

    LOG("Products cache initialized with {} items", all_items_.size());
}

void Items::InitSpecialEditionNames()
{
    auto items_wrapper = gw_->GetItemsWrapper();
    if (items_wrapper.IsNull())
    {
        LOG("no items wrapper?!");
        return;
    }
    auto products = items_wrapper.GetAllProducts();
    for (auto prod : products)
    {
        if (!prod)
        {
            continue;
        }
        for (auto attribute : prod.GetAttributes())
        {
            if (!attribute || attribute.GetAttributeType() != "ProductAttribute_SpecialEditionSettings_TA")
            {
                continue;
            }
            for (const auto& [productId, editionId, label] : ProductAttribute_SpecialEditionSettingsWrapper(
                     attribute.memory_address).GetEditions())
            {
                if (label == "Flare")
                {
                    continue;
                }
                auto prod_id = productId;
                special_edition_names_.emplace(prod_id, label);
            }
            break;
        }
    }
}

void Items::InitProductItemsCache()
{
    auto items_wrapper = gw_->GetItemsWrapper();
    if (items_wrapper.IsNull())
    {
        LOG("no items wrapper?!");
        return;
    }

    for (auto prod_wrapper : items_wrapper.GetAllProducts())
    {
        if (!prod_wrapper)
        {
            continue;
        }
        auto item = GetItemDataImpl(prod_wrapper);
        all_items_[item.id] = item;
        all_items_by_slot_[item.slot.equip_slot].push_back(item);
    }

    for (auto slot : all_items_by_slot_ | std::views::keys)
    {
        // empty "un-equip" item
        all_items_by_slot_[slot].push_back({});
        //Used in GUI. It's better for the user if it's sorted..
        std::ranges::sort(all_items_by_slot_[slot], {}, &ProductItemData::name);
    }
}

const ProductItemData& Items::GetProductItemData(const int item_id)
{
    if (const auto it = all_items_.find(item_id); it != all_items_.end())
    {
        return it->second;
    }

    throw std::invalid_argument("Unknown item " + std::to_string(item_id));
}

void Items::InitSlotsAndDefaultItems()
{
    auto items_wrapper = gw_->GetItemsWrapper();
    if (items_wrapper.IsNull())
    {
        LOG("no items wrapper?!");
        return;
    }

    for (auto slot : items_wrapper.GetAllProductSlots())
    {
        auto slot_data = SlotData::FromSlotWrapper(slot);
        //m_productSlots[slotData.equipSlot] = slot;
        equip_slots_[slot_data.equip_slot] = slot_data;

        if (auto default_item = slot.GetDefaultProduct())
        {
            if (const auto item_id = default_item.GetID(); item_id != 0)
            {
                try
                {
                    const auto& item_data = GetProductItemData(item_id);
                    default_items_[slot_data.equip_slot] = item_data;
                }
                catch (std::invalid_argument&)
                {
                    DEBUGLOG("Default item for slot {} was invalid (id: {})", slot_data.slot_id, item_id);
                }
            }
        }
    }
}

bool Items::IsValidProductId(const int product_id) const
{
    return all_items_.contains(product_id);
}

const SlotData& Items::GetSlot(const pluginsdk::Equipslot equip_slot) const
{
    if (const auto it = equip_slots_.find(equip_slot); it != equip_slots_.end())
    {
        return it->second;
    }

    throw std::invalid_argument("Invalid slot");
}

const ProductItemData& Items::GetDefaultItem(const pluginsdk::Equipslot slot) const
{
    if (const auto it = default_items_.find(slot); it != default_items_.end())
    {
        return it->second;
    }
    return null_item_;
}

const ProductItemData& Items::GetItemOrDefaultData(const int product_id, const pluginsdk::Equipslot slot) const
{
    if (const auto it = all_items_.find(product_id); it != all_items_.end())
    {
        return it->second;
    }
    return GetDefaultItem(slot);
}

const std::vector<ProductItemData>& Items::GetSlotItems(const pluginsdk::Equipslot slot)
{
    if (const auto it = all_items_by_slot_.find(slot); it != all_items_by_slot_.end())
    {
        return it->second;
    }

    throw std::invalid_argument("Unknown slot");
}

ProductItemData Items::GetItemDataImpl(ProductWrapper& prod) const
{
    ProductItemData item;
    item.id = prod.GetID();
    item.name = prod.GetLongLabel().ToString();

    if (const auto it = special_edition_names_.find(item.id); it != special_edition_names_.end())
    {
        item.name = std::format("{}: {}", item.name, it->second);
    }

    item.paintable = prod.IsPaintable();
    item.can_equip = prod.CanEquip();

    for (auto attribute : prod.GetAttributes())
    {
        if (!attribute)
        {
            continue;
        }
        if (attribute.GetAttributeType() == "ProductAttribute_TeamEditionUpload_TA")
        {
            if (auto team_attribute = ProductAttribute_TeamEditionUploadWrapper(attribute.memory_address))
            {
                item.item_can_take_esport_attribute = true;
                item.name += " (org wheel)";
            }
        }
        if (attribute.GetAttributeType() == "UProductAttribute_BodyCompatibility_TA")
        {
            if (auto body_compatibility = ProductAttribute_BodyCompatibilityWrapper(attribute.memory_address))
            {
                item.body_restrictions = true;
                for (auto body : body_compatibility.GetCompatibleBodies())
                {
                    item.compatible_bodies.insert(body.GetID());
                }
            }
        }
    }

    if (auto required_prodcut = prod.GetRequiredProduct())
    {
        item.body_restrictions = true;
        item.compatible_bodies.insert(required_prodcut.GetID());
    }

    if (auto slot_wrapper = prod.GetSlot())
    {
        item.slot = SlotData::FromSlotWrapper(slot_wrapper);
    }

    return item;
}
