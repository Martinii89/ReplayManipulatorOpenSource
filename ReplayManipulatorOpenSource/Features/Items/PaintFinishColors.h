#pragma once
#include "bakkesmod/core/loadout_structs.h"

class PaintFinishColors
{
public:
    struct GridSelectResult
    {
        bool selected = false;
        pluginsdk::PaintFinishColor color{};
    };

    PaintFinishColors();

    [[nodiscard]] static GridSelectResult DrawColorSelectGrid(const std::vector<pluginsdk::PaintFinishColor>&);

    [[nodiscard]] const std::vector<pluginsdk::PaintFinishColor>& GetBlueColorSet() const;
    [[nodiscard]] const std::vector<pluginsdk::PaintFinishColor>& GetOrangeColorSet() const;
    [[nodiscard]] const std::vector<pluginsdk::PaintFinishColor>& GetCustomColorSet() const;

private:
    std::vector<pluginsdk::PaintFinishColor> team_blue_;
    std::vector<pluginsdk::PaintFinishColor> team_orange_;
    std::vector<pluginsdk::PaintFinishColor> custom_;
};
