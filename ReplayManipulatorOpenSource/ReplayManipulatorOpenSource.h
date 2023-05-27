#pragma once

#include <memory>
#include <memory>

#include "GuiBase.h"
#include "Data/PriData.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"

#include "version.h"
#include "Features/CamPathsManager/CamPathsManager.h"
#include "Features/Credits/Credits.h"
#include "Features/CustomTextures/CustomTextures.h"

#include <memory>
#include <bakkesmod/wrappers/GameObject/MeshComponents/CarMeshComponentBaseWrapper.h>

class GuiFeatureBase;
class PlayerRenamer;
class BallHiderAndDecals;
class StadiumManager;
class CameraSettingsOverride;
class ReplayMapChanger;
class SlowmotionTransitionFixerUtility;
class ItemPaints;
class PaintFinishColors;
class CameraFocus;
class LoadoutEditor;
class Items;
class ReplayManager;

// ReSharper disable once CppInconsistentNaming
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "."
    stringify(VERSION_BUILD);


class ReplayManipulatorOpenSource
    : public BakkesMod::Plugin::BakkesModPlugin
      , public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
      , public PluginWindowBase   // Uncomment if you want to render your own plugin window
{
public:
    //std::shared_ptr<bool> enabled;

    //Boilerplate
    void onLoad() override;

    void OnReplayOpen();
    void OnReplayClose() const;

    //void onUnload() override; // Uncomment and implement if you need a unload method

    void OnGameThread(std::function<void()>&& func) const;

    void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
    void RenderWindow() override;   // Uncomment if you want to render your own plugin window

    void DrawPriData(PriData& pri);

    [[nodiscard]] PriData* GetPriData(PriWrapper& pri);
    void OnPriLoadoutSet(PriWrapper& pri);
    void RefreshPriData();
    [[nodiscard]] PriWrapper GetPriWrapper(const PriData& pri_data) const;
    void UpdateLoadout(const PriData& pri_data) const;
    void ApplyDecal(const PriData& pri_data) const;
    void CameraLock() const;
    void ApplyCarHiddenState(const PriData& pri_data) const;
    void OnMaterialInit(const CarMeshComponentBaseWrapper& car_mesh_component);
    void OnSetMeshMaterialColors(CarWrapper& car_wrapper);

private:
    //std::shared_ptr<BakkesModEventDispatcher> event_dispatcher_;

    // Plugin data
    std::vector<PriData> replay_players_;
    std::vector<PriData> replay_players_originals_;
    std::string current_replay_id_;
    std::string current_replay_name_;

    // custom decal TODO: extract
    std::map<std::string, CustomDecal> loaded_custom_decals_;
    [[nodiscard]] CustomDecal& FindCustomDecal(const std::string& name);
    TextureJson custom_decal_configs_;
    void ReadJsons();


    // modules
    template <typename T, typename... Args>
    [[nodiscard]] std::shared_ptr<T> CreateModule(Args&&... args)
    {
        std::shared_ptr<T> created = std::make_shared<T>(std::forward<Args>(args)...);
        if (auto gui_feature = std::dynamic_pointer_cast<GuiFeatureBase>(created))
        {
            gui_features_.emplace_back(gui_feature);
        }
        return created;
    }

    void InitUtilityModules();

    std::vector<std::shared_ptr<GuiFeatureBase>> gui_features_;

    std::shared_ptr<Items> items_;
    std::shared_ptr<LoadoutEditor> loadout_editor_;
    std::shared_ptr<PaintFinishColors> paint_finish_colors_;
    std::shared_ptr<ItemPaints> item_paints_;
    std::shared_ptr<CameraFocus> camera_focus_;
    std::shared_ptr<SlowmotionTransitionFixerUtility> slowmotion_fixer_;
    std::shared_ptr<ReplayMapChanger> map_changer_;
    std::shared_ptr<CameraSettingsOverride> camera_settings_;
    std::shared_ptr<StadiumManager> stadium_manager_;
    std::shared_ptr<BallHiderAndDecals> ball_hider_;
    std::shared_ptr<PlayerRenamer> player_rename_;
    std::shared_ptr<ReplayManager> replay_manager_;
    std::shared_ptr<TextureCache> texture_cache_;
    std::shared_ptr<CamPathsManager> dollycam_manager_;
    std::shared_ptr<CreditsInSettings> credits_;
};
