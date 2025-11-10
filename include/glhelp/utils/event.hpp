#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

namespace glhelp {

/// Represents an event capable of holding EventDelegates.
/// When an event occurs (use of operator()) all registered EventDelegates
/// will be called.
template< typename T >
class Event {
public:
  Event() = default;

  Event(const Event< T >&) = delete;
  auto operator=(const Event< T >&) -> Event< T >& = delete;
  Event(Event< T >&& other) = delete;
  auto operator=(Event< T >&& other) noexcept -> Event< T >& = delete;

  template< typename... P >
  void operator()(P&&... params);

  struct EventDelegate {
  private:
    EventDelegate(std::function< T >&& delegate, std::size_t id, Event< T >& event);

  public:
    EventDelegate(const EventDelegate&) = delete;
    auto operator=(const EventDelegate&) -> EventDelegate& = delete;

    EventDelegate(EventDelegate&& other) noexcept;
    auto operator=(EventDelegate&& other) noexcept -> EventDelegate&;

    ~EventDelegate();

  private:
    template< typename... P >
    void call(P&&... params);

    void disconnect();

    std::function< T > delegate;
    std::size_t id;

    std::optional< std::reference_wrapper< Event< T > > > event;

    friend Event< T >;
  };

  [[nodiscard]] auto connect(std::function< T >&& lambda) -> std::shared_ptr< EventDelegate >;
  void operator+=(std::shared_ptr< EventDelegate >& delegate);

  void disconnect(EventDelegate& event_delegate);

  ~Event();

private:
  std::size_t id{};
  std::unordered_map< std::size_t, std::weak_ptr< EventDelegate > > delegates;
};

} // namespace glhelp

// Include implementation.
#include <glhelp/utils/event_impl.hpp>
