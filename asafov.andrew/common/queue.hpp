#ifndef QUEUE_HPP
#define QUEUE_HPP
#include "deque.hpp"

namespace asafov
{
  template<class T>
  class queue : public deque<T>
  {
  public:
    void push(const T& value)
    {
        this->push_back(value);
    }

    void pop()
    {
        this->pop_front();
    }

    T& front()
    {
        return deque<T>::front();
    }

    const T& front() const
    {
        return deque<T>::front();
    }
  };
}
#endif
