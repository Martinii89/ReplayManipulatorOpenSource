#include "pch.h"
#include "ReplayManipulatorOpenSource.h"

#include "ImguiUtils.h"
#include "Data/PriData.h"
#include "Features/TextureCache.h"
#include "bakkesmod/utilities/LoadoutUtilities.h"
#include "bakkesmod/wrappers/GameObject/MeshComponents/CarMeshComponentBaseWrapper.h"
#include "Features/BallHide/BallHiderAndDecals.h"
#include "Features/CamPathsManager/CamPathsManager.h"
#include "Features/Camera/CameraFocus.h"
#include "Features/Camera/CameraSettingsOverride.h"
#include "Features/CarRotator/CarRotator.h"
#include "Features/Credits/Credits.h"
#include "Features/CustomTextures/CustomTextures.h"
#include "Features/Items/ItemPaints.h"
#include "Features/Items/Items.h"
#include "Features/Items/LoadoutEditor.h"
#include "Features/Items/PaintFinishColors.h"
#include "Features/MapChange/ReplayMapChanger.h"
#include "Features/Names/PlayerRenamer.h"
#include "Features/ReplayManager/ReplayManager.h"
#include "Features/SlowMotionTransitionFixer/Stfu.h"
#include "Features/StadiumColors/StadiumManager.h"
#include "Framework/GuiFeatureBase.h"

BAKKESMOD_PLUGIN(ReplayManipulatorOpenSource, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

// ReSharper disable once CppInconsistentNaming
std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void ReplayManipulatorOpenSource::onLoad()
{
    _globalCvarManager = cvarManager;

    InitUtilityModules();

    stadium_manager_ = CreateModule<StadiumManager>(gameWrapper, cvarManager);
    replay_manager_ = CreateModule<ReplayManager>(gameWrapper, cvarManager);
    camera_focus_ = std::make_shared<CameraFocus>(gameWrapper);
    slowmotion_fixer_ = CreateModule<SlowmotionTransitionFixerUtility>(gameWrapper);
    camera_settings_ = std::make_shared<CameraSettingsOverride>(gameWrapper);
    map_changer_ = CreateModule<ReplayMapChanger>(gameWrapper);

    auto custom_ball_decals_folder = gameWrapper->GetDataFolder() / "acplugin" / "BallTextures";
    ball_hider_ = CreateModule<BallHiderAndDecals>(gameWrapper, texture_cache_, custom_ball_decals_folder);
    player_rename_ = std::make_shared<PlayerRenamer>(gameWrapper);
    dollycam_manager_ = CreateModule<CamPathsManager>(gameWrapper, cvarManager,
                                                      gameWrapper->GetDataFolder() / "campaths");
    credits_ = CreateModule<CreditsInSettings>(gameWrapper);

    items_ = std::make_shared<Items>(gameWrapper);
    paint_finish_colors_ = std::make_shared<PaintFinishColors>();
    item_paints_ = std::make_shared<ItemPaints>(gameWrapper);

    loadout_editor_ = std::make_shared<LoadoutEditor>(items_, paint_finish_colors_, item_paints_);

    loadout_editor_->LoadProductIcons(gameWrapper->GetDataFolder() / "ReplayManipulatorOS" / "slots");

    gameWrapper->HookEventPost("Function TAGame.GameInfo_Replay_TA.HandleReplayImported", [this](...) {
        gameWrapper->HookEventPost("Function TAGame.GameInfo_Replay_TA.EventGameEventSet", [this](...) {
            OnReplayOpen();
            gameWrapper->UnhookEventPost("Function TAGame.GameInfo_Replay_TA.EventGameEventSet");
        });
    });

    gameWrapper->HookEvent("Function TAGame.GFxHUD_Replay_TA.Destroyed", [this](...) {
        OnReplayClose();
    });

    if (gameWrapper->IsInReplay())
    {
        OnReplayOpen();
    }

    ReadJsons();
}

void ReplayManipulatorOpenSource::InitUtilityModules()
{
    texture_cache_ = std::make_shared<TextureCache>(gameWrapper);
    //event_dispatcher_ = std::make_shared<BakkesModEventDispatcher>(this);
}


void ReplayManipulatorOpenSource::OnReplayOpen()
{
    auto game_event = gameWrapper->GetGameEventAsReplay();
    if (!game_event)
    {
        return;
    }
    map_changer_->UpdateCurrentMap();
    auto replay = game_event.GetReplay();
    if (!replay)
    {
        return;
    }
    auto replay_id = replay.GetId().ToString();
    if (replay_id == current_replay_id_)
    {
        LOG("Same replay reloaded");
    }
    else
    {
        LOG("Loading replay with ID: {}", replay_id);
        replay_players_.clear();
        replay_players_originals_.clear();
    }
    current_replay_id_ = replay_id;
    current_replay_name_ = replay.GetReplayName().ToString();

    auto lodout_set_cb = [this](PriWrapper&& pri, void*, const std::string& name) {
        OnPriLoadoutSet(pri);
    };
    gameWrapper->HookEventWithCaller<PriWrapper>("Function TAGame.PRI_TA.OnLoadoutsSetInternal", lodout_set_cb);
    gameWrapper->HookEventWithCaller<PriWrapper>("Function TAGame.PRI_TA.OnLoadoutsSet", lodout_set_cb);
    gameWrapper->HookEventWithCaller<PriWrapper>("Function TAGame.PRI_TA.HandleLoadoutLoaded", lodout_set_cb);
    gameWrapper->HookEventWithCaller<PriWrapper>("Function TAGame.PRI_TA.UpdateFromLoadout", lodout_set_cb);

    gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.CarMeshComponentBase_TA.InitMaterials",
                                                       [this](const ActorWrapper& caller, ...) {
                                                           OnMaterialInit(
                                                               CarMeshComponentBaseWrapper{caller.memory_address});
                                                       });

    gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.UpdateTeamLoadout",
                                                 [this](CarWrapper&& car, ...) {
                                                     if (auto pri = car.GetPRI())
                                                     {
                                                         OnPriLoadoutSet(pri);
                                                     }
                                                 });

    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.CarMeshComponentBase_TA.SetMeshMaterialColors",
                                                   [this](ActorWrapper&& car_mesh, ...) {
                                                       if (auto car = CarMeshComponentBaseWrapper(car_mesh.memory_address).GetCar())
                                                       {
                                                           OnSetMeshMaterialColors(car);
                                                       }
                                                   });

    gameWrapper->HookEvent("Function TAGame.PlayerInput_TA.PlayerInput", [this](...) {
        CameraLock();
    });

    gameWrapper->SetTimeout([this](...) {
        RefreshPriData();
    }, 2);
}

void ReplayManipulatorOpenSource::OnReplayClose() const
{
    gameWrapper->UnhookEvent("Function TAGame.PRI_TA.OnLoadoutsSetInternal");
    gameWrapper->UnhookEvent("Function TAGame.PRI_TA.OnLoadoutsSet");
    gameWrapper->UnhookEvent("Function TAGame.PRI_TA.HandleLoadoutLoaded");
    gameWrapper->UnhookEvent("Function TAGame.PRI_TA.UpdateFromLoadout");
    gameWrapper->UnhookEvent("Function TAGame.Car_TA.UpdateTeamLoadout");
    gameWrapper->UnhookEvent("Function TAGame.PlayerInput_TA.PlayerInput");
    gameWrapper->UnhookEventPost("Function TAGame.CarMeshComponentBase_TA.InitMaterials");
    gameWrapper->UnhookEvent("Function TAGame.CarMeshComponentBase_TA.SetMeshMaterialColors");
}

void ReplayManipulatorOpenSource::OnGameThread(std::function<void()>&& func) const
{
    gameWrapper->Execute([func = std::move(func)](...) {
        func();
    });
}

void ReplayManipulatorOpenSource::RenderSettings()
{
    if (ImGui::Button("open window"))
    {
        OnGameThread([this] {
            cvarManager->executeCommand(std::format("openmenu {}", GetMenuName()));
        });
    }
    for (const auto& gui_feature_base : gui_features_)
    {
        if (gui_feature_base->ShouldDrawPluginSettings())
        {
            if (ImGui::CollapsingHeader(gui_feature_base->GetName().c_str()))
            {
                ImGui::Indent();
                ImGui::ScopeId const module_scope{gui_feature_base->GetName()};
                gui_feature_base->Render();
                ImGui::Unindent();
            }
        }
    }
}

void ReplayManipulatorOpenSource::DrawPriData(PriData& pri)
{
    if (player_rename_)
    {
        static std::string new_name;
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("New name", &new_name);
        ImGui::SameLine();
        {
            ImGui::Disable const disable_if_no_name{new_name.empty()};
            if (ImGui::Button("Change the name"))
            {
                gameWrapper->Execute([this, pri, name = new_name](...) {
                    auto pri_wrapper = GetPriWrapper(pri);
                    player_rename_->Rename(pri_wrapper, name);
                });
            }
        }
        {
            ImGui::Disable const disable_if_not_changed{!player_rename_->IsInRenameCache(pri.uid)};
            ImGui::SameLine();
            if (ImGui::Button("Restore"))
            {
                gameWrapper->Execute([this, pri](...) {
                    auto pri_wrapper = GetPriWrapper(pri);
                    player_rename_->Restore(pri_wrapper);
                });
            }
        }
    }

    if (ImGui::Checkbox("Hidden", &pri.hidden))
    {
        OnGameThread([this, pri]() mutable {
            ApplyCarHiddenState(pri);
        });
    }
    ImGui::SameLine();
    if (ImGui::Button("Focus camera"))
    {
        OnGameThread([this, id = pri.uid.pri_id_string] {
            camera_focus_->FocusCameraOnPlayer(id);
        });
    }

    auto loadout_changed = false;
    loadout_changed |= loadout_editor_->DrawLoadoutEditor(pri.loadout, pri.team);

    char input_buffer3[64] = "";
    std::string preview = pri.custom_decal.name;
    if (preview.empty())
    {
        preview = "Select a decal";
    }
    if (ImGui::BeginSearchableCombo("CustomDecal", preview.c_str(), input_buffer3, 64, "search for the decal name"))
    {
        if (ImGui::Selectable("None", false))
        {
            pri.custom_decal = CustomTextures::default_decal_;
            loadout_changed = true;
        }
        for (auto& [name, val] : custom_decal_configs_.decal_config)
        {
            auto name2 = name + (val.has_invalid_paths ? " (has invalid paths)" : "");
            if (ImGui::Selectable(name2.c_str(), false))
            {
                pri.custom_decal = FindCustomDecal(name);
                int& body_id = pri.loadout.items[pluginsdk::Equipslot::BODY].product_id;
                int& skin_id = pri.loadout.items[pluginsdk::Equipslot::DECAL].product_id;
                if (body_id == pri.custom_decal.BodyID && skin_id == pri.custom_decal.SkinID)
                {
                    OnGameThread([this, &pri] {
                        ApplyDecal(pri);
                    });
                }
                else
                {
                    body_id = pri.custom_decal.BodyID;
                    skin_id = pri.custom_decal.SkinID;
                    loadout_changed = true;
                }
            }
        }
        ImGui::EndSearchableCombo();
    }

    if (loadout_changed)
    {
        OnGameThread([this, pri] {
            LOG("loadout changed");
            UpdateLoadout(pri);
        });
    }

    ImGui::Text("Rotate car");
    static std::vector const rotate_offsets = {-90.0f, -30.0f, -10.0f, 10.0f, 30.0f, 90.0f};
    for (auto& rotation : rotate_offsets)
    {
        ImGui::SameLine();
        if (ImGui::Button(std::format("{:+}", rotation).c_str()))
        {
            OnGameThread([this, pri, rotation] {
                if (auto pri_ta = GetPriWrapper(pri))
                {
                    CarRotator::RotateCarOfPri(pri_ta, rotation);
                }
            });
        }
    }

    auto cam_setting = camera_settings_->GetCameraOverrideSettings(pri.uid);

    if (camera_settings_->RenderCameraOverride(cam_setting))
    {
        OnGameThread([this, pri_data = pri, cam_setting] {
            if (const auto pri_wrapper = GetPriWrapper(pri_data))
            {
                camera_settings_->SetCameraOverrideSettings(pri_data.uid, cam_setting);
                camera_settings_->SetPriCameraSetting(pri_wrapper, cam_setting.GetCameraSettings());
            }
        });
    }
}

void ReplayManipulatorOpenSource::RenderWindow()
{
    ImGuiTabBarFlags constexpr tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("Players", tab_bar_flags))
    {
        for (auto& player : replay_players_)
        {
            ImGui::ScopeId const scoped_player_id{player.uid.pri_id_string.c_str()};
            ImGui::Disable const disable_if_spectating{player.spectating};
            auto tab_lbl = player.player_name + (player.spectating ? " (Spectating)" : "");

            if (player.team == 1)
            {
                auto swap_red_blue = [](ImGuiCol id) {
                    auto c = ImGui::GetStyleColorVec4(id);
                    std::swap(c.x, c.z);
                    return c;
                };

                ImGui::PushStyleColor(ImGuiCol_Tab, swap_red_blue(ImGuiCol_Tab));
                ImGui::PushStyleColor(ImGuiCol_TabHovered, swap_red_blue(ImGuiCol_TabHovered));
                ImGui::PushStyleColor(ImGuiCol_TabActive, swap_red_blue(ImGuiCol_TabActive));
            }

            if (ImGui::BeginTabItem(tab_lbl.c_str()))
            {
                DrawPriData(player);
                ImGui::EndTabItem();
            }

            if (player.team == 1)
            {
                ImGui::PopStyleColor(3);
            }
        }

        for (const auto& gui_feature_base : gui_features_)
        {
            if (gui_feature_base->ShouldDrawPluginWindow())
            {
                if (ImGui::BeginTabItem(gui_feature_base->GetName().c_str()))
                {
                    ImGui::ScopeId const module_scope{gui_feature_base->GetName()};
                    gui_feature_base->Render();
                    ImGui::EndTabItem();
                }
            }
        }

        ImGui::EndTabBar();
    }
}

PriData* ReplayManipulatorOpenSource::GetPriData(PriWrapper& pri)
{
    const auto it = std::ranges::find_if(replay_players_, [pri](const PriData& p)mutable {
        return p == pri;
    });
    if (it != replay_players_.end())
    {
        return &(*it);
    }

    return nullptr;
}

void ReplayManipulatorOpenSource::OnPriLoadoutSet(PriWrapper& pri)
{
    if (!pri)
    {
        return;
    }

    auto car = pri.GetCar();
    //if no car they're spectating. Don't care about those
    if (!car)
    {
        return;
    }
    auto* pri_data = GetPriData(pri);

    if (pri_data == nullptr)
    {
        return;
    }

    auto loadout_maybe = LoadoutUtilities::GetLoadoutFromPri(pri, pri.GetTeamNum2());
    if (!loadout_maybe)
    {
        return;
    }
    auto& [items, paint_finish] = *loadout_maybe;
    if (items != pri_data->loadout.items)
    {
        LoadoutUtilities::SetLoadoutItems(pri, pri_data->loadout.items);
    }
    if (paint_finish != pri_data->loadout.paint_finish)
    {
        LoadoutUtilities::SetLoadoutPaintFinishColors(car, pri_data->loadout.paint_finish);
    }
}

void ReplayManipulatorOpenSource::RefreshPriData()
{
    auto game_event = gameWrapper->GetGameEventAsReplay();
    if (!game_event)
    {
        return;
    }
    auto pris = game_event.GetPRIs();
    if (pris.IsNull())
    {
        return;
    }

    if (replay_players_originals_.empty())
    {
        for (auto pri : pris)
        {
            if (auto loadout = LoadoutUtilities::GetLoadoutFromPri(pri, pri.GetTeamNum2()))
            {
                replay_players_originals_.emplace_back(pri, *loadout);
            }
        }
    }

    for (auto pri : pris)
    {
        auto loadout = LoadoutUtilities::GetLoadoutFromPri(pri, pri.GetTeamNum2());
        if (!loadout)
            continue;
        if (auto* pri_data = GetPriData(pri))
        {
            pri_data->Update(pri, *loadout);
        }
        else
        {
            replay_players_.emplace_back(pri, *loadout);
        }
    }
    std::ranges::sort(replay_players_, [](PriData& a, PriData& b) {
        return std::tie(a.team, a.player_name) < std::tie(b.team, b.player_name);
    });
}


PriWrapper ReplayManipulatorOpenSource::GetPriWrapper(const PriData& pri_data) const
{
    auto game_event = gameWrapper->GetGameEventAsReplay();
    if (!game_event)
    {
        return {0};
    }
    auto pris = game_event.GetPRIs();
    if (pris.IsNull())
    {
        return {0};
    }
    for (PriWrapper pri : pris)
    {
        if (pri_data == pri)
        {
            return pri;
        }
    }
    return {0};
}

void ReplayManipulatorOpenSource::UpdateLoadout(const PriData& pri_data) const
{
    auto pri_wrapper = GetPriWrapper(pri_data);
    if (!pri_wrapper)
    {
        return;
    }

    LoadoutUtilities::ForceSetLoadout(pri_wrapper, pri_data.loadout);

    if (pri_data.custom_decal.name.empty())
        return;

    auto car = pri_wrapper.GetCar();
    if (!car)
        return;
    CustomTextures::ApplyDecalToCar(pri_data.custom_decal, car);
}

void ReplayManipulatorOpenSource::ApplyDecal(const PriData& pri_data) const
{
    auto pri_wrapper = GetPriWrapper(pri_data);
    if (!pri_wrapper)
    {
        return;
    }

    if (pri_data.custom_decal.name.empty())
        return;

    auto car = pri_wrapper.GetCar();
    if (!car)
    {
        return;
    }
    CustomTextures::ApplyDecalToCar(pri_data.custom_decal, car);
}

void ReplayManipulatorOpenSource::CameraLock() const
{
    static auto left_alt_index = gameWrapper->GetFNameIndexByString("LeftAlt");
    const auto alt_pressed = gameWrapper->IsKeyPressed(left_alt_index);
    auto should_lock = isWindowOpen_ && m_is_window_hovered;
    if (alt_pressed)
    {
        should_lock = !should_lock;
    }
    if (should_lock)
    {
        auto pc = gameWrapper->GetPlayerController();
        // Lock mouse movement
        pc.SetALookUp(0);
        pc.SetATurn(0);

        //if (const auto* real_pc = UCast<APlayerController>(gw_->GetPlayerController()))
        //{
        //	auto* pi = real_pc->PlayerInput;
        //	pi->ResetInput();
        //}
    }
}

void ReplayManipulatorOpenSource::ApplyCarHiddenState(const PriData& pri_data) const
{
    if (!gameWrapper->IsInReplay())
        return;
    auto pri_wrapper = GetPriWrapper(pri_data);
    if (!pri_wrapper)
    {
        return;
    }
    auto car_wrapper = pri_wrapper.GetCar();
    if (!car_wrapper)
    {
        return;
    }
    car_wrapper.SetHidden2(pri_data.hidden);
}

void ReplayManipulatorOpenSource::OnMaterialInit(const CarMeshComponentBaseWrapper& car_mesh_component)
{
    auto car = car_mesh_component.GetCar();
    if (!car)
    {
        return;
    }

    auto pri = car.GetPRI();
    if (!pri)
    {
        return;
    }

    if (auto* pri_data = GetPriData(pri))
    {
        ApplyDecal(*pri_data);
    }
}

void ReplayManipulatorOpenSource::OnSetMeshMaterialColors(CarWrapper& car_wrapper)
{
    if (!car_wrapper)
        return;

    auto pri = car_wrapper.GetPRI();
    if (!pri)
        return;

    auto* pri_data = GetPriData(pri);
    if (!pri_data)
        return;

    auto current_paint = LoadoutUtilities::GetPaintFinishColors(car_wrapper);
    if (current_paint != pri_data->loadout.paint_finish)
    {
        LoadoutUtilities::SetLoadoutPaintFinishColors(car_wrapper, pri_data->loadout.paint_finish);
    }
}

CustomDecal& ReplayManipulatorOpenSource::FindCustomDecal(const std::string& name)
{
    if (const auto it = loaded_custom_decals_.find(name); it == loaded_custom_decals_.end())
    {
        const auto it2 = custom_decal_configs_.decal_config.find(name);
        if (it2 == custom_decal_configs_.decal_config.end())
        {
            return CustomTextures::default_decal_;
        }
        loaded_custom_decals_[name] = CustomDecal(it2->second, texture_cache_);
    }

    return loaded_custom_decals_[name];
}

void ReplayManipulatorOpenSource::ReadJsons()
{
    custom_decal_configs_.decal_config.clear();
    const auto root_dir = gameWrapper->GetDataFolder() / "acplugin/DecalTextures";
    if (!exists(root_dir))
    {
        return;
    }
    const auto jsons = CustomTextures::FindJsons(root_dir);
    for (auto& p : jsons)
    {
        auto config = CustomTextures::ReadCustomDecalJsons(p, root_dir);
        for (auto& [key, val] : config.decal_config)
        {
            custom_decal_configs_.decal_config[key] = val;
        }
    }
}
