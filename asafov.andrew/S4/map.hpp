#ifndef MAP_HPP
#define MAP_HPP
#include <utility>
#include <cstddef>
#include <stdexcept>
#include <functional>

namespace asafov
{
  template< class Key, class Value, class Comparator = std::less<Key> >
  class map
  {
    struct node
    {
      node(Key k, Value v, node* p):
        key1(k),
        val1(v),
        left(nullptr),
        middle(nullptr),
        right(nullptr),
        parent(p),
        type(false)
      {}

      ~node()
      {
        left = nullptr;
        middle = nullptr;
        right = nullptr;
        parent = nullptr;
      }

      Key key1, key2;
      Value val1, val2;
      node* left;
      node* middle;
      node* right;
      node* parent;
      bool type;
    };

    node* find_approximately(const Key& k)
    {
      node* where = root_;
      while (where && where->left)
      {
        if (where->type)
        {
          if (Comparator{}(where->key1, k))
          {
            where = where->left;
          }
          else if (Comparator{}(k, where->key2))
          {
            where = where->right;
          }
          else
          {
            where = where->middle;
          }
        }
        else
        {
          if (Comparator{}(where->key1, k))
          {
            where = where->left;
          }
          else
          {
            where = where->right;
          }
        }
      }
      return where;
    }

    const node* find_approximately(const Key& k) const
    {
      const node* where = root_;
      while (where && where->left)
      {
        if (where->type)
        {
          if (Comparator{}(where->key1, k))
          {
            where = where->left;
          }
          else if (Comparator{}(k, where->key2))
          {
            where = where->right;
          }
          else
          {
            where = where->middle;
          }
        }
        else
        {
          if (Comparator{}(where->key1, k))
          {
            where = where->left;
          }
          else
          {
            where = where->right;
          }
        }
      }
      return where;
    }
  public:
    map() noexcept:
      root_(nullptr),
      size_(0)
    {}

    ~map()
    {
      clear(root_);
    }

    void insert(const Key& k, const Value& v)
    {
      if (!root_)
      {
        root_ = new node(k, v, nullptr);
        size_++;
        return;
      }
      node* where = find_approximately(k);
      while (true)
      {
        if (where->type)
        {
          if (where->parent)
          {
            if (Comparator{}(where->key1, k))
            {
              where->key1 = k;
              where->val1 = v;
            }
            else if (Comparator{}(k, where->key2))
            {
              where->key2 = k;
              where->val2 = v;
            }
            where = where->parent;
          }
          else
          {
            if (Comparator{}(where->key1, k))
            {
              where->key1 = k;
              where->val1 = v;
            }
            else if (Comparator{}(k, where->key2))
            {
              where->key2 = k;
              where->val2 = v;
            }
            node* temp = new node(k, v, nullptr);
            temp->left = new node(root_->key1, root_->val1, temp);
            temp->right = new node(root_->key2, root_->val2, temp);
            temp->left->left = root_->left;
            temp->right->right = root_->right;
            temp->left->right = root_->middle->left;
            temp->right->left = root_->middle->right;
            delete root_;
            root_ = temp;
          }
        }
        else
        {
          if (Comparator{}(where->key1, k))
          {
            where->key2 = where->key1;
            where->val2 = where->val1;
            where->key1 = k;
            where->val1 = v;
          }
          else
          {
            where->key2 = k;
            where->val2 = v;
          }
          break;
        }
      }
      size_++;
    }
    void clear(node* there)
    {
      if (!there) return;
      if (there->left) clear(there->left);
      if (there->middle) clear(there->middle);
      if (there->right) clear(there->right);
      there->parent = nullptr;
      delete there;
    }

    Value& operator[](const Key& k)
    {
      if (!root_)
      {
        root_ = new node(k, Value(), nullptr);
        size_++;
        return root_->val1;
      }

      node* where = find_approximately(k);
      if (!where)
      {
        root_ = new node(k, Value(), nullptr);
        size_++;
        return root_->val1;
      }

      if (Comparator{}(where->key1, k) && Comparator{}(k, where->key1))
      {
        return where->val1;
      }
      else if (where->type)
      {
        if (Comparator{}(where->key2, k) && Comparator{}(k, where->key2))
        {
          return where->val2;
        }
      }

      Value v;
      insert(k, v);
      where = find_approximately(k);
      if (Comparator{}(where->key1, k) && Comparator{}(k, where->key1))
      {
        return where->val1;
      }
      return where->val2;
    }
    Value& at(const Key& k)
    {
      Value& temp = operator[](k);
      if (temp != Value())
      {
        throw std::out_of_range("key not found!");
      }
      return temp;
    }
    const Value& at(const Key& k) const
    {
      Value& temp = operator[](k);
      if (temp != Value())
      {
        throw std::out_of_range("key not found!");
      }
      return temp;
    }

    void swap(map& other) noexcept
    {
      std::swap(root_, other->root_);
      std::swap(size_, other->size_);
    }
    size_t size() const noexcept
    {
      return size_;
    }
    bool empty() const noexcept
    {
      return size_ == 0;
    }

    class const_iterator
    {
    public:
      const_iterator():
        current(nullptr),
        pos(0)
      {}
      const_iterator(const node* n, unsigned p = 0):
        current(n),
        pos(p)
      {}

      const std::pair<const Key, Value>& operator*() const
      {
        if (pos == 0)
        {
          return *reinterpret_cast<const std::pair<const Key, Value>*>(&current->key1);
        }
        else
        {
          return *reinterpret_cast<const std::pair<const Key, Value>*>(&current->key2);
        }
      }

      const std::pair<const Key, Value>* operator->() const
      {
        if (pos == 0)
        {
          return reinterpret_cast<const std::pair<const Key, Value>*>(&current->key1);
        }
        return reinterpret_cast<const std::pair<const Key, Value>*>(&current->key2);
      }

      const_iterator& operator++()
      {
        if (current->type && pos == 0)
        {
          pos = 1;
          return *this;
        }

        if (current->right)
        {
          current = current->right;
          while (current->left) current = current->left;
          pos = 0;
          return *this;
        }

        const node* p = current;
        while (p->parent)
        {
          if (p->parent->left == p)
          {
            current = p->parent;
            pos = 0;
            return *this;
          }
          else if (p->parent->type && p->parent->middle == p)
          {
            current = p->parent;
            pos = 1;
            return *this;
          }
          p = p->parent;
        }

        current = nullptr;
        pos = 0;
        return *this;
      }

      const_iterator operator++(int)
      {
        const_iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      bool operator==(const const_iterator& other) const
      {
        return current == other.current && pos == other.pos;
      }

      bool operator!=(const const_iterator& other) const
      {
        return !(*this == other);
      }

    private:
      const node* current;
      unsigned pos;
      friend class map;
    };

    const_iterator begin() const
    {
      if (!root_) return end();
      const node* n = root_;
      while (n->left) n = n->left;
      return const_iterator(n);
    }
    const_iterator end() const
    {
      return const_iterator(nullptr);
    }
    const_iterator cbegin() const
    {
      return begin();
    }
    const_iterator cend() const
    {
      return end();
    }

    const_iterator find(const Key& k) const
    {
      if (!root_) return end();
      const node* where = find_approximately(k);
      if (!where) return end();

      if (Comparator{}(where->key1, k) && Comparator{}(k, where->key1))
      {
        return const_iterator(where);
      }
      else if (where->type)
      {
        if (Comparator{}(where->key2, k) && Comparator{}(k, where->key2))
        {
          return const_iterator(where, 1);
        }
      }
      return end();
    }
  private:
    node* root_;
    size_t size_;
  };
}
#endif
