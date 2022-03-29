#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include <map>
#include <typeindex>
#include <list>
#include "./Event.h"
#include <functional>

class IEventCallBack {
    private:
        virtual void Call(Event& e) = 0;

    public:
        virtual ~IEventCallBack() = default;

        void Execute(Event& e) {
            Call(e);
        }
};

template <typename TOwner, typename TEvent>
class EventCallBack: public IEventCallBack{
    private:
        typedef void (TOwner::*CallbackFunction)(TEvent&);

        TOwner* m_ownerInstance;
        CallbackFunction m_callbackFunction;

        // Here we have the callback function pointer that needs to be invoked
        virtual void Call(Event& e) override {
            std::invoke(m_callbackFunction, m_ownerInstance, static_cast<TEvent&>(e));
        }

    public:
        EventCallBack(TOwner* ownerInstance, CallbackFunction callbackFunction) {
            this->m_ownerInstance = ownerInstance;
            this->m_callbackFunction = callbackFunction;
        }

        virtual ~EventCallBack() override = default;

};

typedef std::list<std::unique_ptr<IEventCallBack>> HandlerList;

class EventBus {
    private:
        // Key is the EventType and the value is a pointer to a list of eventHandlers
        std::map<std::type_index, std::unique_ptr<HandlerList>> m_subscribers;

    public:
        EventBus() {

        }

        ~EventBus() {

        }

        //Clears subscriber list
        void ClearSubscribers() {
            m_subscribers.clear();
        }

        //////////////////////////////////////////////////////
        // Subscribe to an event <t>
        // In our implementation a listener subscribes to an event
        // eventbus->SubscribeToEvenet<CollisionEvent>(this, &Game::onCollision)
        //////////////////////////////////////////////////////
        template <typename TEvent, typename TOwner>
        void SubscribeToEvent(TOwner* ownerInstance, void (TOwner::*callbackFunction)(TEvent&)) {

            if (m_subscribers[typeid(TEvent)].get() == nullptr) {
                m_subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
            }
            auto subscriber = std::make_unique<EventCallBack<TOwner, TEvent>>(ownerInstance, callbackFunction);
            m_subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
        }

        //////////////////////////////////////////////////////
        // Emit an event of type <T>
        // event we go ahead and execute all the listener callbacks
        //////////////////////////////////////////////////////
        template <typename TEvent, typename ...TArgs>
        void EmitEvent(TArgs&& ...args) {
            auto handlers = m_subscribers[typeid(TEvent)].get();
            if (handlers) {
                for (auto it = handlers->begin(); it != handlers->end(); it++) {
                    auto handler = it->get();
                    TEvent event(std::forward<TArgs>(args)...);
                    handler->Execute(event);
                };
            }
        }
};

#endif