#pragma once

class GuiFeatureBase
{
public:
    enum class DefaultVisibility { kSettings, kWindow, kBoth };

    explicit GuiFeatureBase(std::shared_ptr<GameWrapper> gw, std::string name, DefaultVisibility default_visibility);

    GuiFeatureBase(const GuiFeatureBase& other) = delete;
    GuiFeatureBase(GuiFeatureBase&& other) noexcept = delete;
    GuiFeatureBase& operator=(const GuiFeatureBase& other) = delete;
    GuiFeatureBase& operator=(GuiFeatureBase&& other) noexcept = delete;
    virtual ~GuiFeatureBase() = default;

    virtual void Render() = 0;

    [[nodiscard]] virtual std::string GetName() const
    {
        return name_;
    }

    [[nodiscard]] virtual bool ShouldDrawPluginSettings() const
    {
        return settings_header_enabled_;
    }

    [[nodiscard]] virtual bool ShouldDrawPluginWindow() const
    {
        return window_tab_enabled_;
    }

protected:
    void OnGameThread(std::function<void()>&& func) const;

    std::shared_ptr<GameWrapper> gw_;
    std::string name_;
    bool settings_header_enabled_ = false;
    bool window_tab_enabled_ = false;
};
