#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <stdexcept>
#include <initializer_list>
#include <queue>
#include <vector>

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

    void split(node* n, const Key& key, const Value& value)
    {
      // Determine which key is the middle one
      Key keys[3] = {n->pair1.first, n->pair2.first, key};
      Value values[3] = {n->pair1.second, n->pair2.second, value};

      // Sort keys and values
      for (int i = 0; i < 2; i++)
      {
        for (int j = i + 1; j < 3; j++)
        {
          if (keys[i] > keys[j])
          {
            std::swap(keys[i], keys[j]);
            std::swap(values[i], values[j]);
          }
        }
      }

      Key middle_key = keys[1];
      Value middle_value = values[1];

      if (!n->parent)
      {
        // Split root
        node* new_root = new node(middle_key, middle_value);
        node* left_child = new node(keys[0], values[0], new_root);
        node* right_child = new node(keys[2], values[2], new_root);

        new_root->left = left_child;
        new_root->right = right_child;

        // Reassign children
        if (!n->is_leaf())
        {
          left_child->left = n->left;
          left_child->right = n->middle;
          right_child->left = n->right;

          if (left_child->left) left_child->left->parent = left_child;
          if (left_child->right) left_child->right->parent = left_child;
          if (right_child->left) right_child->left->parent = right_child;
        }

        delete n;
        root = new_root;
      }
      else
      {
        node* parent = n->parent;

        if (!parent->is_three_node)
        {
          // Parent is 2-node
          if (parent->left == n)
          {
            parent->pair2 = parent->pair1;
            parent->pair1 = {middle_key, middle_value};
            parent->is_three_node = true;

            node* new_left = new node(keys[0], values[0], parent);
            node* new_middle = new node(keys[2], values[2], parent);

            new_left->left = n->left;
            new_left->right = n->middle;
            new_middle->left = n->right;

            parent->left = new_left;
            parent->middle = new_middle;

            if (new_left->left) new_left->left->parent = new_left;
            if (new_left->right) new_left->right->parent = new_left;
            if (new_middle->left) new_middle->left->parent = new_middle;
          }
          else // parent->right == n
          {
            parent->pair2 = {middle_key, middle_value};
            parent->is_three_node = true;

            node* new_middle = new node(keys[0], values[0], parent);
            node* new_right = new node(keys[2], values[2], parent);

            new_middle->left = n->left;
            new_middle->right = n->middle;
            new_right->left = n->right;

            parent->middle = new_middle;
            parent->right = new_right;

            if (new_middle->left) new_middle->left->parent = new_middle;
            if (new_middle->right) new_middle->right->parent = new_middle;
            if (new_right->left) new_right->left->parent = new_right;
          }
          delete n;
        }
        else
        {
          // Parent is 3-node - need to propagate split
          node* temp = new node(middle_key, middle_value);
          temp->is_three_node = true;
          temp->pair2 = {keys[2], values[2]};

          node* left = new node(keys[0], values[0], temp);
          node* right = new node(keys[2], values[2], temp);

          temp->left = left;
          temp->right = right;

          if (!n->is_leaf())
          {
            left->left = n->left;
            left->right = n->middle;
            right->left = n->right;

            if (left->left) left->left->parent = left;
            if (left->right) left->right->parent = left;
            if (right->left) right->left->parent = right;
          }

          delete n;
          split(parent, middle_key, middle_value);
          delete temp;
        }
      }
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
      iterator(node* root = nullptr, bool end = false) : current(nullptr), use_second(false)
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

      std::pair< const Key, Value >& operator*() const
      {
        return use_second
                 ? reinterpret_cast< std::pair< const Key, Value >& >(current->pair2)
                 : reinterpret_cast< std::pair< const Key, Value >& >(current->pair1);
      }

      std::pair< const Key, Value >* operator->() const
      {
        return &operator*();
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

      bool operator==(const iterator& other) const
      {
        return current == other.current && use_second == other.use_second;
      }
    };

    class const_iterator
    {
    private:
      std::queue< const node* > nodes;
      const node* current;
      bool use_second;

      void traverse(const node* n)
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
      const_iterator(const node* root = nullptr, bool end = false) : current(nullptr), use_second(false)
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

      const std::pair< const Key, Value >& operator*() const
      {
        return use_second
                 ? reinterpret_cast< const std::pair< const Key, Value >& >(current->pair2)
                 : reinterpret_cast< const std::pair< const Key, Value >& >(current->pair1);
      }

      const std::pair< const Key, Value >* operator->() const
      {
        return &operator*();
      }

      const_iterator& operator++()
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

      bool operator!=(const const_iterator& other) const
      {
        return current != other.current || use_second != other.use_second;
      }

      bool operator==(const const_iterator& other) const
      {
        return current == other.current && use_second == other.use_second;
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

    map(const map& other) : map()
    {
      for (const auto& p: other)
        insert(p.first, p.second);
    }

    map(map&& other) noexcept : root(other.root), size_(other.size_)
    {
      other.root = nullptr;
      other.size_ = 0;
    }

    ~map()
    {
      clear(root);
    }

    map& operator=(const map& other)
    {
      if (this != &other)
      {
        clear(root);
        root = nullptr;
        size_ = 0;
        for (const auto& p: other)
          insert(p.first, p.second);
      }
      return *this;
    }

    map& operator=(map&& other) noexcept
    {
      if (this != &other)
      {
        clear(root);
        root = other.root;
        size_ = other.size_;
        other.root = nullptr;
        other.size_ = 0;
      }
      return *this;
    }

    iterator begin()
    {
      return iterator(root);
    }

    iterator end()
    {
      return iterator(root, true);
    }

    const_iterator begin() const
    {
      return const_iterator(root);
    }

    const_iterator end() const
    {
      return const_iterator(root, true);
    }

    const_iterator cbegin() const
    {
      return const_iterator(root);
    }

    const_iterator cend() const
    {
      return const_iterator(root, true);
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
        if (key == current->pair1.first ||
          (current->is_three_node && key == current->pair2.first))
        {
          // Key already exists - update value
          if (key == current->pair1.first)
            current->pair1.second = value;
          else
            current->pair2.second = value;
          return;
        }

        if (key < current->pair1.first)
          current = current->left;
        else if (!current->is_three_node || key < current->pair2.first)
          current = current->middle;
        else
          current = current->right;
      }

      // Check if key already exists in leaf
      if (key == current->pair1.first ||
        (current->is_three_node && key == current->pair2.first))
      {
        if (key == current->pair1.first)
          current->pair1.second = value;
        else
          current->pair2.second = value;
        return;
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
        // Need to split node
        split(current, key, value);
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

    const_iterator find(const Key& key) const
    {
      const node* n = find_node(key);
      if (!n) return cend();

      const_iterator it = cbegin();
      const_iterator e = cend();
      while (it != e)
      {
        if ((*it).first == key)
          return it;
        ++it;
      }
      return cend();
    }

    Value& operator[](const Key& key)
    {
      iterator it = find(key);
      if (it != end())
        return it->second;

      insert(key, Value());
      return find(key)->second;
    }

    Value& at(const Key& key)
    {
      iterator it = find(key);
      if (it == end())
        throw std::out_of_range("Key not found");
      return it->second;
    }

    const Value& at(const Key& key) const
    {
      const_iterator it = find(key);
      if (it == cend())
        throw std::out_of_range("Key not found");
      return it->second;
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

    void swap(map& other) noexcept
    {
      std::swap(root, other.root);
      std::swap(size_, other.size_);
    }
  };
}

#endif
