#pragma once
//#include "BmAdapter.h"

#include "BmAdapter.h"

#include <eventpp/utilities/scopedremover.h>

class BakkesModEventDispatcher;

/// <summary>
/// RAII style object. The callbacks registered to this object are deleted upon destruction
/// </summary>
class ScopedBmEventDispatcher
{
public:
    using EventDispatcher = BmAdaptedEventDispatcher<BmPreHookMixin>;
    using EventDispatcherPost = BmAdaptedEventDispatcher<BmPostHookMixin>;
    using EventCallback = EventDispatcher::Callback;
    using EventCallbackPost = EventDispatcher::Callback;

    ScopedBmEventDispatcher(EventDispatcher& pre_hooks, EventDispatcherPost& post_hooks,
                            BakkesModEventDispatcher* main_dispatcher);

    EventDispatcher::Handle HookEvent(const std::string& event_name, const EventCallback&& callback);
    bool UnHookEvent(const std::string& event_name, const EventDispatcher::Handle& handle);

    EventDispatcherPost::Handle HookEventPost(const std::string& event_name, const EventCallback&& callback);
    bool UnHookEventPost(const std::string& event_name, const EventDispatcherPost::Handle& handle);

    // Delays execution until the next game tick if currently executing on a different thread than the game thread.
    void RunOnGameThread(std::function<void()>&& func) const;
    void RunOnGameThread(std::function<void(GameWrapper* gw)>&& func) const;

    void Reset();

private:
    eventpp::ScopedRemover<EventDispatcher> scoped_pre_dispatcher_;
    eventpp::ScopedRemover<EventDispatcherPost> scoped_post_dispatcher_;
    BakkesModEventDispatcher* main_dispatcher_;
};
