#include "pch.h"

#include "BakkesModEventDispatcher.h"
#include "ScopedBMEventDispatcher.h"

BakkesModEventDispatcher::BakkesModEventDispatcher(BakkesMod::Plugin::BakkesModPlugin* plugin)
    : pre_dispatcher_(plugin),
      post_dispatcher_(plugin),
      plugin_(plugin)
{
    if (plugin->gameWrapper)
    {
        plugin->gameWrapper->Execute([this](...) {
            game_thread_id_ = std::this_thread::get_id();
        });
    }
}

ScopedBmEventDispatcher BakkesModEventDispatcher::GetScopedDispatcher()
{
    return {this->pre_dispatcher_, this->post_dispatcher_, this};
}

BakkesModEventDispatcher::DrawableList::Handle BakkesModEventDispatcher::AddDrawable(
    const DrawableList::Callback&& callback)
{
    if (drawables_list_.empty())
    {
        plugin_->gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
            drawables_list_(canvas);
        });
    }

    return drawables_list_.append(callback);
}

bool BakkesModEventDispatcher::RemoveDrawable(const DrawableList::Handle& handle)
{
    const auto removed = drawables_list_.remove(handle);
    if (removed && drawables_list_.empty())
    {
        plugin_->gameWrapper->UnregisterDrawables();
    }
    return removed;
}

BakkesModEventDispatcher::ScopedDrawableList BakkesModEventDispatcher::GetScopedDrawableList()
{
    return ScopedDrawableList(drawables_list_);
}

void BakkesModEventDispatcher::RunOnGameThread(const std::function<void()>&& func)
{
    if (game_thread_id_ != std::thread::id() && game_thread_id_ == std::this_thread::get_id())
    {
        func();
        return;
    }

    plugin_->gameWrapper->Execute([this,func](...) {
        game_thread_id_ = std::this_thread::get_id();
        func();
    });
}

void BakkesModEventDispatcher::RunOnGameThread(const std::function<void(GameWrapper* gw)>&& func)
{
    if (game_thread_id_ != std::thread::id() && game_thread_id_ == std::this_thread::get_id())
    {
        func(plugin_->gameWrapper.get());
        return;
    }

    plugin_->gameWrapper->Execute([this,func](GameWrapper* gw) {
        game_thread_id_ = std::this_thread::get_id();
        func(gw);
    });
}


BakkesModEventDispatcher::EventDispatcher::Handle BakkesModEventDispatcher::HookEvent(
    const std::string& event_name, const EventCallback&& callback)
{
    return pre_dispatcher_.appendListener(event_name, callback);
}

BakkesModEventDispatcher::EventDispatcherPost::Handle BakkesModEventDispatcher::HookEventPost(
    const std::string& event_name, const EventCallbackPost&& callback)
{
    return post_dispatcher_.appendListener(event_name, callback);
}

bool BakkesModEventDispatcher::UnHookEvent(const std::string& event_name, const EventDispatcher::Handle& handle)
{
    return pre_dispatcher_.removeListener(event_name, handle);
}

bool BakkesModEventDispatcher::UnHookEventPost(const std::string& event_name, const EventDispatcherPost::Handle& handle)
{
    return post_dispatcher_.removeListener(event_name, handle);
}
