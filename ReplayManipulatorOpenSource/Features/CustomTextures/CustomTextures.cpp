#include "pch.h"
#include "CustomTextures.h"

#include <fstream>
#include "Features/TextureCache.h"
#include "bakkesmod/utilities/DecalUtilities.h"

#define JOPTIONAL(var) if (j.find(#var) != j.end()){j.at(#var).get_to(p.var);}
#define JOPTIONAL2(var, var2) if (j.find(#var) != j.end()){j.at(#var).get_to(p.var2);}
#define J(var) j.at(#var).get_to(p.var);
#define J2(var, var2) j.at(#var).get_to(p.var2);


std::vector<std::filesystem::path> CustomTextures::FindJsons(const std::filesystem::path& root)
{
    std::vector<std::filesystem::path> jsons;
    for (const auto& p : std::filesystem::recursive_directory_iterator(root))
    {
        try
        {
            if (!p.is_regular_file())
                continue;

            const auto& path = p.path();
            if (path.extension() == ".json")
            {
                jsons.push_back(path);
            }
        }
        catch (const std::exception& e)
        {
            LOG("Exception: {}", e.what());
        }
    }

    return jsons;
}

// https://stackoverflow.com/questions/7007802/erase-specific-elements-in-stdmap
template <typename Map, typename F>
void MapErasePred(Map& m, F pred)
{
    auto i = m.begin();
    while ((i = std::find_if(i, m.end(), pred)) != m.end())
    {
        m.erase(i++);
    }
}

TextureJson CustomTextures::ReadCustomDecalJsons(const std::filesystem::path& json_file,
                                                 const std::filesystem::path& root)
{
    try
    {
        std::ifstream json_stream(json_file);
        json j;
        json_stream >> j;
        auto texture_json = j.get<TextureJson>();
        for (auto& config : texture_json.decal_config | std::views::values)
        {
            // Remove empty entries
            MapErasePred(config.Body, [](auto it) {
                return it.second.empty();
            });
            MapErasePred(config.Chassis, [](auto it) {
                return it.second.empty();
            });

            //Prepend the right paths
            FixImagePath(config.Body, json_file, root);
            FixImagePath(config.Chassis, json_file, root);

            // Check if any paths are invalid
            if (std::ranges::any_of(config.Body, [](auto it) {
                    return !ValidFile(it.second);
                }) ||
                std::ranges::any_of(config.Chassis, [](auto it) {
                    return !ValidFile(it.second);
                }))
            {
                config.has_invalid_paths = true;
            }
        }
        return texture_json;
    }
    catch (const std::exception& e)
    {
        LOG("Exception: {}", e.what());
    }
    return {};
}

BallJson CustomTextures::ReadCustomBallJsons(const std::filesystem::path& json_file, const std::filesystem::path& root)
{
    try
    {
        std::ifstream json_stream(json_file);
        json j;
        json_stream >> j;
        auto ball_json = j.get<BallJson>();
        for (auto& config : ball_json.ball_config | std::views::values)
        {
            // Remove empty entries
            MapErasePred(config.ball_textures, [](auto it) {
                return it.second.empty();
            });

            //Prepend the right paths
            FixImagePath(config.ball_textures, json_file, root);

            // Check if any paths are invalid
            if (std::ranges::any_of(config.ball_textures,
                                    [](auto& it) {
                                        return !ValidFile(it.second);
                                    }))
            {
                config.has_invalid_paths = true;
            }
        }
        return ball_json;
    }
    catch (const std::exception& e)
    {
        LOG("Exception: {}", e.what());
    }
    return {};
}

void CustomTextures::FixImagePath(DecalPathMap& file_map, const std::filesystem::path& json_dir,
                                  const std::filesystem::path& root_dir)
{
    const auto json_file_dir = json_dir.parent_path();
    for (auto& file : file_map | std::views::values)
    {
        if (ValidFile(json_file_dir / file))
        {
            file = (json_file_dir / file);
        }
        else if (ValidFile(root_dir / file))
        {
            file = (root_dir / file);
        }
        else
        {
            LOG("ERROR: File not found: {} While parsing {}", file, json_dir.string());
            file = "";
        }
    }
}

bool CustomTextures::CarHasRightBodyAndSkin(const DecalBase& decal, const CarWrapper& car)
{
    auto car_asset_ids = DecalUtilities::GetBodyAssetIds(car);
    if (!car_asset_ids)
        return false;

    auto& [body, skin] = *car_asset_ids;
    return decal.BodyID == body && decal.SkinID == skin;
}


void CustomTextures::ApplyDecalToCar(const CustomDecal& decal, const CarWrapper& car)
{
    if (!CarHasRightBodyAndSkin(decal, car))
    {
        DEBUGLOG("validation failed");
        return;
    }

    const auto sdk_decal = pluginsdk::BodyShaderOverride{.body_mic_override = {.textures = decal.body,},
                                                         .chassis_mic_override = {.textures = decal.chassis},
                                                         .body_id = decal.BodyID, .skin_id = decal.SkinID};

    if (auto result = DecalUtilities::ApplyDecalToCar(car, sdk_decal); !result)
    {
        LOG("ApplyDecalToCar: {}", result.error());
    }
}

void CustomTextures::ApplyTextureToBall(const CustomBallDecal& decal, BallWrapper& ball)
{
    if (!ball)
    {
        LOG("no ball");
        return;
    }

    const auto sdk_decal = pluginsdk::ShaderOverride{.textures = decal.ball_textures, .colors = {}, .scalar = {}};

    auto apply_result = DecalUtilities::ApplyDecalToBall(ball, sdk_decal);
    if (!apply_result)
    {
        LOG("{}", apply_result.error());
    }

}

bool CustomTextures::ValidFile(const std::filesystem::path& path)
{
    return exists(path) && is_regular_file(path);
}


void from_json(const json& j, DecalConfig& p)
{
    JOPTIONAL(BodyID)
    JOPTIONAL(SkinID)
    JOPTIONAL(Chassis)
    JOPTIONAL(Body)
}

void from_json(const json& j, TextureJson& p)
{
    p.decal_config = j.get<std::map<std::string, DecalConfig>>();
    for (auto& [key, val] : p.decal_config)
    {
        val.name = key;
    }
}

void from_json(const json& j, BallJson& p)
{
    p.ball_config = j.get<std::map<std::string, CustomBallConfig>>();
    for (auto& [key, val] : p.ball_config)
    {
        val.name = key;
    }
}

void from_json(const json& j, CustomBallConfig& p)
{
    for (auto& [key, val] : j.items())
    {
        p.ball_textures[key] = val.get<std::filesystem::path>();
    }
}

CustomDecal::CustomDecal(const DecalConfig& config, const std::shared_ptr<TextureCache>& tex_cache)
    : DecalBase{config.name, config.BodyID, config.SkinID}
{
    DEBUGLOG("Creating custom decal: {}", config.name);
    DEBUGLOG("Body image count : {}", config.Body.size());
    auto valid_file = [](const std::filesystem::path& path) {
        return exists(path) && is_regular_file(path);
    };
    for (const auto& [name, filePath] : config.Body)
    {
        if (valid_file(filePath))
        {
            body[name] = tex_cache->GetOrCreate(filePath);
        }
        else
        {
            LOG("Invalid file: {}", filePath);
        }
    }

    for (const auto& [name, filePath] : config.Chassis)
    {
        if (valid_file(filePath))
        {
            chassis[name] = tex_cache->GetOrCreate(filePath);
        }
        else
        {
            LOG("Invalid file: {}", filePath);
        }
    }
}

CustomBallDecal::CustomBallDecal(const CustomBallConfig& config, const std::shared_ptr<TextureCache>& tex_cache)
    : CustomBallBase{config.name}
{
    auto valid_file = [](const std::filesystem::path& path) {
        return exists(path) && is_regular_file(path);
    };
    for (const auto& [name, filePath] : config.ball_textures)
    {
        if (valid_file(filePath))
        {
            ball_textures[name] = tex_cache->GetOrCreate(filePath);
        }
    }
}
