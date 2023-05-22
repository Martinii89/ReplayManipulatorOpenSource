#include "pch.h"
#include "StadiumManager.h"
#include "ImguiUtils.h"
#include "bakkesmod/wrappers/PluginManagerWrapper.h"
#include <utility>

constexpr int kColorChangerId = 150;

struct CvarNames
{
    static inline const char* enabled = "Colors_Enabled";
    static inline const char* enabled_cars = "Colors_Enabled_Cars";

    struct TeamColor
    {
        const char* primary;
        const char* secondary;
    };

    static inline constexpr TeamColor blue{"Colors_Preference_Blue_Primary", "Colors_Preference_Blue_Secondary"};
    static inline constexpr TeamColor orange{"Colors_Preference_Orange_Primary", "Colors_Preference_Orange_Secondary"};
};

StadiumManager::StadiumManager(std::shared_ptr<GameWrapper> gw, std::shared_ptr<CVarManagerWrapper> cw_wrapper)
    : GuiFeatureBase{std::move(gw), "Stadium Colors", DefaultVisibility::kWindow},
      cvar_manager_(std::move(cw_wrapper)), plugin_installed_(IsColorChangerInstalled()),
      force_default_colors_(GetForceDefaultColors())
{
    gw_->HookEventPost("Function TAGame.GFxData_Settings_TA.SetBooleanValue", [this](...) {
        RecheckForcedColors();
    });
}

bool StadiumManager::IsColorChangerInstalled() const
{
    auto plugin_manager = gw_->GetPluginManager();
    auto* loaded_plugins = plugin_manager.GetLoadedPlugins();
    return std::ranges::any_of(*loaded_plugins, [](const std::shared_ptr<BakkesMod::Plugin::LoadedPlugin>& plugin) {
        return strcmp(plugin->_details->className, "ColorChanger") == 0;
    });
}

void StadiumManager::InstallColorChanger() const
{
    if (cvar_manager_)
    {
        cvar_manager_->executeCommand(std::format("bpm_install {}", kColorChangerId));
    }
    else
    {
        LOG("Error: Null CVarManagerWrapper in StadionManager");
    }
}

void StadiumManager::DrawInstallGui()
{
    ImGui::Text("Stadium module not loaded. Install/load the ColorChanger plugin to make this work");
    ImGui::Text("You may have to reload the replay after enabling the ColorChanging plugin");
    if (ImGui::Button("Install and load ColorChanger"))
    {
        InstallColorChanger();
        gw_->SetTimeout(
            [this](...) {
                cvar_manager_->executeCommand("plugin load colorchanger");
                if (IsColorChangerInstalled())
                {
                    plugin_installed_ = true;
                }
            },
            2);
    }
    ImGui::SameLine();
    HelpMarker("Installs and rechecks if ColorChanger is installed");

    if (ImGui::Button("Load ColorChanger"))
    {
        cvar_manager_->executeCommand("plugin load colorchanger");
    }
}

LinearColor StadiumManager::GetBlueColor(const ColorSlot& slot) const
{
    const auto* const cvar = (slot == ColorSlot::kPrimary) ? CvarNames::blue.primary : CvarNames::blue.secondary;
    return GetCvarColorValue(cvar);
}

void StadiumManager::SetBlueColor(const LinearColor& color, const ColorSlot& slot) const
{
    const auto* cvar = (slot == ColorSlot::kPrimary) ? CvarNames::blue.primary : CvarNames::blue.secondary;
    SetCvarColorValue(cvar, color);
}

LinearColor StadiumManager::GetOrangeColor(const ColorSlot& slot) const
{
    const auto* cvar = (slot == ColorSlot::kPrimary) ? CvarNames::orange.primary : CvarNames::orange.secondary;
    return GetCvarColorValue(cvar);
}

void StadiumManager::SetOrangeColor(const LinearColor& color, const ColorSlot& slot) const
{
    const auto* cvar = (slot == ColorSlot::kPrimary) ? CvarNames::orange.primary : CvarNames::orange.secondary;
    SetCvarColorValue(cvar, color);
}

void StadiumManager::SetEnabled(const bool enable) const
{
    SetCvarBoolValue(CvarNames::enabled, enable);
    if (!enable)
    {
        SetEnabledCarColor(false);
    }
}

bool StadiumManager::GetEnabled() const
{
    return GetCvarBoolValue(CvarNames::enabled);
}

void StadiumManager::RecheckForcedColors()
{
    force_default_colors_ = GetForceDefaultColors();
}

void StadiumManager::SetEnabledCarColor(bool enable) const
{
    if (!GetEnabled())
    {
        enable = false;
    }
    SetCvarBoolValue(CvarNames::enabled_cars, enable);
}

bool StadiumManager::GetEnabledCarColor() const
{
    return GetCvarBoolValue(CvarNames::enabled_cars);
}

void StadiumManager::Render()
{
    if (!plugin_installed_)
    {
        DrawInstallGui();
        return;
    }

    if (force_default_colors_)
    {
        ImGui::TextColored({1, 0, 0, 1}, "Force Default Colors is on");
        if (ImGui::Button("Turn it off"))
        {
            gw_->Execute([this](...) {
                SetForceDefaultColors(false);
            });
        }
    }

    auto disable_cause_forced_colors = ImGui::Disable(force_default_colors_);

    auto enabled = GetEnabled();
    if (ImGui::Checkbox("Enabled", &enabled))
    {
        SetEnabled(enabled);
    }
    ImGui::SameLine();
    auto enabled_cars = GetEnabledCarColor();
    if (ImGui::Checkbox("Enable car colors", &enabled_cars))
    {
        SetEnabledCarColor(enabled_cars);
    }

    auto blue_team_primary = GetBlueColor(ColorSlot::kPrimary);
    if (ImGui::ColorEdit4("Blue Primary Color", &blue_team_primary.R, ImGuiColorEditFlags_NoInputs))
    {
        SetBlueColor(blue_team_primary, ColorSlot::kPrimary);
    }
    ImGui::SameLine();

    auto blue_team_secondary = GetBlueColor(ColorSlot::kSecondary);
    if (ImGui::ColorEdit4("Blue Secondary Color", &blue_team_secondary.R, ImGuiColorEditFlags_NoInputs))
    {
        SetBlueColor(blue_team_secondary, ColorSlot::kSecondary);
    }

    auto orange_team_primary = GetOrangeColor(ColorSlot::kPrimary);
    if (ImGui::ColorEdit4("Orange Primary Color", &orange_team_primary.R, ImGuiColorEditFlags_NoInputs))
    {
        SetOrangeColor(orange_team_primary, ColorSlot::kPrimary);
    }
    ImGui::SameLine();

    auto orange_team_secondary = GetOrangeColor(ColorSlot::kSecondary);
    if (ImGui::ColorEdit4("Orange Secondary Color", &orange_team_secondary.R, ImGuiColorEditFlags_NoInputs))
    {
        SetOrangeColor(orange_team_secondary, ColorSlot::kSecondary);
    }
}

CVarWrapper StadiumManager::GetValidCvar(const std::string& cvar_name) const
{
    auto cvar = cvar_manager_->getCvar(cvar_name);
    if (cvar.IsNull())
    {
        LOG("Failed to get the cvar: {}", cvar_name);
        throw std::exception(std::format("Failed to get the cvar: {}", cvar_name).c_str());
    }
    return cvar;
}

LinearColor StadiumManager::GetCvarColorValue(const std::string& cvar_name, const bool convert_to_float) const
{
    auto cvar = GetValidCvar(cvar_name);
    auto v = cvar.getColorValue();
    if (convert_to_float)
    {
        v /= 255;
    }
    return v;
}

void StadiumManager::SetCvarColorValue(const std::string& cvar_name, const LinearColor& color,
                                       const bool convert_from_float) const
{
    auto cvar = GetValidCvar(cvar_name);
    if (convert_from_float)
    {
        const auto scaled_value = color * 255;
        cvar.setValue(scaled_value);
    }
    else
    {
        cvar.setValue(color);
    }
}

bool StadiumManager::GetCvarBoolValue(const std::string& cvar_name) const
{
    return GetValidCvar(cvar_name).getBoolValue();
}

void StadiumManager::SetCvarBoolValue(const std::string& cvar_name, const bool value) const
{
    GetValidCvar(cvar_name).setValue(value);
}

bool StadiumManager::GetForceDefaultColors() const
{
    const auto settings = gw_->GetSettings();
    return settings.GetForceDefaultColors();
}

void StadiumManager::SetForceDefaultColors(const bool value) const
{
    auto settings = gw_->GetSettings();
    settings.SetForceDefaultColors(value);
}
