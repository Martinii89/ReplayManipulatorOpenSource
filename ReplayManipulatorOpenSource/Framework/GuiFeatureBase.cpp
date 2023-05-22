#include "pch.h"
#include "GuiFeatureBase.h"

GuiFeatureBase::GuiFeatureBase(std::shared_ptr<GameWrapper> gw, std::string name,
                               const DefaultVisibility default_visibility)
    : gw_(std::move(gw)), name_{std::move(name)}
{
    switch (default_visibility)
    {
    case DefaultVisibility::kSettings:
        settings_header_enabled_ = true;
        break;
    case DefaultVisibility::kWindow:
        window_tab_enabled_ = true;
        break;
    case DefaultVisibility::kBoth:
        window_tab_enabled_ = true;
        settings_header_enabled_ = true;
        break;
    }
}

void GuiFeatureBase::OnGameThread(std::function<void()>&& func) const
{
    gw_->Execute([func = std::move(func)](...) {
        func();
    });
}
