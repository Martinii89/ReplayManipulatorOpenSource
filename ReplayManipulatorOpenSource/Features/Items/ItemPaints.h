#pragma once

class ItemPaints
{
public:
    struct ItemPaint
    {
        int attribute_id = 0;
        std::string name;
        std::vector<LinearColor> colors;
        unsigned char /*U_Types_TA_EPaintFinishType*/ type;
        bool is_custom = false;
    };

    explicit ItemPaints(std::shared_ptr<GameWrapper> gw);

    [[nodiscard]] PaintDatabaseWrapper GetPaintDb() const;
    [[nodiscard]] bool AddCustomColor(const std::string& label, const LinearColor& color);
    [[nodiscard]] bool DrawAddCustomPaintColorWidget();
    void SetUpdatingColor(const std::string& name, const LinearColor& color);
    [[nodiscard]] bool DrawUpdateCustomPaintColorWidget();
    [[nodiscard]] bool UpdateCustomColor(const std::string& name, const LinearColor& color);
    [[nodiscard]] const std::map<int, ItemPaint>& GetItemPaints() const;

private:
    void RefreshPaintCache();
    std::map<int, ItemPaint> paint_cache_;
    std::shared_ptr<GameWrapper> game_wrapper_;

    LinearColor new_custom_color_{0, 0, 0, 1};
    std::string new_custom_name_;
};
