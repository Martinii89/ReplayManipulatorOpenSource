#pragma once
#include "eventpp/eventdispatcher.h"


struct BmEvent
{
    enum class EventType
    {
        PRE,
        POST,
        //MANUAL
    };

    std::string name;
    ActorWrapper caller;
    void* params;
    EventType type;

    template<typename T>
    T CallerCast()
    {
        return {caller.memory_address};
    }

};

struct BmEventPolicy
{
    
    // ReSharper disable once CppInconsistentNaming
    static const std::string& getEvent(const BmEvent& e) // special named for eventpp
    {
        return e.name;
    }
};

using BmCallback = std::function<void(ActorWrapper caller, void* params, std::string event_name)>;

template <typename T>
concept BmHookMixin = requires(T, GameWrapper* gw, const std::string& event_name, const BmCallback& callback) {
    T::HookEvent(gw, event_name, callback);
    T::UnHookEvent(gw, event_name);
    { T::GetEventType() } -> std::convertible_to<BmEvent::EventType>;
};

struct BmPostHookMixin
{
    static void HookEvent(GameWrapper* gw, const std::string& event_name, const BmCallback& callback);
    static void UnHookEvent(GameWrapper* gw, const std::string& event_name);
    static BmEvent::EventType GetEventType();
};

struct BmPreHookMixin
{
    static void HookEvent(GameWrapper* gw, const std::string& event_name, const BmCallback& callback);
    static void UnHookEvent(GameWrapper* gw, const std::string& event_name);
    static BmEvent::EventType GetEventType();
};


template <BmHookMixin BmHookMixin>
class BmAdaptedEventDispatcher : public eventpp::EventDispatcher<std::string, void(BmEvent&), BmEventPolicy>
{
public:
    using Super = EventDispatcher;


    explicit BmAdaptedEventDispatcher(BakkesMod::Plugin::BakkesModPlugin* plugin)
        : plugin_(plugin) {}


    void HookIfNoListeners(const Event& event)
    {
        if (!hasAnyListener(event))
        {
            //plugin_->cvarManager->log("Hooking cause no previous listeners");
            HookBm(event);
        }
    }

    //DO NOT RENAME! We use these to intercept adding and removing events handlers to manage the event registration with BakkesMod
    // ReSharper disable CppInconsistentNaming
    Handle appendListener(const Event& event, const Callback& callback)
    {
        LOG("appendListener: {}", event);
        HookIfNoListeners(event);
        return Super::appendListener(event, callback);
    }

    //DO NOT RENAME!
    Handle prependListener(const Event& event, const Callback& callback)
    {
        LOG("prependListener: {}", event);
        HookIfNoListeners(event);
        return Super::prependListener(event, callback);
    }

    //DO NOT RENAME!
    Handle insertListener(const Event& event, const Callback& callback, const Handle& before)
    {
        LOG("insertListener: {}", event);
        HookIfNoListeners(event);
        return Super::insertListener(event, callback, before);
    }

    //DO NOT RENAME!
    bool removeListener(const Event& event, const Handle& handle)
    {
        LOG("removeListener: {}", event);
        const auto res = Super::removeListener(event, handle);
        if (!hasAnyListener(event))
        {
            //plugin_->cvarManager->log("Unhooking cause no more listeners: " + event);
            UnhookBm(event);
        }

        return res;
    }
    // ReSharper restore CppInconsistentNaming

private:
    BakkesMod::Plugin::BakkesModPlugin* plugin_;

    void HookBm(const Event& event)
    {
        LOG("HookBm: {}", event);
        auto hook_lambda = [this](const ActorWrapper& caller, void* params, const std::string& event_name) {
            BmEvent bm_event{event_name, caller, params, BmHookMixin::GetEventType()};
            dispatch(bm_event);
        };
        BmHookMixin::HookEvent(plugin_->gameWrapper.get(), event, hook_lambda);
    }

    void UnhookBm(const Event& event)
    {
        LOG("UnhookBm: {}", event);
        BmHookMixin::UnHookEvent(plugin_->gameWrapper.get(), event);
    }
};
