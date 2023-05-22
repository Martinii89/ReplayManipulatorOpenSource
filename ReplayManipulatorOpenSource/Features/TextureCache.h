#pragma once
#include <memory>
#include <unordered_map>
#include <mutex>
#include <filesystem>


namespace tex_cache_detail
{
struct FsPathHash
{
    std::size_t operator()(const std::filesystem::path& path) const
    {
        return hash_value(path);
    }
};
}


class TextureCache
{
public:
    explicit TextureCache(std::shared_ptr<GameWrapper> game_wrapper);

    std::shared_ptr<ImageWrapper> GetOrCreate(const std::filesystem::path& image_path);
    void RemoveInvalidReferencesFromCache();
    // Hook up the filewatcher to this to automatically refresh textures
    //void OnFileChanged(const std::filesystem::path& file_path);
private:
    std::mutex cache_mutex_;
    std::unordered_map<std::filesystem::path, std::weak_ptr<ImageWrapper>, tex_cache_detail::FsPathHash> texture_cache_;
    std::shared_ptr<GameWrapper> game_wrapper_;
};
