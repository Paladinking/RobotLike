#ifndef ENGINE_ENVENT_H
#define ENGINE_ENVENT_H
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>
#include <forward_list>

enum class EventType {
    EMPTY,
    IMMEDIATE,  // Event triggers callaback instantly
    DELAYED,    // Event triggers callback after tick
    UNIFIED,    // Event triggers one callback for all changes this tick
    UNIFIED_VEC // Event triggers one callback for each unique data
};

typedef uint32_t event_t;

constexpr event_t NULL_EVENT = 0;

union EventInfo {
    int64_t i;
    uint64_t u;
    void *ptr;

    template <class T> T get() { return get_helper<T>(); }

    template <class T> void set(T t) { set_helper<T>(t); }

private:
    template <class T, T = nullptr> T get_helper() {
        return reinterpret_cast<T>(ptr);
    }

    template <class T, T = nullptr> void set_helper(T t) {
        ptr = t;
    }
};

template <> uint64_t EventInfo::get();

template <> uint32_t EventInfo::get();

template <> uint16_t EventInfo::get();

template <> uint8_t EventInfo::get();

template <> int64_t EventInfo::get();

template <> int32_t EventInfo::get();

template <> int16_t EventInfo::get();

template <> int8_t EventInfo::get();
    
template <> void EventInfo::set(uint64_t t);

template <> void EventInfo::set(uint32_t t);
    
template <> void EventInfo::set(uint16_t t);
    
template <> void EventInfo::set(uint8_t t);

template <> void EventInfo::set(int64_t t);

template <> void EventInfo::set(int32_t t);
    
template <> void EventInfo::set(int16_t t);
    
template <> void EventInfo::set(int8_t t);

class Events;

class EventScope {
public:
    EventScope() = default;

    explicit EventScope(Events* events);

    EventScope(const EventScope&) = delete;
    EventScope& operator=(const EventScope&) = delete;
    EventScope(EventScope&& other) noexcept;
    EventScope& operator=(EventScope&& other) noexcept;

    ~EventScope();
private:
    friend class Events;

    void add_callback(event_t event, std::size_t ix);

    void add_aux(std::size_t ix);

    void add_event(event_t event);

    void finalize();

    bool finalized = false;
    Events* events {nullptr};

    std::forward_list<std::size_t> cb_indicies {};
    std::forward_list<std::size_t> aux_indicies {};
    std::forward_list<event_t> evt_indicies {};
};

class Events {
    friend class EventScope;
public:
    Events() noexcept;
    
    std::unique_ptr<EventScope> begin_scope();

    void finalize_scope();

    event_t register_event(EventType type, int vector_size = -1);

    void register_callback(event_t id, void (*callback)(EventInfo, void *),
                           void *aux);

    template <class T, class... Args>
    void register_callback(event_t id, void (*callback)(T, Args...), Args... args) {
        struct Wrapper : public WrapperBase {
            explicit Wrapper(Args... args, void (*cb)(T, Args...))
                : args{args...}, cb{cb} {}
            std::tuple<Args...> args;
            void (*cb)(T, Args...);
        };
        auto* aux = new Wrapper{args..., callback};
        add_aux(aux);
        auto call = [](EventInfo i, void *aux) {
            Wrapper &w = *reinterpret_cast<Wrapper *>(aux);
            w.cb(i.get<T>(), std::get<Args>(w.args)...);
        };
        register_callback(id, call, aux);
    }

    template <class... Args>
    void register_callback(event_t id, void (*callback)(Args...), Args... args) {
        struct Wrapper : public WrapperBase {
            explicit Wrapper(Args... args, void (*cb)(Args...))
                : args{args...}, cb{cb} {}
            std::tuple<Args...> args;
            void (*cb)(Args...);
        };
        auto* aux = new Wrapper{args..., callback};
        add_aux(aux);
        auto call = [](EventInfo i, void *aux) {
            Wrapper &w = *reinterpret_cast<Wrapper *>(aux);
            w.cb(std::get<Args>(w.args)...);
        };
        register_callback(id, call, aux);
    }

    template<class T>
    void notify_event(event_t id, T t) {
        EventInfo info;
        info.set<T>(t);
        notify_event(id, info);
    }

    void notify_event(event_t id, EventInfo data);

    void handle_events();

private:
    struct WrapperBase {
        virtual ~WrapperBase() = default;
    };

    void add_aux(WrapperBase* ptr);

    void remove_callback(event_t event, std::size_t ix);

    void remove_aux(std::size_t ix);

    void remove_event(event_t event);

    void end_scope(EventScope* ptr);

    std::vector<std::unique_ptr<WrapperBase>> aux_data{};
    std::size_t free_aux = 0;

    struct CallbackData {
        void (*callback)(EventInfo, void *);
        void *aux;
    };
    struct EventData {
        EventType type;
        EventInfo data;
        bool triggered;
        std::vector<EventInfo> buffer{};
        std::vector<CallbackData> callbacks{};
        std::size_t free_ix = 0;
        int64_t last_ix = -1;
    };

    std::vector<EventData> events {};
    std::size_t free_ix = 1;
    int64_t last_ix = 0;

    std::vector<EventScope*> scopes {};

    std::unique_ptr<EventScope> root_scope{};
};

#endif
