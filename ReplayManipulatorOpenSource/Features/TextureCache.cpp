#include "pch.h"
#include "TextureCache.h"


TextureCache::TextureCache(std::shared_ptr<GameWrapper> game_wrapper)
    : game_wrapper_{std::move(game_wrapper)} {}

std::shared_ptr<ImageWrapper> TextureCache::GetOrCreate(const std::filesystem::path& image_path)
{
    std::lock_guard const lock(cache_mutex_);
    if (auto image_ptr = texture_cache_[image_path].lock())
    {
        DEBUGLOG("returning cached image for {}", image_path.string());
        return image_ptr;
    }
    DEBUGLOG("Creating image for {}", image_path.string());
    auto ret = std::make_shared<ImageWrapper>(image_path);
    texture_cache_[image_path] = ret;
    return ret;
}

void TextureCache::RemoveInvalidReferencesFromCache()
{
    std::lock_guard const lock(cache_mutex_);
    using KeyValuePair = decltype(texture_cache_)::value_type;
    const auto removed_count = std::erase_if(texture_cache_, [](const KeyValuePair& item) {
        auto const& [key, value] = item;
        return value.expired();
    });
    if (removed_count > 0)
    {
        LOG("Removed {} unused textures", removed_count);
    }
}
