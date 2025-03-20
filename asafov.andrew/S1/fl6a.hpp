#ifndef FL6A_HPP
#define FL6A_HPP
#include <memory>

namespace asafov
{
  template <typename T>
  class Forward_list
  {
    struct Node
    {
      explicit Node(const T& value, Node* ptr = nullptr) noexcept:
      data_(value),
      next_(ptr)
      {}
      T data_;
      Node* next_;
    };
  public:
    forward_list() noexcept:
    head_(nullptr),
    tail_(nullptr),
    size_(0)
    {}
    forward_list(const forward_list& list):
    forward_list()
    {
      for (auto i = list.cbegin(); i != list.cend(); ++i)
      {
        push_back();
      }
    }
    ~forward_list() noexcept
    {
      clear();
    }

    forward_list& operator=(const forward_list& list)
    {
      clear();
      for (auto data : list)
      {
        push_back(data);
      }
      return *this;
    }

    class const_iterator
    {
      friend class forward_list;
    public:
      const_iterator():
      current_(nullptr),
      last_(nullptr)
      {}
      const_iterator(Node* node, Node* last):
      current_(node),
      last_(last)
      {}
      const_iterator(const const_iterator& data) noexcept:
      current_(data.current_),
      last_(data.last_)
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

      const_iterator& operator=(const const_iterator& data) noexcept
      {
        if (this != &data)
        {
          current_ = data.current_;
          last_ = data.last_;
        }
        return *this;
      }

      bool operator==(const const_iterator& rhs) const
      {
        return current_ == rhs.current_ && last_ == rhs.last_;
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

    bool empty() const
    {
      return !head_;
    }
    size_t size() const
    {
      return size_;
    }
    void pop_front()
    {
      size_--;
      Node* temp = head_;
      if (head_ == nullptr)
      {}
      else
      {
        if (head_ == tail_)
        {
          delete head_;
          *this = forward_list();
        }
        else
        {
          head_ = head_->next_;
          tail_->next_ = head_;
          delete temp;
        }
      }
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
      size_++;
    }
    void clear() noexcept
    {
      while (size_ != 0)
      {
        pop_front();
      }
    }

  private:
    Node* head_;
    Node* tail_;
    size_t size_;
  };
}
#endif
