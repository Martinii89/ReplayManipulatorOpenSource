#include "pch.h"
#include "PaintFinishColors.h"

#include "ImguiUtils.h"
#include "bakkesmod/utilities/LoadoutUtilities.h"

PaintFinishColors::GridSelectResult PaintFinishColors::DrawColorSelectGrid(
    const std::vector<pluginsdk::PaintFinishColor>& color_list)
{
    int row = -1;
    GridSelectResult selected_color{.selected = false};
    for (const auto& [color, pos, index] : color_list)
    {
        ImGui::ScopeId const color_id(std::format("{}:{}", pos.column, pos.row));
        if (pos.row == row)
            ImGui::SameLine();
        row = pos.row;
        if (ImGui::ColorButton("Color", color))
        {
            selected_color.selected = true;
            selected_color.color = {color, pos, index};
        }
    }
    return selected_color;
}

PaintFinishColors::PaintFinishColors()
{
    team_blue_ = LoadoutUtilities::GetBlueColorSet();
    team_orange_ = LoadoutUtilities::GetOrangeColorSet();
    custom_ = LoadoutUtilities::GetCustomColorSet();
}

const std::vector<pluginsdk::PaintFinishColor>& PaintFinishColors::GetBlueColorSet() const
{
    return team_blue_;
}

const std::vector<pluginsdk::PaintFinishColor>& PaintFinishColors::GetOrangeColorSet() const
{
    return team_orange_;
}

const std::vector<pluginsdk::PaintFinishColor>& PaintFinishColors::GetCustomColorSet() const
{
    return custom_;
}
