#ifndef DEQUE_HPP
#define DEQUE_HPP

#include <iostream>
#include <stdexcept>

namespace asafov
{
  template<typename T>
  class deque
  {
  public:
    deque():
      data_(nullptr),
      capacity_(0),
      size_(0),
      front_index_(0),
      back_index_(0)
    {}

    ~deque()
    {
      delete[] data_;
    }

    void push_back(const T& value)
    {
      if (size_ == capacity_)
      {
        resize(capacity_ == 0 ? 1 : capacity_ * 2);
      }
      data_[back_index_] = value;
      back_index_ = (back_index_ + 1) % capacity_;
      ++size_;
    }

    void push_front(const T& value)
    {
      if (size_ == capacity_)
      {
        resize(capacity_ == 0 ? 1 : capacity_ * 2);
      }
      front_index_ = (front_index_ - 1 + capacity_) % capacity_;
      data_[front_index_] = value;
      ++size_;
    }

    void pop_back()
    {
      if (empty())
      {
        throw std::out_of_range("deque::pop_back(): empty deque");
      }
      back_index_ = (back_index_ - 1 + capacity_) % capacity_;
      --size_;
    }

    void pop_front()
    {
      if (empty())
      {
        throw std::out_of_range("deque::pop_front(): empty deque");
      }
      front_index_ = (front_index_ + 1) % capacity_;
      --size_;
    }

    T& front()
    {
      if (empty())
      {
        throw std::out_of_range("deque::front(): empty deque");
      }
      return data_[front_index_];
    }

    T& back()
    {
      if (empty())
      {
        throw std::out_of_range("deque::back(): empty deque");
      }
      return data_[(back_index_ - 1 + capacity_) % capacity_];
    }

    bool empty() const
    {
      return size_ == 0;
    }

    size_t size() const
    {
      return size_;
    }

    void clear()
    {
      front_index_ = 0;
      back_index_ = 0;
      size_ = 0;
    }
  private:
    void resize(const size_t& new_capacity)
    {
      T* new_data = new T[new_capacity];
      for (size_t i = 0; i < size_; ++i)
      {
        new_data[i] = std::move(data_[(front_index_ + i) % capacity_]);
      }
      delete[] data_;
      data_ = new_data;
      front_index_ = 0;
      back_index_ = size_;
      capacity_ = new_capacity;
    }

    T* data_;
    size_t capacity_;
    size_t size_;
    size_t front_index_;
    size_t back_index_;
  };
}
#endif
