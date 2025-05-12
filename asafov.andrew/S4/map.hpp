#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include <initializer_list>

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

      Key key1, key2;
      Value val1, val2;
      node* left;
      node* middle;
      node* right;
      node* parent;
      bool type;
    };

    node* find_approximately(const Key& k) const
    {
      node* where = root_;
      while (where && where->left)
      {
        if (where->type)
        {
          if (Comparator{}(k, where->key1))
          {
            where = where->left;
          }
          else if (Comparator{}(where->key2, k))
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
          if (Comparator{}(k, where->key1))
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

    void split(node* n)
    {
      if (!n) return;

      if (!n->parent)
      {
        node* new_root = new node(n->key2, n->val2, nullptr);
        new_root->left = new node(n->key1, n->val1, new_root);
        new_root->right = new node(n->key2, n->val2, new_root);
        
        if (n->left) 
        {
          new_root->left->left = n->left;
          n->left->parent = new_root->left;
        }
        if (n->middle)
        {
          new_root->left->right = n->middle;
          n->middle->parent = new_root->left;
        }
        if (n->middle && n->middle->right)
        {
          new_root->right->left = n->middle->right;
          n->middle->right->parent = new_root->right;
        }
        if (n->right)
        {
          new_root->right->right = n->right;
          n->right->parent = new_root->right;
        }
        
        delete n;
        root_ = new_root;
      }
      else
      {
        node* p = n->parent;
        if (p->type)
        {
          if (p->left == n)
          {
            p->key2 = p->key1;
            p->val2 = p->val1;
            p->key1 = n->key2;
            p->val1 = n->val2;
            
            node* new_left = new node(n->key1, n->val1, p);
            node* new_middle = new node(n->key2, n->val2, p);
            
            if (n->left)
            {
              new_left->left = n->left;
              n->left->parent = new_left;
            }
            if (n->middle)
            {
              new_left->right = n->middle;
              n->middle->parent = new_left;
            }
            if (n->middle && n->middle->right)
            {
              new_middle->left = n->middle->right;
              n->middle->right->parent = new_middle;
            }
            if (n->right)
            {
              new_middle->right = n->right;
              n->right->parent = new_middle;
            }
            
            p->middle = new_middle;
            p->left = new_left;
          }
          else if (p->right == n)
          {
            p->key2 = n->key2;
            p->val2 = n->val2;
            
            node* new_middle = new node(n->key1, n->val1, p);
            node* new_right = new node(n->key2, n->val2, p);
            
            if (n->left)
            {
              new_middle->left = n->left;
              n->left->parent = new_middle;
            }
            if (n->middle)
            {
              new_middle->right = n->middle;
              n->middle->parent = new_middle;
            }
            if (n->middle && n->middle->right)
            {
              new_right->left = n->middle->right;
              n->middle->right->parent = new_right;
            }
            if (n->right)
            {
              new_right->right = n->right;
              n->right->parent = new_right;
            }
            
            p->middle = new_middle;
            p->right = new_right;
          }
          else
          {
            Key temp_key = n->key2;
            Value temp_val = n->val2;
            node* new_left = new node(p->key1, p->val1, p);
            node* new_right = new node(p->key2, p->val2, p);
            
            if (p->left)
            {
              new_left->left = p->left;
              p->left->parent = new_left;
            }
            if (n->left)
            {
              new_left->right = n->left;
              n->left->parent = new_left;
            }
            if (n->right)
            {
              new_right->left = n->right;
              n->right->parent = new_right;
            }
            if (p->right)
            {
              new_right->right = p->right;
              p->right->parent = new_right;
            }
            
            p->key1 = temp_key;
            p->val1 = temp_val;
            p->left = new_left;
            p->right = new_right;
            p->middle = nullptr;
            p->type = false;
          }
          delete n;
          split(p);
        }
        else
        {
          if (p->left == n)
          {
            p->key2 = p->key1;
            p->val2 = p->val1;
            p->key1 = n->key2;
            p->val1 = n->val2;
            p->type = true;
            
            node* new_left = new node(n->key1, n->val1, p);
            node* new_middle = new node(n->key2, n->val2, p);
            
            if (n->left)
            {
              new_left->left = n->left;
              n->left->parent = new_left;
            }
            if (n->middle)
            {
              new_left->right = n->middle;
              n->middle->parent = new_left;
            }
            if (n->middle && n->middle->right)
            {
              new_middle->left = n->middle->right;
              n->middle->right->parent = new_middle;
            }
            if (n->right)
            {
              new_middle->right = n->right;
              n->right->parent = new_middle;
            }
            
            p->left = new_left;
            p->middle = new_middle;
          }
          else
          {
            p->key2 = n->key2;
            p->val2 = n->val2;
            p->type = true;
            
            node* new_middle = new node(n->key1, n->val1, p);
            node* new_right = new node(n->key2, n->val2, p);
            
            if (n->left)
            {
              new_middle->left = n->left;
              n->left->parent = new_middle;
            }
            if (n->middle)
            {
              new_middle->right = n->middle;
              n->middle->parent = new_middle;
            }
            if (n->middle && n->middle->right)
            {
              new_right->left = n->middle->right;
              n->middle->right->parent = new_right;
            }
            if (n->right)
            {
              new_right->right = n->right;
              n->right->parent = new_right;
            }
            
            p->middle = new_middle;
            p->right = new_right;
          }
          delete n;
        }
      }
    }

  public:
    map() noexcept:
      root_(nullptr),
      size_(0)
    {}
    map(std::initializer_list<std::pair<const Key, Value>> init):
      map()
    {
      for (const auto& p : init)
      {
        insert(p.first, p.second);
      }
    }
    map(const map& other):
      map()
    {
      for (const auto& p : other)
      {
        insert(p.first, p.second);
      }
    }
    map(map&& other) noexcept:
      root_(other.root_),
      size_(other.size_)
    {
      other.root_ = nullptr;
      other.size_ = 0;
    }
    ~map()
    {
      clear();
    }

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
    map& operator=(map&& other) noexcept
    {
      if (this != &other)
      {
        clear();
        root_ = other.root_;
        size_ = other.size_;
        other.root_ = nullptr;
        other.size_ = 0;
      }
      return *this;
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
      if (!where) return;

      if (!Comparator{}(where->key1, k) && !Comparator{}(k, where->key1))
      {
        where->val1 = v;
        return;
      }
      if (where->type && !Comparator{}(where->key2, k) && !Comparator{}(k, where->key2))
      {
        where->val2 = v;
        return;
      }
      if (where->type)
      {
        if (Comparator{}(k, where->key1))
        {
          where->key2 = where->key1;
          where->val2 = where->val1;
          where->key1 = k;
          where->val1 = v;
        }
        else if (Comparator{}(where->key2, k))
        {
          where->key2 = k;
          where->val2 = v;
        }
        else
        {
          Key temp_key = where->key1;
          Value temp_val = where->val1;
          where->key1 = k;
          where->val1 = v;
          where->key2 = temp_key;
          where->val2 = temp_val;
        }
        split(where);
      }
      else
      {
        if (Comparator{}(k, where->key1))
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
        where->type = true;
      }
      size_++;
    }

    void clear() noexcept
    {
      clear(root_);
      root_ = nullptr;
      size_ = 0;
    }

    Value& operator[](const Key& k)
    {
      node* where = find_approximately(k);
      if (where)
      {
        if (!Comparator{}(where->key1, k) && !Comparator{}(k, where->key1))
        {
          return where->val1;
        }
        if (where->type && !Comparator{}(where->key2, k) && !Comparator{}(k, where->key2))
        {
          return where->val2;
        }
      }
      Value v{};
      insert(k, v);
      where = find_approximately(k);
      if (!where) throw std::runtime_error("Insertion failed");
      
      if (!Comparator{}(where->key1, k) && !Comparator{}(k, where->key1))
      {
        return where->val1;
      }
      return where->val2;
    }

    Value& at(const Key& k)
    {
      node* where = find_approximately(k);
      if (!where)
      {
        throw std::out_of_range("key not found!");
      }
      if (!Comparator{}(where->key1, k) && !Comparator{}(k, where->key1))
      {
        return where->val1;
      }
      if (where->type && !Comparator{}(where->key2, k) && !Comparator{}(k, where->key2))
      {
        return where->val2;
      }
      throw std::out_of_range("key not found!");
    }

    const Value& at(const Key& k) const
    {
      const node* where = find_approximately(k);
      if (!where)
      {
        throw std::out_of_range("key not found!");
      }
      if (!Comparator{}(where->key1, k) && !Comparator{}(k, where->key1))
      {
        return where->val1;
      }
      if (where->type && !Comparator{}(where->key2, k) && !Comparator{}(k, where->key2))
      {
        return where->val2;
      }
      throw std::out_of_range("key not found!");
    }

    void swap(map& other) noexcept
    {
      std::swap(root_, other.root_);
      std::swap(size_, other.size_);
    }

    size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    class const_iterator
    {
      friend class map;
    public:
      const_iterator():
        current(nullptr),
        pos(0)
      {}
      const_iterator(const node* n, unsigned p = 0):
        current(n),
        pos(p)
      {}

      const std::pair<const Key, Value> operator*() const
      {
        if (!current) throw std::out_of_range("Dereferencing end iterator");
        if (pos == 0)
        {
          return std::make_pair(current->key1, current->val1);
        }
        return std::make_pair(current->key2, current->val2);
      }

      const std::pair<const Key, Value>* operator->() const
      {
        if (!current) throw std::out_of_range("Dereferencing end iterator");
        if (pos == 0)
        {
          return reinterpret_cast<const std::pair<const Key, Value>*>(&current->key1);
        }
        return reinterpret_cast<const std::pair<const Key, Value>*>(&current->key2);
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
      const node* where = find_approximately(k);
      if (!where) return end();
      if (!Comparator{}(where->key1, k) && !Comparator{}(k, where->key1))
      {
        return const_iterator(where);
      }
      if (where->type && !Comparator{}(where->key2, k) && !Comparator{}(k, where->key2))
      {
        return const_iterator(where, 1);
      }
      return end();
    }

  private:
    void clear(node* n) noexcept
    {
      if (!n) return;
      clear(n->left);
      clear(n->middle);
      clear(n->right);
      delete n;
    }

    node* root_;
    size_t size_;
  };
}
#endif
