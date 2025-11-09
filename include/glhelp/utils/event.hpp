#pragma once

#include <functional>
#include <optional>

namespace glhelp {

/// Represents an event capable of holding EventDelegates.
/// When an event occurs (use of operator()) all registered EventDelegates
/// will be called.
template< typename T >
class Event {
public:
  Event()
  {
  }

  Event(const Event< T >&) = delete;
  Event< T >& operator=(const Event< T >&) = delete;

  Event(Event< T >&& other)
      : delegates(std::move(other.delegates))
  {
  }

  Event< T >& operator=(Event< T >&& other)
  {
    std::swap(delegates, other.delegates);
  }

  template< typename... P >
  void operator()(P&&... params)
  {
    for (const auto& [key, delegate] : delegates) {
      delegate(std::forward< P >(params)...);
    }
  }

  struct EventDelegate {
    EventDelegate(std::function< T >&& delegate, std::size_t id, Event< T >& event)
        : delegate(delegate), id(id), event(event)
    {
    }

    ~EventDelegate()
    {
      if (event.has_value()) {
        event.value().get().disconnect(*this);
      }
    }

    template< typename... P >
    void operator()(P&&... params)
    {
      delegate(std::forward< P >(params)...);
    }

    void disconnect() { event.reset(); }

    std::function< T > delegate;
    std::size_t id;

    std::optional< std::reference_wrapper< Event< T > > > event;

    friend Event< T >;
  };

  EventDelegate new_delegate(std::function< T >&& lambda)
  {
    return EventDelegate(std::move(lambda), id++, *this);
  }

  void connect(EventDelegate& delegate)
  {
    delegates.insert({delegate.id, delegate});
  }

  void disconnect(EventDelegate& event_delegate)
  {
    delegates.erase(event_delegate.id);
  }

  ~Event()
  {
    for (auto& [key, event] : delegates) {
      event.get().disconnect();
    }
  }

private:
  std::size_t id{};
  std::unordered_map< std::size_t, std::reference_wrapper< EventDelegate > > delegates;
};

} // namespace glhelp
