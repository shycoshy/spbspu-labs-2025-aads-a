#ifndef FL6A_HPP
#define FL6A_HPP
#include <memory>
namespace asafov
{
  template <typename T>
  class forward_list
  {
    struct Node
    {
      T data_;
      Node* next_;

      Node(const T& value, Node* ptr) noexcept:
      data_(value),
      next_(ptr)
      {}

      Node(const T& value) noexcept:
      data_(value),
      next_(nullptr)
      {}
    };
  public:
    forward_list() noexcept:
    head_(nullptr),
    tail_(nullptr)
    {}

    ~forward_list() noexcept
    {
      clear();
    }

    class const_iterator
    {
      friend class Forward_list;
    public:
      const_iterator():
      current_(nullptr),
      last_(nullptr)
      {}

      const_iterator(Node* node, Node* last):
      current_(node),
      last_(last)
      {}

      ~const_iterator() = default;

      const T& operator*() const
      {
        return current_->data_;
      }

      T* operator->() const
      {
        return std::addressof(current_->data_);
      }

      const_iterator& operator++()
      {
        if (current_ && current_ != last_)
        {
          current_ = current_->next_;
        }
        else
        {
          current_ = nullptr;
        }
        return *this;
      }

      bool operator==(const const_iterator& rhs) const
      {
        return current_ == rhs.current_;
      }

      bool operator!=(const const_iterator& rhs) const
      {
        return !(*this == rhs);
      }

    private:
      Node* current_;
      Node* last_;
    };

    const_iterator cbegin() const
    {
      return const_iterator(head_, tail_);
    }
    const_iterator cend() const
    {
      return const_iterator(nullptr, tail_);
    }
    const_iterator begin() const
    {
      return const_iterator(head_, tail_);
    }
    const_iterator end() const
    {
      return const_iterator(nullptr, tail_);
    }

    bool empty()
    {
      return !head_;
    }

    size_t size() const
    {
      size_t i = 0;
      for (const_iterator it = cbegin(); it != cend(); ++it)
      {
        ++i;
      }
      return i;
    }

    void pop_front()
    {
      if (!head_) return;
      else if (head_ != tail_)
      {
        auto temp = head_;
        head_ = head_->next_;
        tail_->next_ = head_;
        delete head_;
      }
      else
      {
        delete head_;
        head_ = nullptr;
        tail_ = nullptr;
      }
      --size_;
    }

    void clear()
    {
      do
      {
        pop_front();
      }
      while (!empty())
    }

    void push_back(const T& value)
    {
      Node* new_node = new Node(value);
      if (!head_)
      {
        head_ = new_node;
        tail_ = new_node;
        new_node->next_ = head_;
      }
      else
      {
        tail_->next_ = new_node;
        new_node->next_ = head_;
        tail_ = new_node;
      }
      ++size_;
    }
  private:
    Node* head_;
    Node* tail_;
    size_t size_;
  };
}
#endif
