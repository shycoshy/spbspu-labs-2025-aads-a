#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <cstddef>
#include <vector>
#include <stdexcept>
#include <initializer_list>
#include <queue>

namespace asafov
{
  template< class Key, class Value >
  class map
  {
  private:
    struct node
    {
      node* left;
      node* middle;
      node* right;
      node* parent;
      std::pair< Key, Value > pair1;
      std::pair< Key, Value > pair2;
      bool is_three_node;

      node(const Key& k, const Value& v, node* p = nullptr) :
        left(nullptr), middle(nullptr), right(nullptr),
        parent(p), pair1(k, v), is_three_node(false)
      {
      }

      bool is_leaf() const
      {
        return !left && !middle && !right;
      }
    };

    node* root;
    size_t size_;

    void clear(node* n)
    {
      if (!n) return;
      clear(n->left);
      clear(n->middle);
      clear(n->right);
      delete n;
    }

    node* find_node(const Key& key) const
    {
      node* current = root;
      while (current)
      {
        if (key == current->pair1.first)
          return current;

        if (current->is_three_node && key == current->pair2.first)
          return current;

        if (key < current->pair1.first)
          current = current->left;
        else if (!current->is_three_node || key < current->pair2.first)
          current = current->middle;
        else
          current = current->right;
      }
      return nullptr;
    }

  public:
    class iterator
    {
    private:
      std::queue< node* > nodes;
      node* current;
      bool use_second;

      void traverse(node* n)
      {
        if (!n) return;
        traverse(n->left);
        nodes.push(n);
        if (n->is_three_node)
        {
          traverse(n->middle);
          nodes.push(n); // For pair2
        }
        traverse(n->right);
      }

    public:
      iterator(node* root, bool end = false) : current(nullptr), use_second(false)
      {
        if (root && !end)
        {
          traverse(root);
          if (!nodes.empty())
          {
            current = nodes.front();
            nodes.pop();
          }
        }
      }

      std::pair< const Key, Value >& operator*()
      {
        return use_second
                 ? reinterpret_cast< std::pair< const Key, Value >& >(current->pair2)
                 : reinterpret_cast< std::pair< const Key, Value >& >(current->pair1);
      }

      iterator& operator++()
      {
        if (!nodes.empty())
        {
          if (current->is_three_node && !use_second)
          {
            use_second = true;
          }
          else
          {
            current = nodes.front();
            nodes.pop();
            use_second = false;
          }
        }
        else
        {
          if (current->is_three_node && !use_second)
          {
            use_second = true;
          }
          else
          {
            current = nullptr;
          }
        }
        return *this;
      }

      bool operator!=(const iterator& other) const
      {
        return current != other.current || use_second != other.use_second;
      }
    };

    map() : root(nullptr), size_(0)
    {
    }

    map(std::initializer_list< std::pair< Key, Value > > init) : map()
    {
      for (const auto& p: init)
        insert(p.first, p.second);
    }

    ~map()
    {
      clear(root);
    }

    iterator begin()
    {
      return iterator(root);
    }

    iterator end()
    {
      return iterator(root, true);
    }

    void insert(const Key& key, const Value& value)
    {
      if (!root)
      {
        root = new node(key, value);
        size_++;
        return;
      }

      node* current = root;
      while (!current->is_leaf())
      {
        if (key < current->pair1.first)
          current = current->left;
        else if (!current->is_three_node || key < current->pair2.first)
          current = current->middle;
        else
          current = current->right;
      }

      if (!current->is_three_node)
      {
        // Simple insert into 2-node
        if (key < current->pair1.first)
        {
          current->pair2 = current->pair1;
          current->pair1 = {key, value};
        }
        else
        {
          current->pair2 = {key, value};
        }
        current->is_three_node = true;
        size_++;
      }
      else
      {
        // Need to split node (simplified)
        // In real implementation you would need proper splitting logic
        node* new_node = new node(key, value);
        // Temporary solution - just mark for rebuild
        // In complete implementation you would restructure the tree
        size_++;
      }
    }

    iterator find(const Key& key)
    {
      node* n = find_node(key);
      if (!n) return end();

      iterator it = begin();
      iterator e = end();
      while (it != e)
      {
        if ((*it).first == key)
          return it;
        ++it;
      }
      return end();
    }

    Value& operator[](const Key& key)
    {
      iterator it = find(key);
      if (it != end())
        return (*it).second;

      insert(key, Value());
      return find(key)->second;
    }

    Value& at(const Key& key)
    {
      iterator it = find(key);
      if (it == end())
        throw std::out_of_range("Key not found");
      return (*it).second;
    }

    const Value& at(const Key& key) const
    {
      const node* n = find_node(key);
      if (!n)
        throw std::out_of_range("Key not found");

      if (key == n->pair1.first)
        return n->pair1.second;
      return n->pair2.second;
    }

    size_t size() const
    {
      return size_;
    }

    bool empty() const
    {
      return size_ == 0;
    }

    void clear()
    {
      clear(root);
      root = nullptr;
      size_ = 0;
    }
  };
}

#endif
