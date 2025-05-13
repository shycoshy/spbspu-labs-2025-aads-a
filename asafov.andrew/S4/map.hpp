#ifndef SMART_PTR_MAP_HPP
#define SMART_PTR_MAP_HPP

#include <memory>
#include <utility>
#include <functional>
#include <initializer_list>
#include <stdexcept>

namespace asafov
{
  template <typename Key, typename Value, typename Comparator = std::less<Key>>
  class map
  {
    struct node
    {
      node(Key k, Value v, node* p):
        key1(std::move(k)),
        val1(std::move(v)),
        parent(p),
        type(false)
      {}

      Key key1, key2;
      Value val1, val2;
      std::unique_ptr<node> left;
      std::unique_ptr<node> middle;
      std::unique_ptr<node> right;
      node* parent;
      bool type;
    };

  public:
    map() = default;

    map(std::initializer_list<std::pair<const Key, Value>> init)
    {
      for (const auto& p : init)
      {
        insert(p.first, p.second);
      }
    }

    map(const map& other)
    {
      for (const auto& p : other)
      {
        insert(p.first, p.second);
      }
    }

    map(map&& other) noexcept = default;

    ~map() = default;

    map& operator=(const map& other)
    {
      if (this != &other)
      {
        clear();
        for (const auto& p : other)
        {
          insert(p.first, p.second);
        }
      }
      return *this;
    }

    map& operator=(map&& other) noexcept = default;

    void insert(const Key& key, const Value& value)
    {
      if (!root_)
      {
        root_ = std::make_unique<node>(key, value, nullptr);
        size_++;
        return;
      }

      node* where = find_node(key);
      if (!where) return;

      if (!compare(where->key1, key) && !compare(key, where->key1))
      {
        where->val1 = value;
        return;
      }
      if (where->type && !compare(where->key2, key) && !compare(key, where->key2))
      {
        where->val2 = value;
        return;
      }

      if (where->type)
      {
        if (compare(key, where->key1))
        {
          where->key2 = where->key1;
          where->val2 = where->val1;
          where->key1 = key;
          where->val1 = value;
        }
        else if (compare(where->key2, key))
        {
          where->key2 = key;
          where->val2 = value;
        }
        else
        {
          std::swap(where->key1, key);
          std::swap(where->val1, value);
          where->key2 = key;
          where->val2 = value;
        }
        split(where);
      }
      else
      {
        if (compare(key, where->key1))
        {
          where->key2 = where->key1;
          where->val2 = where->val1;
          where->key1 = key;
          where->val1 = value;
        }
        else
        {
          where->key2 = key;
          where->val2 = value;
        }
        where->type = true;
      }
      size_++;
    }

    Value& at(const Key& key)
    {
      node* n = find_node(key);
      if (!n) throw std::out_of_range("Key not found");
      if (!compare(n->key1, key) && !compare(key, n->key1)) return n->val1;
      if (n->type && !compare(n->key2, key) && !compare(key, n->key2)) return n->val2;
      throw std::out_of_range("Key not found");
    }

    const Value& at(const Key& key) const
    {
      const node* n = find_node(key);
      if (!n) throw std::out_of_range("Key not found");
      if (!compare(n->key1, key) && !compare(key, n->key1)) return n->val1;
      if (n->type && !compare(n->key2, key) && !compare(key, n->key2)) return n->val2;
      throw std::out_of_range("Key not found");
    }

    Value& operator[](const Key& key)
    {
      node* n = find_node(key);
      if (n)
      {
        if (!compare(n->key1, key) && !compare(key, n->key1)) return n->val1;
        if (n->type && !compare(n->key2, key) && !compare(key, n->key2)) return n->val2;
      }
      insert(key, Value{});
      n = find_node(key);
      return !compare(n->key1, key) && !compare(key, n->key1) ? n->val1 : n->val2;
    }

    void clear() noexcept
    {
      root_.reset();
      size_ = 0;
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
      friend class map;
    public:
      const_iterator() = default;

      const std::pair<const Key, Value> operator*() const
      {
        if (!current) throw std::out_of_range("Dereferencing end iterator");
        return pos == 0 ? std::make_pair(current->key1, current->val1) : std::make_pair(current->key2, current->val2);
      }

      const_iterator& operator++()
      {
        if (!current) return *this;
        if (current->type && pos == 0)
        {
          pos = 1;
          return *this;
        }
        if (current->right)
        {
          current = current->right.get();
          while (current->left) current = current->left.get();
          pos = 0;
          return *this;
        }
        node* p = current;
        while (p->parent)
        {
          if (p->parent->left.get() == p)
          {
            current = p->parent;
            pos = 0;
            return *this;
          }
          if (p->parent->type && p->parent->middle.get() == p)
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

      bool operator==(const const_iterator& other) const
      {
        return current == other.current && pos == other.pos;
      }

      bool operator!=(const const_iterator& other) const
      {
        return !(*this == other);
      }

    private:
      const_iterator(const node* n, unsigned p = 0) : current(n), pos(p) {}
      const node* current = nullptr;
      unsigned pos = 0;
    };

    const_iterator begin() const
    {
      if (!root_) return end();
      const node* n = root_.get();
      while (n->left) n = n->left.get();
      return const_iterator(n);
    }

    const_iterator end() const
    {
      return const_iterator();
    }

    const_iterator find(const Key& key) const
    {
      const node* n = find_node(key);
      if (!n) return end();
      if (!compare(n->key1, key) && !compare(key, n->key1)) return const_iterator(n);
      if (n->type && !compare(n->key2, key) && !compare(key, n->key2)) return const_iterator(n, 1);
      return end();
    }

  private:
    bool compare(const Key& a, const Key& b) const
    {
      return Comparator{}(a, b);
    }

    node* find_node(const Key& key) const
    {
      node* current = root_.get();
      while (current && current->left)
      {
        if (current->type)
        {
          if (compare(key, current->key1))
          {
            current = current->left.get();
          }
          else if (compare(current->key2, key))
          {
            current = current->right.get();
          }
          else
          {
            current = current->middle.get();
          }
        }
        else
        {
          current = compare(key, current->key1) ? current->left.get() : current->right.get();
        }
      }
      return current;
    }

    void split(node* n)
    {
      if (!n) return;

      if (!n->parent)
      {
        auto new_root = std::make_unique<node>(n->key2, n->val2, nullptr);
        new_root->left = std::make_unique<node>(n->key1, n->val1, new_root.get());
        new_root->right = std::make_unique<node>(n->key2, n->val2, new_root.get());

        if (n->left)
        {
          new_root->left->left = std::move(n->left);
          new_root->left->left->parent = new_root->left.get();
        }
        if (n->middle)
        {
          new_root->left->right = std::move(n->middle);
          new_root->left->right->parent = new_root->left.get();
        }
        if (n->right)
        {
          new_root->right->right = std::move(n->right);
          new_root->right->right->parent = new_root->right.get();
        }

        root_ = std::move(new_root);
      }
      else
      {
        node* parent = n->parent;
        if (parent->type)
        {
          if (parent->left.get() == n)
          {
            parent->key2 = parent->key1;
            parent->val2 = parent->val1;
            parent->key1 = n->key2;
            parent->val1 = n->val2;

            auto new_left = std::make_unique<node>(n->key1, n->val1, parent);
            auto new_middle = std::make_unique<node>(n->key2, n->val2, parent);

            if (n->left)
            {
              new_left->left = std::move(n->left);
              new_left->left->parent = new_left.get();
            }
            if (n->middle)
            {
              new_left->right = std::move(n->middle);
              new_left->right->parent = new_left.get();
            }
            if (n->right)
            {
              new_middle->right = std::move(n->right);
              new_middle->right->parent = new_middle.get();
            }

            parent->middle = std::move(new_middle);
            parent->left = std::move(new_left);
          }
          else if (parent->right.get() == n)
          {
            parent->key2 = n->key2;
            parent->val2 = n->val2;

            auto new_middle = std::make_unique<node>(n->key1, n->val1, parent);
            auto new_right = std::make_unique<node>(n->key2, n->val2, parent);

            if (n->left)
            {
              new_middle->left = std::move(n->left);
              new_middle->left->parent = new_middle.get();
            }
            if (n->middle)
            {
              new_middle->right = std::move(n->middle);
              new_middle->right->parent = new_middle.get();
            }
            if (n->right)
            {
              new_right->right = std::move(n->right);
              new_right->right->parent = new_right.get();
            }

            parent->middle = std::move(new_middle);
            parent->right = std::move(new_right);
          }
          else
          {
            Key temp_key = n->key2;
            Value temp_val = n->val2;

            auto new_left = std::make_unique<node>(parent->key1, parent->val1, parent);
            auto new_right = std::make_unique<node>(parent->key2, parent->val2, parent);

            if (parent->left)
            {
              new_left->left = std::move(parent->left);
              new_left->left->parent = new_left.get();
            }
            if (n->left)
            {
              new_left->right = std::move(n->left);
              new_left->right->parent = new_left.get();
            }
            if (n->right)
            {
              new_right->left = std::move(n->right);
              new_right->left->parent = new_right.get();
            }
            if (parent->right)
            {
              new_right->right = std::move(parent->right);
              new_right->right->parent = new_right.get();
            }

            parent->key1 = temp_key;
            parent->val1 = temp_val;
            parent->left = std::move(new_left);
            parent->right = std::move(new_right);
            parent->middle.reset();
            parent->type = false;
          }
          split(parent);
        }
        else
        {
          if (parent->left.get() == n)
          {
            parent->key2 = parent->key1;
            parent->val2 = parent->val1;
            parent->key1 = n->key2;
            parent->val1 = n->val2;
            parent->type = true;

            auto new_left = std::make_unique<node>(n->key1, n->val1, parent);
            auto new_middle = std::make_unique<node>(n->key2, n->val2, parent);

            if (n->left)
            {
              new_left->left = std::move(n->left);
              new_left->left->parent = new_left.get();
            }
            if (n->middle)
            {
              new_left->right = std::move(n->middle);
              new_left->right->parent = new_left.get();
            }
            if (n->right)
            {
              new_middle->right = std::move(n->right);
              new_middle->right->parent = new_middle.get();
            }

              parent->middle = std::move(new_middle);
              parent->left = std::move(new_left);
          }
          else
          {
            parent->key2 = n->key2;
            parent->val2 = n->val2;
            parent->type = true;

            auto new_middle = std::make_unique<node>(n->key1, n->val1, parent);
            auto new_right = std::make_unique<node>(n->key2, n->val2, parent);

            if (n->left)
            {
              new_middle->left = std::move(n->left);
              new_middle->left->parent = new_middle.get();
            }
            if (n->middle)
            {
              new_middle->right = std::move(n->middle);
              new_middle->right->parent = new_middle.get();
            }
            if (n->right)
            {
              new_right->right = std::move(n->right);
              new_right->right->parent = new_right.get();
            }

            parent->middle = std::move(new_middle);
            parent->right = std::move(new_right);
          }
        }
      }
    }

    std::unique_ptr<node> root_;
    size_t size_ = 0;
  };
}

#endif
