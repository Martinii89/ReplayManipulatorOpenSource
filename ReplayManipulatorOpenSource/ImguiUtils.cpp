#include "pch.h"
#include "ImguiUtils.h"
#include "IMGUI/imgui_internal.h"

#include <windows.h>
#include <shellapi.h>

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

void ImGui::AddUnderLine(const ImColor col)
{
    ImVec2 min = GetItemRectMin();
    ImVec2 max = GetItemRectMax();
    min.y = max.y;
    GetWindowDrawList()->AddLine(min, max, col, 1.0f);
}

void ImGui::TextUrl(const char* name, const char* url)
{
    PushStyleColor(ImGuiCol_Text, GetStyle().Colors[ImGuiCol_ButtonHovered]);
    TextUnformatted(name);
    PopStyleColor();
    if (IsItemHovered())
    {
        if (IsMouseClicked(0))
        {
            ::ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWDEFAULT);
        }
        AddUnderLine(GetStyle().Colors[ImGuiCol_ButtonHovered]);
        //ImGui::SetTooltip( ICON_FA_LINK "  Open in browser\n%s", URL_ );
        SetTooltip("Open in browser: %s", url);
    }
    else
    {
        AddUnderLine(GetStyle().Colors[ImGuiCol_Button]);
    }
}
