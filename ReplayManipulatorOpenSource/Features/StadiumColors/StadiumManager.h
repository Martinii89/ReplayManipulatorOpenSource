#pragma once
#include "Framework/GuiFeatureBase.h"

class StadiumManager final : public GuiFeatureBase
{
public:
    explicit StadiumManager(std::shared_ptr<GameWrapper> gw, std::shared_ptr<CVarManagerWrapper> cw_wrapper);

    void Render() override;

private:
    enum class ColorSlot { kPrimary, kSecondary };

    void DrawInstallGui();

    [[nodiscard]] bool IsColorChangerInstalled() const;
    void InstallColorChanger() const;
    [[nodiscard]] LinearColor GetBlueColor(const ColorSlot& slot) const;
    void SetBlueColor(const LinearColor& color, const ColorSlot& slot) const;
    [[nodiscard]] LinearColor GetOrangeColor(const ColorSlot& slot) const;
    void SetOrangeColor(const LinearColor& color, const ColorSlot& slot) const;

    void SetEnabled(bool enable) const;
    [[nodiscard]] bool GetEnabled() const;
    void SetEnabledCarColor(bool enable) const;
    [[nodiscard]] bool GetEnabledCarColor() const;
    [[nodiscard]] CVarWrapper GetValidCvar(const std::string& cvar_name) const;

    [[nodiscard]] LinearColor GetCvarColorValue(const std::string& cvar_name, bool convert_to_float = true) const;
    void SetCvarColorValue(const std::string& cvar_name, const LinearColor& color,
                           bool convert_from_float = true) const;

    [[nodiscard]] bool GetCvarBoolValue(const std::string& cvar_name) const;
    void SetCvarBoolValue(const std::string& cvar_name, bool value) const;

    void RecheckForcedColors();
    [[nodiscard]] bool GetForceDefaultColors() const;
    void SetForceDefaultColors(bool value) const;

    std::shared_ptr<CVarManagerWrapper> cvar_manager_;
    bool plugin_installed_ = false;
    bool force_default_colors_ = false;
};
