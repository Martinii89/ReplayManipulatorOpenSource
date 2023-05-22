#include "pch.h"
#include "ImguiUtils.h"
#include "IMGUI/imgui_internal.h"

ImGui::Disable::Disable(const bool should_disable, const float alpha)
    : should_disable(should_disable)
{
    if (should_disable)
    {
        PushItemFlag(ImGuiItemFlags_Disabled, true);
        PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    }
}

ImGui::Disable::~Disable()
{
    if (should_disable)
    {
        PopItemFlag();
        PopStyleVar();
    }
}

ImGui::ScopeStyleColor::ScopeStyleColor(const ImGuiCol idx, const ImVec4& col, const bool active)
    : m_active(active)
{
    if (m_active)
    {
        PushStyleColor(idx, col);
    }
}

ImGui::ScopeStyleColor::~ScopeStyleColor()
{
    if (m_active)
    {
        PopStyleColor();
    }
}
