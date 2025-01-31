#include "events.h"
#include "exceptions.h"
#include "log.h"


template <> uint64_t EventInfo::get() { return u; }

template <> uint32_t EventInfo::get() { return static_cast<uint32_t>(u); }

template <> uint16_t EventInfo::get() { return static_cast<uint16_t>(u); }

template <> uint8_t EventInfo::get() { return static_cast<uint8_t>(u); }

template <> int64_t EventInfo::get() { return i; }

template <> int32_t EventInfo::get() { return static_cast<int32_t>(i); }

template <> int16_t EventInfo::get() { return static_cast<int16_t>(i); }

template <> int8_t EventInfo::get() { return static_cast<int8_t>(i); }

template <> void EventInfo::set(uint64_t t) { u = t; }

template <> void EventInfo::set(uint32_t t) { u = t; }
   
template <> void EventInfo::set(uint16_t t) { u = t; }
    
template <> void EventInfo::set(uint8_t t) { u = t; }

template <> void EventInfo::set(int64_t t) { i = t; }

template <> void EventInfo::set(int32_t t) { i = t; }
    
template <> void EventInfo::set(int16_t t) { i = t; }
    
template <> void EventInfo::set(int8_t t) { i = t; }

EventScope::EventScope(Events *events) : events{events} {}

EventScope::EventScope(EventScope &&other) noexcept
    : events{other.events}, cb_indicies{std::move(other.cb_indicies)},
      aux_indicies{std::move(other.aux_indicies)} {
    other.events = nullptr;
}

EventScope& EventScope::operator=(EventScope&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    std::swap(events, other.events);
    std::swap(aux_indicies, other.aux_indicies);
    std::swap(cb_indicies, other.cb_indicies);
    return *this;
}

void EventScope::add_callback(event_t event, std::size_t ix) {
    cb_indicies.push_front(ix);
    cb_indicies.push_front(event);
}

void EventScope::add_aux(std::size_t ix) { aux_indicies.push_front(ix); }

void EventScope::add_event(event_t event) {
    evt_indicies.push_front(event);
}

void EventScope::finalize() {
    finalized = true;
}

EventScope::~EventScope() {
    if (events == nullptr) {
        return;
    }
    auto stamp = SDL_GetTicks64();
    auto it = cb_indicies.begin();
    while (it != cb_indicies.end()) {
        event_t evt = *it;
        ++it;
        std::size_t ix = *it;
        ++it;
        events->remove_callback(evt, ix);
    }

    for (std::size_t aux : aux_indicies) {
        events->remove_aux(aux);
    }

    for (event_t evt: evt_indicies) {
        events->remove_event(evt);
    }
    if (!finalized) {
        events->end_scope(this);
    }
}

Events::Events() noexcept : events{{EventType::EMPTY}}, scopes{} {
    root_scope = begin_scope();
    root_scope->finalize();
}

std::unique_ptr<EventScope> Events::begin_scope() {
    auto ptr = std::make_unique<EventScope>(this);
    scopes.emplace_back(ptr.get());
    return ptr;
}

void Events::end_scope(EventScope *ptr) {
    for (int64_t i = scopes.size() - 1; i >= 0; --i) {
        if (scopes[i] == ptr) {
            LOG_WARNING("End scope called");

            scopes.erase(scopes.begin() + i);
            return;
        }
    }
    LOG_WARNING("Invalid scope end");
}

void Events::finalize_scope() {
    if (scopes.size() > 1) {
        scopes.back()->finalize();
        scopes.pop_back();
    } else {
        throw base_exception("No matching begin_scope");
    }
}

event_t Events::register_event(EventType type, int vector_size) {
    event_t id = free_ix;
    LOG_DEBUG("Event %u registered", id);
    ++free_ix;
    for (; free_ix < events.size(); ++free_ix) {
        if (events[free_ix].type == EventType::EMPTY) {
            break;
        }
    }
    if (id > last_ix) {
        last_ix = id;
    }
    if (id == events.size()) {
        events.resize(id + 1);
    }
    events[id] = {type};
    if (type == EventType::UNIFIED_VEC) {
        if (vector_size <= 1) {
            events[id].type = EventType::UNIFIED;
        } else {
            events[id].buffer.resize(vector_size);
            for (auto &e : events[id].buffer) {
                e.u = 0;
            }
        }
    }
    scopes.back()->add_event(id);
    return id;
}

void Events::register_callback(event_t id, void (*callback)(EventInfo, void *),
                               void *aux) {
    scopes.back()->add_callback(id, events[id].free_ix);
    LOG_DEBUG("Event %d callback added at %d", id, events[id].free_ix);
    if (events[id].free_ix == events[id].callbacks.size()) {
        events[id].callbacks.push_back({callback, aux});
        events[id].last_ix = events[id].free_ix;
        ++events[id].free_ix;
    } else {
        events[id].callbacks[events[id].free_ix] = {callback, aux};
        if (events[id].free_ix > events[id].last_ix) {
            events[id].last_ix = events[id].free_ix;
        }
        std::size_t ix = events[id].free_ix + 1;
        for (; ix < events[id].callbacks.size(); ++ix) {
            if (events[id].callbacks[ix].callback == nullptr) {
                break;
            }
        }
        events[id].free_ix = ix;
    }
}

void Events::notify_event(event_t id, EventInfo data) {
    switch (events[id].type) {
    case EventType::IMMEDIATE:
        for (auto &cb : events[id].callbacks) {
            cb.callback(data, cb.aux);
        }
        return;
    case EventType::DELAYED:
        events[id].buffer.push_back(data);
        return;
    case EventType::UNIFIED:
        events[id].data = data;
        events[id].triggered = true;
        return;
    case EventType::UNIFIED_VEC:
        events[id].buffer[data.u].u = 1;
        events[id].triggered = true;
        return;
    case EventType::EMPTY:
        LOG_WARNING("Empty event %d called", id);
        return;
    }
}

void Events::add_aux(WrapperBase *aux) {
    scopes.back()->add_aux(free_aux);
    if (free_aux == aux_data.size()) {
        aux_data.emplace_back(aux);
        ++free_aux;
    } else {
        aux_data[free_aux].reset(aux);
        ++free_aux;
        for (; free_aux < aux_data.size(); ++free_aux) {
            if (aux_data[free_aux] == nullptr) {
                break;
            }
        }
    }
}

void null_callback(EventInfo, void*) {}

void Events::remove_event(event_t event) {
    LOG_DEBUG("Removing event %d", event);
    if (event <= 0 || event >= events.size()) {
        LOG_WARNING("Out of bounds event removed");
        return;
    }
    events[event].type = EventType::EMPTY;
    events[event].buffer.clear();
    events[event].callbacks.clear();
    if (event < free_ix) {
        free_ix = event;
    }
    if (event == last_ix) {
        int64_t i = event - 1;
        for (; i >= 1; --i) {
            if (events[i].type != EventType::EMPTY) {
                break;
            }
        }
        last_ix = i;
        events.resize(i + 1);
    }
}

void Events::remove_callback(event_t event, std::size_t ix) {
    LOG_DEBUG("Removing event %d callback %d", event, ix);
    if (event <= 0 || event >= events.size()) {
        LOG_WARNING("Out of bounds event callback removed");
        return;
    }
    if (events[event].callbacks.size() <= ix) {
        LOG_WARNING("Out of bounds event callback removed");
        return;
    }
    if (events[event].type == EventType::EMPTY) {
        LOG_WARNING("Removing callback from empty event");
        return;
    }
    events[event].callbacks[ix].callback = null_callback;
    events[event].callbacks[ix].aux = nullptr;
    if (ix < events[event].free_ix) {
        events[event].free_ix = ix;
    }
    if (ix == events[event].last_ix) {
        int64_t i = ix - 1;
        for (; i >= 0; --i) {
            if (events[event].callbacks[i].callback != null_callback) {
                break;
            }
        }
        events[event].last_ix = i;
        events[event].callbacks.resize(i + 1);
    }
}

void Events::remove_aux(std::size_t ix) {
    LOG_DEBUG("Removing aux %d", ix);
    aux_data[ix].reset();
    if (ix < free_aux) {
        free_aux = ix;
    }
}

void Events::handle_events() {
    for (auto event = events.begin() + 1; event != events.end(); ++event) {
        if (event->type == EventType::UNIFIED && event->triggered) {
            for (auto &cb : event->callbacks) {
                cb.callback(event->data, cb.aux);
            }
            event->triggered = false;
        } else if (event->type == EventType::DELAYED) {
            for (EventInfo data : event->buffer) {
                for (auto &cb : event->callbacks) {
                    cb.callback(data, cb.aux);
                }
            }
            event->buffer.clear();
        } else if (event->type == EventType::UNIFIED_VEC && event->triggered) {
            for (uint64_t i = 0; i < event->buffer.size(); ++i) {
                if (event->buffer[i].u != 0) {
                    event->buffer[i].u = 0;
                    EventInfo data {};
                    data.u = i;
                    for (auto &cb : event->callbacks) {
                        cb.callback(data, cb.aux);
                    }
                }
            }
            event->triggered = false;
        }
    }
}
