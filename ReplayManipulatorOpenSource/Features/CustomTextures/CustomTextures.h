#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <bakkesmod/core/custom_decals_structs.h>


class TextureCache;

using DecalPathMap = std::map<std::string, std::filesystem::path>;
using DecalTexMap = pluginsdk::TextureOverride;

// ReSharper disable CppInconsistentNaming
struct DecalBase
{
    std::string name;
    int BodyID = -1;
    int SkinID = -1;
};

struct DecalConfig : DecalBase
{
    DecalPathMap Chassis;
    DecalPathMap Body;
    bool has_invalid_paths = false;
};

// ReSharper restore CppInconsistentNaming

struct CustomDecal : DecalBase
{
    DecalTexMap chassis;
    DecalTexMap body;

    explicit CustomDecal(const DecalConfig& config, const std::shared_ptr<TextureCache>& tex_cache);
    CustomDecal() = default;
};

struct CustomBallBase
{
    std::string name;
};

struct CustomBallConfig : CustomBallBase
{
    DecalPathMap ball_textures;
    bool has_invalid_paths = false;
};

struct CustomBallDecal : CustomBallBase
{
    DecalTexMap ball_textures;
    explicit CustomBallDecal(const CustomBallConfig& config, const std::shared_ptr<TextureCache>& tex_cache);
    CustomBallDecal() = default;
};


struct TextureJson
{
    std::map<std::string, DecalConfig> decal_config;
};

struct BallJson
{
    std::map<std::string, CustomBallConfig> ball_config;
};

void from_json(const json& j, DecalConfig& p);
void from_json(const json& j, TextureJson& p);
void from_json(const json& j, BallJson& p);
void from_json(const json& j, CustomBallConfig& p);

class CustomTextures
{
public:
    static std::vector<std::filesystem::path> FindJsons(const std::filesystem::path& root);
    static TextureJson ReadCustomDecalJsons(const std::filesystem::path& json_file, const std::filesystem::path& root);
    static BallJson ReadCustomBallJsons(const std::filesystem::path& json_file, const std::filesystem::path& root);
    static void FixImagePath(DecalPathMap& file_map, const std::filesystem::path& json_dir,
                             const std::filesystem::path&
                             root_dir);

    static bool CarHasRightBodyAndSkin(const DecalBase& decal, const CarWrapper& car);
    static void ApplyDecalToCar(const CustomDecal& decal, const CarWrapper& car);
    static void ApplyTextureToBall(const CustomBallDecal& decal, BallWrapper& ball);
    static bool ValidFile(const std::filesystem::path&);

    static inline CustomDecal default_decal_{};
    static inline CustomBallDecal default_ball_{};


};
