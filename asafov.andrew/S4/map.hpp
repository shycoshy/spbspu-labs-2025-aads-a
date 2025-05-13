#ifndef MAP_HPP
#define MAP_HPP
#include <utility>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include <stack>

namespace asafov
{
  template< class Key, class Value, class Comparator = std::less< Key > >
  class map
  {
    struct node
    {
      node(Key k, Value v, node* p):
        pair1(k, v),
        left(nullptr),
        middle(nullptr),
        right(nullptr),
        parent(p),
        type(false)
      {
      }

      std::pair< Key, Value > pair1, pair2;
      node *left, *middle, *right, *parent;
      bool type;
    };

    node* find_approximately(const Key& k) const
    {
      node* where = root_;
      while (where && where->left)
      {
        if (where->type)
        {
          if (Comparator(k, where->pair1.first))
          {
            where = where->left;
          }
          else if (Comparator(where->pair2.first, k))
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
          if (Comparator(k, where->pair1.first))
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
    {
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
        if (where->type) //3-node
        {
          if (where->parent)
          {
            if (Comparator(k, where->pair1.first))
            {
              std::swap(where->pair1.first, k);
              std::swap(where->pair1.second, v);
            }
            else if (Comparator(where->pair2.first, k))
            {
              std::swap(where->pair2.first, k);
              std::swap(where->pair2.second, v);
            }
            where = where->parent;
          }
          else //root-node
          {
            if (Comparator(k, where->pair1.first))
            {
              std::swap(where->pair1.first, k);
              std::swap(where->pair1.second, v);
            }
            else if (Comparator(where->pair2.first, k))
            {
              std::swap(where->pair2.first, k);
              std::swap(where->pair2.second, v);
            }
            node* temp = new node(k, v, nullptr);
            temp->left = new node(where->pair1.first, where->pair1.second, temp);
            temp->right = new node(where->pair2.first, where->pair2.second, temp);
            temp->left->left = where->left;
            temp->right->right = where->right;
            if (where->left) where->left->parent = temp->left;
            if (where->right) where->right->parent = temp->right;
            temp->left->right = where->middle;
            if (where->middle) where->middle->parent = temp->left;
            delete where;
            root_ = temp;
            size_++;
            return;
          }
        }
        else //2-node
        {
          if (Comparator(k, where->pair1.first))
          {
            where->pair2 = where->pair1;
            where->pair1 = std::make_pair(k, v);
          }
          else
          {
            where->pair2 = std::make_pair(k, v);
          }
          where->type = true;
          size_++;
          return;
        }
      }
    }

    void clear(node* there)
    {
      if (there == nullptr) return;
      if (there->type) clear(there->middle);
      clear(there->left);
      clear(there->right);
      delete there;
    }

    void clear()
    {
      clear(root_);
      root_ = nullptr;
      size_ = 0;
    }

    Value& operator[](const Key& k)
    {
      node* where = find_approximately(k);
      if (!Comparator(where->pair1.first, k) && !Comparator(k, where->pair1.first))
      {
        return where->pair1.second;
      }
      else if (where->type)
      {
        if (!Comparator(where->pair2.first, k) && !Comparator(k, where->pair2.first))
        {
          return where->pair2.second;
        }
      }
      insert(k, Value());
      return operator[](k);
    }

    Value& at(const Key& k)
    {
      node* where = find_approximately(k);
      if (!where) throw std::out_of_range("key not found!");
      if (!Comparator(where->pair1.first, k) && !Comparator(k, where->pair1.first))
      {
        return where->pair1.second;
      }
      else if (where->type)
      {
        if (!Comparator(where->pair2.first, k) && !Comparator(k, where->pair2.first))
        {
          return where->pair2.second;
        }
      }
      throw std::out_of_range("key not found!");
    }

    const Value& at(const Key& k) const
    {
      node* where = find_approximately(k);
      if (!where) throw std::out_of_range("key not found!");
      if (!Comparator(where->pair1.first, k) && !Comparator(k, where->pair1.first))
      {
        return where->pair1.second;
      }
      else if (where->type)
      {
        if (!Comparator(where->pair2.first, k) && !Comparator(k, where->pair2.first))
        {
          return where->pair2.second;
        }
      }
      throw std::out_of_range("key not found!");
    }

    void swap(map& other) noexcept
    {
      std::swap(root_, other.root_);
      std::swap(size_, other.size_);
    }

    size_t size() const noexcept
    {
      return size_;
    }

    bool empty() const noexcept
    {
      return size_ == 0;
    }

    class iterator
    {
    public:
      iterator():
        current_node(nullptr),
        current_pair(0)
      {
      }

      iterator(node* n, int pair = 0):
        current_node(n),
        current_pair(pair)
      {
      }

      std::pair< const Key, Value >& operator*()
      {
        if (current_pair == 0)
          return reinterpret_cast< std::pair< const Key, Value >& >(current_node->pair1);
        else
          return reinterpret_cast< std::pair< const Key, Value >& >(current_node->pair2);
      }

      std::pair< const Key, Value >* operator->()
      {
        return &(operator*());
      }

      iterator& operator++()
      {
        if (!current_node) return *this;

        if (current_node->type && current_pair == 0)
        {
          current_pair = 1;
          return *this;
        }

        // Find next node
        std::stack< node* > nodes;
        node* p = current_node;
        while (p->parent)
        {
          if (p->parent->left == p)
          {
            nodes.push(p->parent);
            if (p->parent->type)
              nodes.push(p->parent->middle);
            p = p->parent;
            break;
          }
          else if (p->parent->middle == p && p->parent->type)
          {
            nodes.push(p->parent);
            p = p->parent;
            break;
          }
          p = p->parent;
        }

        while (!nodes.empty())
        {
          p = nodes.top();
          nodes.pop();
          if (p->left)
          {
            current_node = p;
            current_pair = 0;
            return *this;
          }
        }

        // If we get here, we're at the end
        current_node = nullptr;
        current_pair = 0;
        return *this;
      }

      iterator operator++(int)
      {
        iterator temp = *this;
        ++(*this);
        return temp;
      }

      bool operator==(const iterator& other) const
      {
        return current_node == other.current_node && current_pair == other.current_pair;
      }

      bool operator!=(const iterator& other) const
      {
        return !(*this == other);
      }

    private:
      node* current_node;
      int current_pair; // 0 for pair1, 1 for pair2
      friend class map;
    };

    iterator begin()
    {
      if (!root_) return end();

      node* n = root_;
      while (n->left)
        n = n->left;
      return iterator(n, 0);
    }

    iterator end()
    {
      return iterator(nullptr);
    }

    class const_iterator
    {
    public:
      const_iterator():
        current_node(nullptr),
        current_pair(0)
      {
      }

      const_iterator(node* n, int pair = 0):
        current_node(n),
        current_pair(pair)
      {
      }

      const_iterator(iterator it):
        current_node(it.current_node),
        current_pair(it.current_pair)
      {
      }

      const std::pair< const Key, Value >& operator*() const
      {
        if (current_pair == 0)
          return reinterpret_cast< const std::pair< const Key, Value >& >(current_node->pair1);
        else
          return reinterpret_cast< const std::pair< const Key, Value >& >(current_node->pair2);
      }

      const std::pair< const Key, Value >* operator->() const
      {
        return &(operator*());
      }

      const_iterator& operator++()
      {
        if (!current_node) return *this;

        if (current_node->type && current_pair == 0)
        {
          current_pair = 1;
          return *this;
        }

        // Find next node
        std::stack< node* > nodes;
        node* p = current_node;
        while (p->parent)
        {
          if (p->parent->left == p)
          {
            nodes.push(p->parent);
            if (p->parent->type)
              nodes.push(p->parent->middle);
            p = p->parent;
            break;
          }
          else if (p->parent->middle == p && p->parent->type)
          {
            nodes.push(p->parent);
            p = p->parent;
            break;
          }
          p = p->parent;
        }

        while (!nodes.empty())
        {
          p = nodes.top();
          nodes.pop();
          if (p->left)
          {
            current_node = p;
            current_pair = 0;
            return *this;
          }
        }

        // If we get here, we're at the end
        current_node = nullptr;
        current_pair = 0;
        return *this;
      }

      const_iterator operator++(int)
      {
        const_iterator temp = *this;
        ++(*this);
        return temp;
      }

      bool operator==(const const_iterator& other) const
      {
        return current_node == other.current_node && current_pair == other.current_pair;
      }

      bool operator!=(const const_iterator& other) const
      {
        return !(*this == other);
      }

    private:
      const node* current_node;
      int current_pair; // 0 for pair1, 1 for pair2
      friend class map;
    };

    const_iterator cbegin() const
    {
      if (!root_) return cend();

      node* n = root_;
      while (n->left)
        n = n->left;
      return const_iterator(n, 0);
    }

    const_iterator cend() const
    {
      return const_iterator(nullptr);
    }

    const_iterator begin() const
    {
      return cbegin();
    }

    const_iterator end() const
    {
      return cend();
    }

    iterator find(const Key& k)
    {
      node* where = find_approximately(k);
      if (!where) return end();
      if (!Comparator(where->pair1.first, k) && !Comparator(k, where->pair1.first))
      {
        return iterator(where, 0);
      }
      else if (where->type)
      {
        if (!Comparator(where->pair2.first, k) && !Comparator(k, where->pair2.first))
        {
          return iterator(where, 1);
        }
      }
      return end();
    }

    const_iterator find(const Key& k) const
    {
      node* where = find_approximately(k);
      if (!where) return cend();
      if (!Comparator(where->pair1.first, k) && !Comparator(k, where->pair1.first))
      {
        return const_iterator(where, 0);
      }
      else if (where->type)
      {
        if (!Comparator(where->pair2.first, k) && !Comparator(k, where->pair2.first))
        {
          return const_iterator(where, 1);
        }
      }
      return cend();
    }

    ~map()
    {
      clear();
    }

  private:
    node* root_;
    size_t size_;
  };
}

#endif
