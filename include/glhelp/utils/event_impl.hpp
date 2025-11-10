#pragma once

// 'classic' header-guard to avoid recursive definition
#ifndef REC_EVENT_GUARD
#define REC_EVENT_GUARD

#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>

#include <glhelp/utils/event.hpp>

namespace glhelp {

template< typename T >
Event< T >::EventDelegate::EventDelegate(std::function< T >&& delegate, std::size_t id, Event< T >& event)
    : delegate(delegate), id(id), event(event)
{
}

template< typename T >
Event< T >::EventDelegate::EventDelegate(EventDelegate&& other) noexcept
    : delegate(std::move(other.delegate)), id(other.id), event(std::move(other.event))
{
  other.event.reset();
}

template< typename T >
auto Event< T >::EventDelegate::operator=(EventDelegate&& other) noexcept -> EventDelegate&
{
  if (this == &other) {
    return *this;
  }

  disconnect();

  delegate = std::move(other.delegate);
  id = other.id;
  event = std::move(other.event);
  other.event.reset();
}

template< typename T >
Event< T >::EventDelegate::~EventDelegate()
{
  if (event.has_value()) {
    event.value().get().disconnect(*this);
  }
}

template< typename T >
template< typename... P >
void Event< T >::EventDelegate::call(P&&... params)
{
  delegate(std::forward< P >(params)...);
}

template< typename T >
void Event< T >::EventDelegate::disconnect() { event.reset(); }

template< typename T >
template< typename... P >
void Event< T >::operator()(P&&... params)
{
  for (const auto& [key, delegate] : delegates) {
    delegate.lock()->call(std::forward< P >(params)...);
  }
}

template< typename T >
[[nodiscard]] auto Event< T >::connect(std::function< T >&& lambda) -> std::shared_ptr< EventDelegate >
{
  auto new_delegate{std::make_shared< EventDelegate >(EventDelegate(std::move(lambda), id++, *this))};
  delegates[new_delegate->id] = new_delegate;
  return new_delegate;
}

template< typename T >
void Event< T >::operator+=(std::shared_ptr< EventDelegate >& delegate)
{
  delegates[delegate->id] = delegate;
}

template< typename T >
void Event< T >::disconnect(EventDelegate& event_delegate)
{
  delegates.erase(event_delegate.id);
}

template< typename T >
Event< T >::~Event()
{
  for (auto& [key, event] : delegates) {
    event.lock()->disconnect();
  }
}

} // namespace glhelp

#endif
