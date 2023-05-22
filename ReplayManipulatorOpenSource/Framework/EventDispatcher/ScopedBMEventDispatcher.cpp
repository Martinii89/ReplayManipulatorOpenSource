#include "pch.h"
#include "ScopedBMEventDispatcher.h"
#include "BakkesModEventDispatcher.h"

BakkesModEventDispatcher::EventDispatcher::Handle ScopedBmEventDispatcher::HookEvent(
    const std::string& event_name, const EventCallback&& callback)
{
    DEBUGLOG("{}", event_name);
    return scoped_pre_dispatcher_.appendListener(event_name, callback);
}


BakkesModEventDispatcher::EventDispatcherPost::Handle ScopedBmEventDispatcher::HookEventPost(
    const std::string& event_name, const EventCallback&& callback)
{
    DEBUGLOG("{}", event_name);
    return scoped_post_dispatcher_.appendListener(event_name, callback);
}


bool ScopedBmEventDispatcher::UnHookEvent(const std::string& event_name,
                                          const EventDispatcher::Handle& handle)
{
    DEBUGLOG("{}", event_name);
    return scoped_pre_dispatcher_.removeListener(event_name, handle);
}

bool ScopedBmEventDispatcher::UnHookEventPost(const std::string& event_name,
                                              const EventDispatcherPost::Handle& handle)
{
    DEBUGLOG("{}", event_name);
    return scoped_post_dispatcher_.removeListener(event_name, handle);
}

void ScopedBmEventDispatcher::RunOnGameThread(std::function<void()>&& func) const
{
    main_dispatcher_->RunOnGameThread(std::move(func));
}

void ScopedBmEventDispatcher::RunOnGameThread(std::function<void(GameWrapper* gw)>&& func) const
{
    main_dispatcher_->RunOnGameThread(std::move(func));
}

void ScopedBmEventDispatcher::Reset()
{
    scoped_post_dispatcher_.reset();
    scoped_pre_dispatcher_.reset();
}

ScopedBmEventDispatcher::ScopedBmEventDispatcher(EventDispatcher& pre_hooks,
                                                 EventDispatcherPost& post_hooks,
                                                 BakkesModEventDispatcher* main_dispatcher)
    : scoped_pre_dispatcher_(pre_hooks),
      scoped_post_dispatcher_(post_hooks),
      main_dispatcher_(main_dispatcher) {}
