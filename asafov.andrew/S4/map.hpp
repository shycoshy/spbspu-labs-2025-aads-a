#ifndef MAP_HPP
#define MAP_HPP
#include <iostream>
#include <stdexcept>

namespace asafov
{
  template< typename Key, typename Value >
  class map
  {
    struct node
    {
      node(const Key& k, const Value& v):
        key1(k),
        val1(v),
        hasSecond(false),
        left(nullptr),
        middle(nullptr),
        right(nullptr),
        parent(nullptr)
      {
      }

      Key key1, key2;
      Value val1, val2;
      bool hasSecond;
      node* left;
      node* middle;
      node* right;
      node* parent;
    };

  public:
    map() = default;

    map(const map& other)
    {
      root_ = copy_node(other.root_, nullptr);
      tree_size_ = other.tree_size_;
    }

    map& operator=(const map& other)
    {
      if (this == &other) return *this;
      clear();
      root_ = copy_node(other.root_, nullptr);
      tree_size_ = other.tree_size_;
      return *this;
    }

    map(map&& other) noexcept
      : root_(other.root_), tree_size_(other.tree_size_)
    {
      other.root_ = nullptr;
      other.tree_size_ = 0;
    }

    map& operator=(map&& other) noexcept
    {
      if (this != &other)
      {
        clear();
        root_ = other.root_;
        tree_size_ = other.tree_size_;
        other.root_ = nullptr;
        other.tree_size_ = 0;
      }
      return *this;
    }

    ~map()
    {
      clear();
    }

    bool empty() const
    {
      return tree_size_ == 0;
    }

    size_t size() const
    {
      return tree_size_;
    }

    Value* find(const Key& key) const
    {
      node* n = find_node(root_, key);
      if (!n) return nullptr;
      if (n->key1 == key) return &n->val1;
      if (n->hasSecond && n->key2 == key) return &n->val2;
      return nullptr;
    }

    void swap(map& other) noexcept
    {
      std::swap(root_, other.root_);
      std::swap(tree_size_, other.tree_size_);
    }

    Value& at(const Key& key)
    {
      node* n = find_node(root_, key);
      if (!n) throw std::out_of_range("Key not found");
      if (n->key1 == key) return n->val1;
      if (n->hasSecond && n->key2 == key) return n->val2;
      throw std::out_of_range("Key not found");
    }

    Value& operator[](const Key& key)
    {
      node* n = find_node(root_, key);
      if (n)
      {
        if (n->key1 == key) return n->val1;
        if (n->hasSecond && n->key2 == key) return n->val2;
      }
      insert(key, Value());
      return at(key);
    }

    void insert(const Key& key, const Value& value)
    {
      bool inserted = false;
      root_ = insert_node(root_, nullptr, key, value, inserted);
      if (inserted) tree_size_++;
    }

    void erase(const Key& key)
    {
      bool removed = false;
      root_ = erase_node(root_, key, removed);
      if (removed) tree_size_--;
    }

    void clear()
    {
      destroy_tree(root_);
      root_ = nullptr;
      tree_size_ = 0;
    }

  private:
    node* copy_node(node* n, node* parent)
    {
      if (!n) return nullptr;
      node* newnode = new node(n->key1, n->val1);
      newnode->parent = parent;
      if (n->hasSecond)
      {
        newnode->key2 = n->key2;
        newnode->val2 = n->val2;
        newnode->hasSecond = true;
      }
      newnode->left = copy_node(n->left, newnode);
      newnode->middle = copy_node(n->middle, newnode);
      newnode->right = copy_node(n->right, newnode);
      return newnode;
    }

    void destroy_tree(node* n)
    {
      if (!n) return;
      destroy_tree(n->left);
      destroy_tree(n->middle);
      destroy_tree(n->right);
      delete n;
    }

    node* find_node(node* n, const Key& key) const
    {
      if (!n) return nullptr;
      if (key == n->key1 || (n->hasSecond && key == n->key2)) return n;
      if (key < n->key1) return find_node(n->left, key);
      if (!n->hasSecond || key < n->key2) return find_node(n->middle, key);
      return find_node(n->right, key);
    }

    node* insert_node(node* n, node* parent, const Key& key, const Value& value, bool& inserted)
    {
      if (!n)
      {
        inserted = true;
        node* newnode = new node(key, value);
        newnode->parent = parent;
        return newnode;
      }

      if (key == n->key1 || (n->hasSecond && key == n->key2)) return n;

      if (!n->hasSecond)
      {
        if (key < n->key1)
        {
          n->key2 = n->key1;
          n->val2 = n->val1;
          n->key1 = key;
          n->val1 = value;
        }
        else
        {
          n->key2 = key;
          n->val2 = value;
        }
        n->hasSecond = true;
        inserted = true;
        return n;
      }

      if (key < n->key1)
      {
        node* child = insert_node(n->left, n, key, value, inserted);
        n->left = child;
      }
      else if (key < n->key2)
      {
        node* child = insert_node(n->middle, n, key, value, inserted);
        n->middle = child;
      }
      else
      {
        node* child = insert_node(n->right, n, key, value, inserted);
        n->right = child;
      }

      return n;
    }

    node* erase_node(node* n, const Key& key, bool& removed)
    {
      if (!n) return nullptr;

      if (key == n->key1)
      {
        if (n->hasSecond)
        {
          n->key1 = n->key2;
          n->val1 = n->val2;
          n->hasSecond = false;
          removed = true;
          return n;
        }
        delete n;
        removed = true;
        return nullptr;
      }

      if (n->hasSecond && key == n->key2)
      {
        n->hasSecond = false;
        removed = true;
        return n;
      }

      if (key < n->key1)
        n->left = erase_node(n->left, key, removed);
      else if (!n->hasSecond || key < n->key2)
        n->middle = erase_node(n->middle, key, removed);
      else
        n->right = erase_node(n->right, key, removed);

      return n;
    }

    node* find_min(node* n) const
    {
      while (n && n->left)
        n = n->left;
      return n;
    }

  public:
    class iterator
    {
      node* current_;
      bool secondKey_;

    public:
      iterator(node* n = nullptr)
        : current_(n), secondKey_(false)
      {
      }

      std::pair< const Key&, Value& > operator*() const
      {
        if (secondKey_)
          return {current_->key2, current_->val2};
        else
          return {current_->key1, current_->val1};
      }

      iterator& operator++()
      {
        if (!current_) return *this;

        if (!secondKey_ && current_->hasSecond)
        {
          secondKey_ = true;
          return *this;
        }

        secondKey_ = false;

        if (current_->hasSecond && current_->middle)
        {
          current_ = find_min(current_->middle);
        }
        else if (current_->right)
        {
          current_ = find_min(current_->right);
        }
        else
        {
          node* p = current_->parent;
          while (p && (current_ == p->right || (p->hasSecond && current_ == p->middle)))
          {
            current_ = p;
            p = p->parent;
          }
          current_ = p;
        }

        return *this;
      }

      bool operator!=(const iterator& other) const
      {
        return current_ != other.current_;
      }

      bool operator==(const iterator& other) const
      {
        return current_ == other.current_;
      }

    private:
      node* find_min(node* n) const
      {
        while (n && n->left)
          n = n->left;
        return n;
      }
    };

    class const_iterator
    {
      node* current_;
      bool secondKey_;

    public:
      const_iterator(node* n = nullptr)
        : current_(n), secondKey_(false)
      {
      }

      std::pair< const Key&, const Value& > operator*() const
      {
        if (secondKey_)
          return {current_->key2, current_->val2};
        else
          return {current_->key1, current_->val1};
      }

      const_iterator& operator++()
      {
        if (!current_) return *this;

        if (!secondKey_ && current_->hasSecond)
        {
          secondKey_ = true;
          return *this;
        }

        secondKey_ = false;

        if (current_->hasSecond && current_->middle)
        {
          current_ = find_min(current_->middle);
        }
        else if (current_->right)
        {
          current_ = find_min(current_->right);
        }
        else
        {
          node* p = current_->parent;
          while (p && (current_ == p->right || (p->hasSecond && current_ == p->middle)))
          {
            current_ = p;
            p = p->parent;
          }
          current_ = p;
        }

        return *this;
      }

      bool operator!=(const const_iterator& other) const
      {
        return current_ != other.current_;
      }

      bool operator==(const const_iterator& other) const
      {
        return current_ == other.current_;
      }

    private:
      node* find_min(node* n) const
      {
        while (n && n->left) n = n->left;
        return n;
      }
    };

    iterator begin()
    {
      return iterator(find_min(root_));
    }

    iterator end()
    {
      return iterator(nullptr);
    }

    const_iterator cbegin() const
    {
      return const_iterator(find_min(root_));
    }

    const_iterator cend() const
    {
      return const_iterator(nullptr);
    }

  private:
    node* root_ = nullptr;
    size_t tree_size_ = 0;
  };
}
#endif
