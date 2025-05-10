#ifndef MAP_HPP
#define MAP_HPP
#include <utility>
#include <cstddef>
#include <stdexcept>
#include <functional>

namespace asafov
{
  template< class Key, class Value, class Comparator = std::less<Key>>
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

      Key key1, key2;
      Value val1, val2;
      node* left, middle, right, parent;
      bool type;
    };

    node* find_approximately(const Key& k)
    {
      node* where = root_;
      while (where->left)
      {
        if (where->type)
        {
          if (Comparator(where->key1, k))
          {
            where = where->left;
          }
          else if (Comparator(k, where->key2))
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
          if (Comparator(where->key1, k))
          {
            where = where->left;
          }
          else
          {
            where = where->rigth;
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

    void insert(Key& k, Value& v)
    {
      if (!root_)
      {
        root_ = new node(k, v, nullptr);
        return;
      }
      node* where = find_approximately(k);
      while (true)
      {
        if (where->type) //3-нода
        {
          if (where->parent)
          {
            if (Comparator(where->key1, k))
            {
              std::swap(where->key1, k);
              std::swap(where->val1, v);
            }
            else if (Comparator(k, where->key2))
            {
              std::swap(where->key2, k);
              std::swap(where->val2, v);
            }
            where = where->parent;
          }
          else //root-нода
          {
            if (Comparator(where->key1, k))
            {
              std::swap(where->key1, k);
              std::swap(where->val1, v);
            }
            else if (Comparator(k, where->key2))
            {
              std::swap(where->key2, k);
              std::swap(where->val2, v);
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
        else //2-нода
        {
          if (Comparator(where->key1, k))
          {
            where->key2 = where->key1;
            where->key1 = k;
          }
          else
          {
            where->key2 = k;
          }
          break;
        }
      }
      size_++;
    }
    void clear(node* there)
    {
      if (there == nullptr) return;
      if (there->type) clear(there->middle);
      clear(there->left);
      clear(there->right);
      delete there;
    }

    Value& operator[](const Key& k)
    {
      node* where = find_approximately(k);
      if (Comparator(where->key1, k) && Comparator(k, where->key1))
      {
        return where->val1;
      }
      else if (where->type)
      {
        if (Comparator(where->key2, k) && Comparator(k, where->key2))
        {
          return where->val2;
        }
      }
      return Value();
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
      const_iterator() : current(nullptr), pos(0) {}
      const_iterator(const node* n, unsigned p = 0) : current(n), pos(p) {}
      const_iterator(const const_iterator& it) : current(it.current), pos(it.pos) {}

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
            return reinterpret_cast<const std::pair<const Key, Value>*>(&current->key1);
        return reinterpret_cast<const std::pair<const Key, Value>*>(&current->key2);
    }

    const_iterator& operator++()
    {
        if (!current) return *this;

        // Если текущий узел 3-нода и мы на первом ключе
        if (current->type && pos == 0)
        {
            pos = 1;
            return *this;
        }

        // Если есть правое поддерево - идем туда
        if (current->right)
        {
            current = current->right;
            while (current->left)
                current = current->left;
            pos = 0;
            return *this;
        }

        // Идем вверх по родителям
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

        // Достигли корня - конец обхода
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
    unsigned pos; // 0 - key1, 1 - key2
    friend class map;
};

    const_iterator begin()
    {
      if (!root_) return end();
      node* n = root_;
      while (n->left) n = n->left;
      return iterator(n);
    }
    const_iterator end()
    {
      return const_iterator(nullptr);
    }
    const_iterator cbegin()
    {
      if (!root_) return cend();
      node* n = root_;
      while (n->left) n = n->left;
      return const_iterator(n);
    }
    const_iterator cbegin() const
    {
      if (!root_) return cend();
      node* n = root_;
      while (n->left) n = n->left;
      return const_iterator(n);
    }
    const_iterator cend()
    {
      return const_iterator(nullptr);
    }
    const_iterator cend() const
    {
      return const_iterator(nullptr);
    }

    const_iterator find(const Key& k) const
    {
      const node* where = find_approximately(k);
      if (where && !Comparator(where->key1, k) && !Comparator(k, where->key1))
      {
        return const_iterator(where, 0);
      }
      else if (where && where->type && !Comparator(where->key2, k) && !Comparator(k, where->key2))
      {
        return const_iterator(where, 1);
      }
      return end();
    }
  private:
    node* root_;
    size_t size_;
  };
}

#endif
