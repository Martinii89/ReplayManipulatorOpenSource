#include "pch.h"
#include "ItemPaints.h"

#include "ImguiUtils.h"

ItemPaints::ItemPaints(std::shared_ptr<GameWrapper> gw)
    : game_wrapper_(std::move(gw))
{
    RefreshPaintCache();
}

PaintDatabaseWrapper ItemPaints::GetPaintDb() const
{
    auto items_wrapper = game_wrapper_->GetItemsWrapper();

    if (!items_wrapper)
    {
        LOG("no items wrapper");
        return {0};
    }

    return items_wrapper.GetPaintDB();
}

bool ItemPaints::AddCustomColor(const std::string& label, const LinearColor& color)
{
    auto paint_db = GetPaintDb();
    if (!paint_db)
        return false;

    if (!paint_db.AddCustomPaintColor(label, color))
    {
        DEBUGLOG("failed adding custom paint");
        return false;
    }

    RefreshPaintCache();
    return true;
}

bool ItemPaints::DrawAddCustomPaintColorWidget()
{
    bool done = false;
    ImGui::TextUnformatted("Add custom paint color");
    ImGui::InputText("Paint Name", &new_custom_name_);
    ImGui::ColorPicker3("Color", &new_custom_color_.R, ImGuiColorEditFlags__OptionsDefault | ImGuiColorEditFlags_PickerHueWheel);
    ImGui::SliderFloat("Metallic <-> Matt", &new_custom_color_.A, 0.f, 1.f);
    if (ImGui::Button("Add"))
    {
        game_wrapper_->Execute([this, name = new_custom_name_, color = new_custom_color_](...) {
            auto _ = AddCustomColor(name, color);
        });
        done = true;
    }
    return done;
}

void ItemPaints::SetUpdatingColor(const std::string& name, const LinearColor& color)
{
    new_custom_color_ = color;
    new_custom_name_ = name;
}

bool ItemPaints::DrawUpdateCustomPaintColorWidget()
{
    bool done = false;
    ImGui::TextUnformatted("Update custom paint color");
    {
        ImGui::Disable const d{true};
        ImGui::InputText("Paint Name", &new_custom_name_);
    }
    ImGui::ColorPicker3("Color", &new_custom_color_.R, ImGuiColorEditFlags__OptionsDefault | ImGuiColorEditFlags_PickerHueWheel);
    ImGui::SliderFloat("Metallic <-> Matt", &new_custom_color_.A, 0.f, 1.f);
    if (ImGui::Button("Update"))
    {
        game_wrapper_->Execute([this, name = new_custom_name_, color = new_custom_color_](...) {
            auto _ = UpdateCustomColor(name, color);
        });
        done = true;
    }
    return done;
}

bool ItemPaints::UpdateCustomColor(const std::string& name, const LinearColor& color)
{
    auto paint_db = GetPaintDb();
    if (!paint_db)
        return false;

    if (!paint_db.UpdateCustomPaintColor(name, color))
    {
        DEBUGLOG("failed adding custom paint");
        return false;
    }

    RefreshPaintCache();
    return true;
}


const std::map<int, ItemPaints::ItemPaint>& ItemPaints::GetItemPaints() const
{
    return paint_cache_;
}

void ItemPaints::RefreshPaintCache()
{
    auto items_wrapper = game_wrapper_->GetItemsWrapper();

    if (!items_wrapper)
    {
        LOG("no items wrapper");
        return;
    }

    auto paint_db = items_wrapper.GetPaintDB();

    if (!paint_db)
    {
        LOG("no paint db");
        return;
    }

    paint_cache_.clear();

    auto paints = paint_db.GetPaints();

    for (int i = 0; i < paints.Count(); ++i)
    {
        auto paint_wrapper = paints.Get(i);
        if (!paint_wrapper)
        {
            continue;
        }
        ItemPaint const paint =
        {
            .attribute_id = i,
            .name = paint_wrapper.GetLabel(),
            .colors = paint_wrapper.GetColors(),
            .type = paint_wrapper.GetFinishType(),
            .is_custom = paint_wrapper.GetId() == 0
        };
        paint_cache_.emplace(paint.attribute_id, paint);
    }

    paint_cache_[0] = ItemPaint
    {
        .attribute_id = 0,
        .name = "Unpainted",
        .colors = {{0, 0, 0, 0}},
        .type = 0 /*EPaintFinishType_PaintFinishType_Standard */
    };
}
