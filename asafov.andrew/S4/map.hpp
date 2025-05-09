#ifndef MAP_H
#define MAP_H

#include <stdexcept>
#include <utility>

namespace asafov
{
  template< typename Key, typename Value >
  class map
  {
  private:
    struct node
    {
      Key keys[2];
      Value values[2];
      node* children[3];
      node* parent;
      int key_count;
      bool is_leaf;

      node(const Key& key, const Value& value)
        : parent(nullptr), key_count(1), is_leaf(true)
      {
        keys[0] = key;
        values[0] = value;
        children[0] = children[1] = children[2] = nullptr;
      }

      node() : parent(nullptr), key_count(0), is_leaf(true)
      {
        children[0] = children[1] = children[2] = nullptr;
      }

      bool is_full() const
      {
        return key_count == 2;
      }

      bool is_2node() const
      {
        return key_count == 1;
      }

      bool is_3node() const
      {
        return key_count == 2;
      }

      void insert_key_value(const Key& key, const Value& value)
      {
        if (key_count == 0 || key < keys[0])
        {
          keys[1] = keys[0];
          values[1] = values[0];
          keys[0] = key;
          values[0] = value;
        }
        else
        {
          keys[1] = key;
          values[1] = value;
        }
        key_count++;
      }

      void remove_key(int index)
      {
        if (index == 0)
        {
          keys[0] = keys[1];
          values[0] = values[1];
        }
        key_count--;
      }

      int find_key_index(const Key& key) const
      {
        if (key == keys[0]) return 0;
        if (key_count == 2 && key == keys[1]) return 1;
        return -1;
      }
    };

    node* root;
    size_t tree_size;

    // Вспомогательные методы
    node* find_node(const Key& key) const
    {
      node* current = root;
      while (current)
      {
        if (key == current->keys[0])
        {
          return current;
        }
        else if (current->key_count == 2 && key == current->keys[1])
        {
          return current;
        }

        if (current->key_count == 1)
        {
          if (key < current->keys[0])
          {
            current = current->children[0];
          }
          else
          {
            current = current->children[1];
          }
        }
        else
        {
          if (key < current->keys[0])
          {
            current = current->children[0];
          }
          else if (key < current->keys[1])
          {
            current = current->children[1];
          }
          else
          {
            current = current->children[2];
          }
        }
      }
      return nullptr;
    }

    void split(node* n)
    {
      if (!n->is_full()) return;

      node* left = new node(n->keys[0], n->values[0]);
      left->children[0] = n->children[0];
      left->children[1] = n->children[1];
      if (!n->is_leaf)
      {
        left->is_leaf = false;
        if (n->children[0]) n->children[0]->parent = left;
        if (n->children[1]) n->children[1]->parent = left;
      }

      node* right = new node(n->keys[1], n->values[1]);
      right->children[0] = n->children[1];
      right->children[1] = n->children[2];
      if (!n->is_leaf)
      {
        right->is_leaf = false;
        if (n->children[1]) n->children[1]->parent = right;
        if (n->children[2]) n->children[2]->parent = right;
      }

      if (!n->parent)
      {
        root = new node(n->keys[1], n->values[1]);
        root->is_leaf = false;
        root->children[0] = left;
        root->children[1] = right;
        left->parent = root;
        right->parent = root;
      }
      else
      {
        node* parent = n->parent;
        parent->insert_key_value(n->keys[1], n->values[1]);

        if (parent->children[0] == n)
        {
          parent->children[2] = parent->children[1];
          parent->children[0] = left;
          parent->children[1] = right;
        }
        else if (parent->children[1] == n)
        {
          parent->children[2] = right;
          parent->children[1] = left;
        }

        left->parent = parent;
        right->parent = parent;

        if (parent->is_full())
        {
          split(parent);
        }
      }
    }

    void clear_helper(node* n)
    {
      if (!n) return;
      if (!n->is_leaf)
      {
        for (int i = 0; i <= n->key_count; ++i)
        {
          clear_helper(n->children[i]);
        }
      }
      delete n;
    }

    node* get_min_node(node* n) const
    {
      while (n && !n->is_leaf)
      {
        n = n->children[0];
      }
      return n;
    }

    void fix_underflow(node* n)
    {
      if (n->key_count >= 1) return; // Нет недополнения

      if (n == root)
      {
        if (n->is_leaf)
        {
          delete root;
          root = nullptr;
        }
        else
        {
          root = n->children[0];
          root->parent = nullptr;
          delete n;
        }
        return;
      }

      node* parent = n->parent;
      int child_pos = -1;
      for (int i = 0; i <= parent->key_count; ++i)
      {
        if (parent->children[i] == n)
        {
          child_pos = i;
          break;
        }
      }

      // Попробуем занять у левого брата
      if (child_pos > 0)
      {
        node* left_sibling = parent->children[child_pos - 1];
        if (left_sibling->key_count > 1)
        {
          // Переносим ключ из родителя в n
          n->insert_key_value(parent->keys[child_pos - 1], parent->values[child_pos - 1]);

          // Переносим ключ из левого брата в родителя
          parent->keys[child_pos - 1] = left_sibling->keys[left_sibling->key_count - 1];
          parent->values[child_pos - 1] = left_sibling->values[left_sibling->key_count - 1];

          // Переносим ребенка, если есть
          if (!left_sibling->is_leaf)
          {
            n->children[n->key_count] = left_sibling->children[left_sibling->key_count];
            n->children[n->key_count]->parent = n;
            left_sibling->children[left_sibling->key_count] = nullptr;
          }

          left_sibling->remove_key(left_sibling->key_count - 1);
          return;
        }
      }

      // Попробуем занять у правого брата
      if (child_pos < parent->key_count)
      {
        node* right_sibling = parent->children[child_pos + 1];
        if (right_sibling->key_count > 1)
        {
          // Переносим ключ из родителя в n
          n->insert_key_value(parent->keys[child_pos], parent->values[child_pos]);

          // Переносим ключ из правого брата в родителя
          parent->keys[child_pos] = right_sibling->keys[0];
          parent->values[child_pos] = right_sibling->values[0];

          // Переносим ребенка, если есть
          if (!right_sibling->is_leaf)
          {
            for (int i = 0; i <= right_sibling->key_count; ++i)
            {
              right_sibling->children[i] = right_sibling->children[i + 1];
            }
            n->children[n->key_count] = right_sibling->children[0];
            n->children[n->key_count]->parent = n;
            right_sibling->children[0] = nullptr;
          }

          right_sibling->remove_key(0);
          return;
        }
      }

      // Слияние с братом
      if (child_pos > 0)
      {
        // Слияние с левым братом
        node* left_sibling = parent->children[child_pos - 1];

        // Переносим ключ из родителя в левого брата
        left_sibling->insert_key_value(parent->keys[child_pos - 1], parent->values[child_pos - 1]);

        // Переносим детей, если есть
        if (!n->is_leaf)
        {
          left_sibling->children[left_sibling->key_count] = n->children[0];
          if (left_sibling->children[left_sibling->key_count])
          {
            left_sibling->children[left_sibling->key_count]->parent = left_sibling;
          }
        }

        // Удаляем ключ из родителя
        for (int i = child_pos - 1; i < parent->key_count - 1; ++i)
        {
          parent->keys[i] = parent->keys[i + 1];
          parent->values[i] = parent->values[i + 1];
          parent->children[i + 1] = parent->children[i + 2];
        }
        parent->key_count--;

        delete n;

        if (parent->key_count == 0)
        {
          fix_underflow(parent);
        }
      }
      else
      {
        // Слияние с правым братом
        node* right_sibling = parent->children[child_pos + 1];

        // Переносим ключ из родителя в n
        n->insert_key_value(parent->keys[child_pos], parent->values[child_pos]);

        // Переносим ключ из правого брата в n
        n->insert_key_value(right_sibling->keys[0], right_sibling->values[0]);

        // Переносим детей, если есть
        if (!right_sibling->is_leaf)
        {
          n->children[1] = right_sibling->children[0];
          n->children[2] = right_sibling->children[1];
          if (n->children[1]) n->children[1]->parent = n;
          if (n->children[2]) n->children[2]->parent = n;
        }

        // Удаляем ключ из родителя
        for (int i = child_pos; i < parent->key_count - 1; ++i)
        {
          parent->keys[i] = parent->keys[i + 1];
          parent->values[i] = parent->values[i + 1];
          parent->children[i + 1] = parent->children[i + 2];
        }
        parent->key_count--;

        delete right_sibling;

        if (parent->key_count == 0)
        {
          fix_underflow(parent);
        }
      }
    }

  public:
    // Итераторы
    class iterator
    {
    private:
      node* current;
      int key_index;

    public:
      iterator(node* n = nullptr, int index = 0) : current(n), key_index(index)
      {
      }

      std::pair< const Key&, Value& > operator*()
      {
        return {current->keys[key_index], current->values[key_index]};
      }

      iterator& operator++()
      {
        if (current->is_leaf)
        {
          if (key_index < current->key_count - 1)
          {
            key_index++;
          }
          else
          {
            node* parent = current->parent;
            while (parent)
            {
              if (parent->children[0] == current)
              {
                key_index = 0;
                current = parent;
                break;
              }
              else if (parent->children[1] == current)
              {
                if (parent->key_count > 1)
                {
                  key_index = 1;
                  current = parent;
                  break;
                }
                else
                {
                  current = parent;
                  parent = current->parent;
                }
              }
              else
              {
                current = parent;
                parent = current->parent;
              }
            }
            if (!parent)
            {
              current = nullptr;
              key_index = 0;
            }
          }
        }
        else
        {
          node* child = current->children[key_index + 1];
          while (!child->is_leaf)
          {
            child = child->children[0];
          }
          current = child;
          key_index = 0;
        }
        return *this;
      }

      bool operator==(const iterator& other) const
      {
        return current == other.current && key_index == other.key_index;
      }

      bool operator!=(const iterator& other) const
      {
        return !(*this == other);
      }
    };

    class const_iterator
    {
    private:
      node* current;
      int key_index;

    public:
      const_iterator(node* n = nullptr, int index = 0) : current(n), key_index(index)
      {
      }

      const_iterator(const iterator& it) : current(it.current), key_index(it.key_index)
      {
      }

      const std::pair< const Key&, const Value& > operator*() const
      {
        return {current->keys[key_index], current->values[key_index]};
      }

      const_iterator& operator++()
      {
        if (current->is_leaf)
        {
          if (key_index < current->key_count - 1)
          {
            key_index++;
          }
          else
          {
            node* parent = current->parent;
            while (parent)
            {
              if (parent->children[0] == current)
              {
                key_index = 0;
                current = parent;
                break;
              }
              else if (parent->children[1] == current)
              {
                if (parent->key_count > 1)
                {
                  key_index = 1;
                  current = parent;
                  break;
                }
                else
                {
                  current = parent;
                  parent = current->parent;
                }
              }
              else
              {
                current = parent;
                parent = current->parent;
              }
            }
            if (!parent)
            {
              current = nullptr;
              key_index = 0;
            }
          }
        }
        else
        {
          node* child = current->children[key_index + 1];
          while (!child->is_leaf)
          {
            child = child->children[0];
          }
          current = child;
          key_index = 0;
        }
        return *this;
      }

      bool operator==(const const_iterator& other) const
      {
        return current == other.current && key_index == other.key_index;
      }

      bool operator!=(const const_iterator& other) const
      {
        return !(*this == other);
      }
    };

    // Конструкторы
    map() : root(nullptr), tree_size(0)
    {
    }

    ~map()
    {
      clear();
    }

    // Итераторы
    iterator begin()
    {
      if (!root) return end();
      node* n = root;
      while (!n->is_leaf)
      {
        n = n->children[0];
      }
      return iterator(n, 0);
    }

    const_iterator begin() const
    {
      if (!root) return end();
      node* n = root;
      while (!n->is_leaf)
      {
        n = n->children[0];
      }
      return const_iterator(n, 0);
    }

    iterator end()
    {
      return iterator(nullptr, 0);
    }

    const_iterator end() const
    {
      return const_iterator(nullptr, 0);
    }

    // Методы
    void insert(const Key& key, const Value& value)
    {
      if (!root)
      {
        root = new node(key, value);
        tree_size++;
        return;
      }

      node* current = root;
      while (!current->is_leaf)
      {
        if (current->key_count == 1)
        {
          if (key < current->keys[0])
          {
            current = current->children[0];
          }
          else
          {
            current = current->children[1];
          }
        }
        else
        {
          if (key < current->keys[0])
          {
            current = current->children[0];
          }
          else if (key < current->keys[1])
          {
            current = current->children[1];
          }
          else
          {
            current = current->children[2];
          }
        }
      }

      if (current->key_count == 1)
      {
        if (key == current->keys[0])
        {
          current->values[0] = value;
          return;
        }
      }
      else
      {
        if (key == current->keys[0] || key == current->keys[1])
        {
          if (key == current->keys[0])
          {
            current->values[0] = value;
          }
          else
          {
            current->values[1] = value;
          }
          return;
        }
      }

      current->insert_key_value(key, value);
      tree_size++;

      if (current->is_full())
      {
        split(current);
      }
    }

    void erase(const Key& key)
    {
      node* n = find_node(key);
      if (!n) return;

      int key_index = n->find_key_index(key);
      if (key_index == -1) return;

      if (!n->is_leaf)
      {
        // Находим следующий ключ (преемника)
        node* successor = n->children[key_index + 1];
        while (!successor->is_leaf)
        {
          successor = successor->children[0];
        }

        // Заменяем удаляемый ключ преемником
        n->keys[key_index] = successor->keys[0];
        n->values[key_index] = successor->values[0];

        // Удаляем преемника из листа
        n = successor;
        key_index = 0;
      }

      // Удаляем ключ из листа
      n->remove_key(key_index);
      tree_size--;

      // Исправляем недополнение, если необходимо
      if (n->key_count == 0)
      {
        fix_underflow(n);
      }
    }

    size_t size() const
    {
      return tree_size;
    }

    bool empty() const
    {
      return tree_size == 0;
    }

    void clear()
    {
      clear_helper(root);
      root = nullptr;
      tree_size = 0;
    }

    void swap(map& other)
    {
      node* temp_root = root;
      size_t temp_size = tree_size;
      root = other.root;
      tree_size = other.tree_size;
      other.root = temp_root;
      other.tree_size = temp_size;
    }

    iterator find(const Key& key)
    {
      node* n = find_node(key);
      if (!n) return end();
      if (n->keys[0] == key)
      {
        return iterator(n, 0);
      }
      else if (n->key_count > 1 && n->keys[1] == key)
      {
        return iterator(n, 1);
      }
      return end();
    }

    const_iterator find(const Key& key) const
    {
      node* n = find_node(key);
      if (!n) return end();
      if (n->keys[0] == key)
      {
        return const_iterator(n, 0);
      }
      else if (n->key_count > 1 && n->keys[1] == key)
      {
        return const_iterator(n, 1);
      }
      return end();
    }

    Value& at(const Key& key)
    {
      node* n = find_node(key);
      if (!n) throw std::out_of_range("Key not found");
      if (n->keys[0] == key)
      {
        return n->values[0];
      }
      else if (n->key_count > 1 && n->keys[1] == key)
      {
        return n->values[1];
      }
      throw std::out_of_range("Key not found");
    }

    const Value& at(const Key& key) const
    {
      node* n = find_node(key);
      if (!n) throw std::out_of_range("Key not found");
      if (n->keys[0] == key)
      {
        return n->values[0];
      }
      else if (n->key_count > 1 && n->keys[1] == key)
      {
        return n->values[1];
      }
      throw std::out_of_range("Key not found");
    }

    Value& operator[](const Key& key)
    {
      node* n = find_node(key);
      if (n)
      {
        if (n->keys[0] == key)
        {
          return n->values[0];
        }
        else if (n->key_count > 1 && n->keys[1] == key)
        {
          return n->values[1];
        }
      }

      Value value = Value();
      insert(key, value);
      return find_node(key)->values[0];
    }
  };
}
#endif
