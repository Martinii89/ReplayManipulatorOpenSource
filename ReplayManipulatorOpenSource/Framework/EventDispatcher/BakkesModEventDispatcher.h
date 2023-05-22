#pragma once
#include "BmAdapter.h"
#include "ScopedBMEventDispatcher.h"
#include "eventpp/eventdispatcher.h"
#include "eventpp/utilities/scopedremover.h"

class BakkesModEventDispatcher
{
public:
    using EventDispatcher = BmAdaptedEventDispatcher<BmPreHookMixin>;
    using EventDispatcherPost = BmAdaptedEventDispatcher<BmPostHookMixin>;
    using EventCallback = EventDispatcher::Callback;
    using EventCallbackPost = EventDispatcher::Callback;

    using DrawableSignature = void(CanvasWrapper&);
    using DrawableList = eventpp::CallbackList<DrawableSignature>;
    using ScopedDrawableList = eventpp::ScopedRemover<DrawableList>;

    explicit BakkesModEventDispatcher(BakkesMod::Plugin::BakkesModPlugin* plugin);

    [[nodiscard]] ScopedBmEventDispatcher GetScopedDispatcher();
    EventDispatcher::Handle HookEvent(const std::string& event_name, const EventCallback&& callback);
    bool UnHookEvent(const std::string& event_name, const EventDispatcher::Handle& handle);

    EventDispatcherPost::Handle HookEventPost(const std::string& event_name, const EventCallbackPost&& callback);
    bool UnHookEventPost(const std::string& event_name, const EventDispatcherPost::Handle& handle);

    DrawableList::Handle AddDrawable(const DrawableList::Callback&& callback);
    bool RemoveDrawable(const DrawableList::Handle& handle);
    ScopedDrawableList GetScopedDrawableList();

    // Delays execution until the next game tick if currently executing on a different thread than the game thread.
    void RunOnGameThread(const std::function<void()>&& func);
    void RunOnGameThread(const std::function<void(GameWrapper* gw)>&& func);

private:
    EventDispatcher pre_dispatcher_;
    EventDispatcherPost post_dispatcher_;
    DrawableList drawables_list_;
    BakkesMod::Plugin::BakkesModPlugin* plugin_;
    std::thread::id game_thread_id_;
};
