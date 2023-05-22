#include "pch.h"
#include "BmAdapter.h"

void BmPostHookMixin::HookEvent(GameWrapper* gw, const std::string& event_name, const BmCallback& callback)
{
    gw->HookEventWithCallerPost<ActorWrapper>(event_name, callback);
}

void BmPostHookMixin::UnHookEvent(GameWrapper* gw, const std::string& event_name)
{
    gw->UnhookEventPost(event_name);
}

BmEvent::EventType BmPostHookMixin::GetEventType()
{
    return BmEvent::EventType::POST;
}

void BmPreHookMixin::HookEvent(GameWrapper* gw, const std::string& event_name, const BmCallback& callback)
{
    gw->HookEventWithCaller<ActorWrapper>(event_name, callback);
}

void BmPreHookMixin::UnHookEvent(GameWrapper* gw, const std::string& event_name)
{
    gw->UnhookEvent(event_name);
}

BmEvent::EventType BmPreHookMixin::GetEventType()
{
    return BmEvent::EventType::PRE;
}
