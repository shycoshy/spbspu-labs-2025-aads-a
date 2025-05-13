#ifndef MAP_HPP
#define MAP_HPP
#include <memory>
#include <utility>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <iterator>

namespace asafov
{
  template< typename Key, typename T, typename Compare = std::less< Key >,
            typename Allocator = std::allocator< std::pair< const Key, T > > >
  class map
  {
  public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair< const Key, T >;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using allocator_type = Allocator;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits< Allocator >::pointer;
    using const_pointer = typename std::allocator_traits< Allocator >::const_pointer;

    class iterator;
    class const_iterator;
    using reverse_iterator = std::reverse_iterator< iterator >;
    using const_reverse_iterator = std::reverse_iterator< const_iterator >;

  private:
    struct NodeBase
    {
      NodeBase* parent;
      NodeBase* left;
      NodeBase* middle;
      NodeBase* right;

      NodeBase() : parent(nullptr), left(nullptr), middle(nullptr), right(nullptr)
      {
      }
    };

    struct Node: NodeBase
    {
      value_type value1;
      value_type value2; // Используется только для 3-узлов
      bool is_three_node;

      Node(const value_type& val)
        : value1(val), is_three_node(false)
      {
      }

      Node(const value_type& val1, const value_type& val2)
        : value1(val1), value2(val2), is_three_node(true)
      {
      }
    };

    using NodeAllocator = typename std::allocator_traits< Allocator >::template rebind_alloc< Node >;
    using NodeAllocatorTraits = std::allocator_traits< NodeAllocator >;

    NodeBase header;
    size_type node_count;
    Compare comp;
    NodeAllocator alloc;

  public:
    // Конструкторы
    map() : node_count(0), comp(Compare()), alloc(NodeAllocator())
    {
      init_header();
    }

    explicit map(const Compare& comp, const Allocator& alloc = Allocator())
      : node_count(0), comp(comp), alloc(alloc)
    {
      init_header();
    }

    explicit map(const Allocator& alloc)
      : node_count(0), comp(Compare()), alloc(alloc)
    {
      init_header();
    }

    template< class InputIt >
    map(InputIt first, InputIt last,
                const Compare& comp = Compare(),
                const Allocator& alloc = Allocator())
      : node_count(0), comp(comp), alloc(alloc)
    {
      init_header();
      insert(first, last);
    }

    map(std::initializer_list< value_type > init,
                const Compare& comp = Compare(),
                const Allocator& alloc = Allocator())
      : map(init.begin(), init.end(), comp, alloc)
    {
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
        comp = other.comp;
        insert(other.begin(), other.end());
      }
      return *this;
    }

    map& operator=(std::initializer_list< value_type > ilist)
    {
      clear();
      insert(ilist.begin(), ilist.end());
      return *this;
    }

    allocator_type get_allocator() const
    {
      return alloc;
    }

    // Итераторы
    iterator begin() noexcept
    {
      return iterator(leftmost());
    }

    const_iterator begin() const noexcept
    {
      return const_iterator(leftmost());
    }

    const_iterator cbegin() const noexcept
    {
      return const_iterator(leftmost());
    }

    iterator end() noexcept
    {
      return iterator(&header);
    }

    const_iterator end() const noexcept
    {
      return const_iterator(const_cast< NodeBase* >(&header));
    }

    const_iterator cend() const noexcept
    {
      return const_iterator(const_cast< NodeBase* >(&header));
    }

    reverse_iterator rbegin() noexcept
    {
      return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept
    {
      return const_reverse_iterator(cend());
    }

    reverse_iterator rend() noexcept
    {
      return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept
    {
      return const_reverse_iterator(cbegin());
    }

    // Доступ к элементам
    T& at(const Key& key)
    {
      iterator it = find(key);
      if (it == end())
      {
        throw std::out_of_range("map::at");
      }
      return it->second;
    }

    const T& at(const Key& key) const
    {
      const_iterator it = find(key);
      if (it == end())
      {
        throw std::out_of_range("map::at");
      }
      return it->second;
    }

    T& operator[](const Key& key)
    {
      return try_emplace(key).first->second;
    }

    T& operator[](Key&& key)
    {
      return try_emplace(std::move(key)).first->second;
    }

    // Поиск
    iterator find(const Key& key)
    {
      NodeBase* node = root();
      while (node)
      {
        Node* current = static_cast< Node* >(node);

        if (!comp(key, current->value1.first) && !comp(current->value1.first, key))
        {
          return iterator(current, 0);
        }

        if (current->is_three_node &&
          !comp(key, current->value2.first) && !comp(current->value2.first, key))
        {
          return iterator(current, 1);
        }

        if (comp(key, current->value1.first))
        {
          node = current->left;
        }
        else if (!current->is_three_node || comp(key, current->value2.first))
        {
          node = current->middle;
        }
        else
        {
          node = current->right;
        }
      }
      return end();
    }

    const_iterator find(const Key& key) const
    {
      const NodeBase* node = root();
      while (node)
      {
        const Node* current = static_cast< const Node* >(node);

        if (!comp(key, current->value1.first) && !comp(current->value1.first, key))
        {
          return const_iterator(current, 0);
        }

        if (current->is_three_node &&
          !comp(key, current->value2.first) && !comp(current->value2.first, key))
        {
          return const_iterator(current, 1);
        }

        if (comp(key, current->value1.first))
        {
          node = current->left;
        }
        else if (!current->is_three_node || comp(key, current->value2.first))
        {
          node = current->middle;
        }
        else
        {
          node = current->right;
        }
      }
      return cend();
    }

    size_type count(const Key& key) const
    {
      return find(key) != end() ? 1 : 0;
    }

    iterator lower_bound(const Key& key)
    {
      NodeBase* node = root();
      NodeBase* result = &header;

      while (node)
      {
        Node* current = static_cast< Node* >(node);

        if (!comp(current->value1.first, key))
        {
          result = node;
          node = current->left;
        }
        else if (current->is_three_node && !comp(current->value2.first, key))
        {
          result = node;
          node = current->middle;
        }
        else
        {
          node = current->is_three_node ? current->right : current->middle;
        }
      }

      return iterator(static_cast< Node* >(result));
    }

    const_iterator lower_bound(const Key& key) const
    {
      const NodeBase* node = root();
      const NodeBase* result = &header;

      while (node)
      {
        const Node* current = static_cast< const Node* >(node);

        if (!comp(current->value1.first, key))
        {
          result = node;
          node = current->left;
        }
        else if (current->is_three_node && !comp(current->value2.first, key))
        {
          result = node;
          node = current->middle;
        }
        else
        {
          node = current->is_three_node ? current->right : current->middle;
        }
      }

      return const_iterator(static_cast< const Node* >(result));
    }

    iterator upper_bound(const Key& key)
    {
      iterator it = lower_bound(key);
      if (it != end() && !comp(key, it->first))
      {
        ++it;
      }
      return it;
    }

    const_iterator upper_bound(const Key& key) const
    {
      const_iterator it = lower_bound(key);
      if (it != cend() && !comp(key, it->first))
      {
        ++it;
      }
      return it;
    }

    std::pair< iterator, iterator > equal_range(const Key& key)
    {
      return {lower_bound(key), upper_bound(key)};
    }

    std::pair< const_iterator, const_iterator > equal_range(const Key& key) const
    {
      return {lower_bound(key), upper_bound(key)};
    }

    // Модификаторы
    void clear() noexcept
    {
      erase_nodes(root());
      init_header();
      node_count = 0;
    }

    std::pair< iterator, bool > insert(const value_type& value)
    {
      return emplace(value);
    }

    std::pair< iterator, bool > insert(value_type&& value)
    {
      return emplace(std::move(value));
    }

    iterator insert(const_iterator hint, const value_type& value)
    {
      return emplace_hint(hint, value);
    }

    iterator insert(const_iterator hint, value_type&& value)
    {
      return emplace_hint(hint, std::move(value));
    }

    template< class InputIt >
    void insert(InputIt first, InputIt last)
    {
      for (; first != last; ++first)
      {
        insert(*first);
      }
    }

    void insert(std::initializer_list< value_type > ilist)
    {
      insert(ilist.begin(), ilist.end());
    }

    template< class... Args >
    std::pair< iterator, bool > emplace(Args&&... args)
    {
      value_type* new_value = create_value(std::forward< Args >(args)...);
      return insert_value(new_value);
    }

    template< class... Args >
    iterator emplace_hint(const_iterator hint, Args&&... args)
    {
      (void)hint; // Подсказка не используется в этой реализации
      return emplace(std::forward< Args >(args)...).first;
    }

    template< class M >
    std::pair< iterator, bool > insert_or_assign(const Key& key, M&& obj)
    {
      auto result = try_emplace(key, std::forward< M >(obj));
      if (!result.second)
      {
        result.first->second = std::forward< M >(obj);
      }
      return result;
    }

    template< class M >
    std::pair< iterator, bool > insert_or_assign(Key&& key, M&& obj)
    {
      auto result = try_emplace(std::move(key), std::forward< M >(obj));
      if (!result.second)
      {
        result.first->second = std::forward< M >(obj);
      }
      return result;
    }

    template< class... Args >
    std::pair< iterator, bool > try_emplace(const Key& key, Args&&... args)
    {
      iterator it = find(key);
      if (it != end())
      {
        return {it, false};
      }
      return emplace(std::piecewise_construct,
                     std::forward_as_tuple(key),
                     std::forward_as_tuple(std::forward< Args >(args)...));
    }

    template< class... Args >
    std::pair< iterator, bool > try_emplace(Key&& key, Args&&... args)
    {
      iterator it = find(key);
      if (it != end())
      {
        return {it, false};
      }
      return emplace(std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(std::forward< Args >(args)...));
    }

    iterator erase(const_iterator pos)
    {
      iterator next_it(pos.node(), pos.index());
      ++next_it;
      erase_node(pos.node(), pos.index());
      return next_it;
    }

    iterator erase(iterator pos)
    {
      iterator next_it(pos.node(), pos.index());
      ++next_it;
      erase_node(pos.node(), pos.index());
      return next_it;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
      while (first != last)
      {
        first = erase(first);
      }
      return iterator(first.node(), first.index());
    }

    size_type erase(const Key& key)
    {
      iterator it = find(key);
      if (it == end())
      {
        return 0;
      }
      erase(it);
      return 1;
    }

    void swap(map& other) noexcept
    {
      std::swap(header, other.header);
      std::swap(node_count, other.node_count);
      std::swap(comp, other.comp);
      std::swap(alloc, other.alloc);
    }

    // Наблюдатели
    size_type size() const noexcept
    {
      return node_count;
    }

    bool empty() const noexcept
    {
      return node_count == 0;
    }

    size_type max_size() const noexcept
    {
      return NodeAllocatorTraits::max_size(alloc);
    }

    key_compare key_comp() const
    {
      return comp;
    }

    template< typename K1, typename T1, typename C1, typename A1 >
    friend bool operator==(const map< K1, T1, C1, A1 >& lhs,
                           const map< K1, T1, C1, A1 >& rhs);

    template< typename K1, typename T1, typename C1, typename A1 >
    friend bool operator<(const map< K1, T1, C1, A1 >& lhs,
                          const map< K1, T1, C1, A1 >& rhs);

  private:
    // Внутренние методы
    void init_header()
    {
      header.parent = nullptr;
      header.left = &header;
      header.right = &header;
    }

    NodeBase* root() const
    {
      return header.parent;
    }

    void root(NodeBase* node)
    {
      header.parent = node;
      if (node)
      {
        node->parent = &header;
      }
    }

    NodeBase* leftmost() const
    {
      return header.left;
    }

    void leftmost(NodeBase* node)
    {
      header.left = node;
    }

    NodeBase* rightmost() const
    {
      return header.right;
    }

    void rightmost(NodeBase* node)
    {
      header.right = node;
    }

    template< typename... Args >
    value_type* create_value(Args&&... args)
    {
      value_type* val = std::allocator_traits< Allocator >::allocate(alloc, 1);
      try
      {
        std::allocator_traits< Allocator >::construct(alloc, val, std::forward< Args >(args)...);
        return val;
      }
      catch (...)
      {
        std::allocator_traits< Allocator >::deallocate(alloc, val, 1);
        throw;
      }
    }

    void destroy_value(value_type* val) noexcept
    {
      std::allocator_traits< Allocator >::destroy(alloc, val);
      std::allocator_traits< Allocator >::deallocate(alloc, val, 1);
    }

    Node* create_node(const value_type& val)
    {
      Node* node = NodeAllocatorTraits::allocate(alloc, 1);
      try
      {
        NodeAllocatorTraits::construct(alloc, node, val);
        return node;
      }
      catch (...)
      {
        NodeAllocatorTraits::deallocate(alloc, node, 1);
        throw;
      }
    }

    Node* create_node(value_type&& val)
    {
      Node* node = NodeAllocatorTraits::allocate(alloc, 1);
      try
      {
        NodeAllocatorTraits::construct(alloc, node, std::move(val));
        return node;
      }
      catch (...)
      {
        NodeAllocatorTraits::deallocate(alloc, node, 1);
        throw;
      }
    }

    Node* create_node(const value_type& val1, const value_type& val2)
    {
      Node* node = NodeAllocatorTraits::allocate(alloc, 1);
      try
      {
        NodeAllocatorTraits::construct(alloc, node, val1, val2);
        return node;
      }
      catch (...)
      {
        NodeAllocatorTraits::deallocate(alloc, node, 1);
        throw;
      }
    }

    void destroy_node(Node* node) noexcept
    {
      if (node)
      {
        NodeAllocatorTraits::destroy(alloc, node);
        NodeAllocatorTraits::deallocate(alloc, node, 1);
      }
    }

    std::pair< iterator, bool > insert_value(value_type* new_value)
    {
      // Реализация вставки в 2-3 дерево
      // ...
      // Возвращаем итератор на вставленный элемент и флаг успеха
      return {iterator(nullptr), false};
    }

    void erase_node(Node* node, int index)
    {
      // Реализация удаления из 2-3 дерева
      // ...
    }

    void erase_nodes(NodeBase* node)
    {
      if (!node) return;

      erase_nodes(node->left);
      erase_nodes(node->middle);
      if (static_cast< Node* >(node)->is_three_node)
      {
        erase_nodes(node->right);
      }

      destroy_node(static_cast< Node* >(node));
    }
  };

  // Итератор
  template< typename Key, typename T, typename Compare, typename Allocator >
  class map< Key, T, Compare, Allocator >::iterator
  {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = map::value_type;
    using difference_type = map::difference_type;
    using pointer = map::pointer;
    using reference = map::reference;

  private:
    Node* current;
    int index; // 0 для value1, 1 для value2

    friend class map;
    friend class const_iterator;

  public:
    iterator() : current(nullptr), index(0)
    {
    }

    explicit iterator(Node* node, int idx = 0) : current(node), index(idx)
    {
    }

    reference operator*() const
    {
      return index == 0 ? current->value1 : current->value2;
    }

    pointer operator->() const
    {
      return index == 0 ? &current->value1 : &current->value2;
    }

    iterator& operator++()
    {
      // Логика перехода к следующему элементу
      // ...
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    iterator& operator--()
    {
      // Логика перехода к предыдущему элементу
      // ...
      return *this;
    }

    iterator operator--(int)
    {
      iterator tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator==(const iterator& other) const
    {
      return current == other.current &&
        (current == nullptr || index == other.index);
    }

    bool operator!=(const iterator& other) const
    {
      return !(*this == other);
    }
  };

  // Константный итератор
  template< typename Key, typename T, typename Compare, typename Allocator >
  class map< Key, T, Compare, Allocator >::const_iterator
  {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = map::value_type;
    using difference_type = map::difference_type;
    using pointer = map::const_pointer;
    using reference = map::const_reference;

  private:
    const Node* current;
    int index; // 0 для value1, 1 для value2

    friend class map;

  public:
    const_iterator() : current(nullptr), index(0)
    {
    }

    explicit const_iterator(const Node* node, int idx = 0) : current(node), index(idx)
    {
    }

    const_iterator(const iterator& it) : current(it.current), index(it.index)
    {
    }

    reference operator*() const
    {
      return index == 0 ? current->value1 : current->value2;
    }

    pointer operator->() const
    {
      return index == 0 ? &current->value1 : &current->value2;
    }

    const_iterator& operator++()
    {
      // Логика перехода к следующему элементу
      // ...
      return *this;
    }

    const_iterator operator++(int)
    {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    const_iterator& operator--()
    {
      // Логика перехода к предыдущему элементу
      // ...
      return *this;
    }

    const_iterator operator--(int)
    {
      const_iterator tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator==(const const_iterator& other) const
    {
      return current == other.current &&
        (current == nullptr || index == other.index);
    }

    bool operator!=(const const_iterator& other) const
    {
      return !(*this == other);
    }
  };

  // Операторы сравнения
  template< typename Key, typename T, typename Compare, typename Allocator >
  bool operator==(const map< Key, T, Compare, Allocator >& lhs,
                  const map< Key, T, Compare, Allocator >& rhs)
  {
    return lhs.size() == rhs.size() &&
      std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  template< typename Key, typename T, typename Compare, typename Allocator >
  bool operator!=(const map< Key, T, Compare, Allocator >& lhs,
                  const map< Key, T, Compare, Allocator >& rhs)
  {
    return !(lhs == rhs);
  }

  template< typename Key, typename T, typename Compare, typename Allocator >
  bool operator<(const map< Key, T, Compare, Allocator >& lhs,
                 const map< Key, T, Compare, Allocator >& rhs)
  {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end(),
                                        lhs.key_comp());
  }

  template< typename Key, typename T, typename Compare, typename Allocator >
  bool operator<=(const map< Key, T, Compare, Allocator >& lhs,
                  const map< Key, T, Compare, Allocator >& rhs)
  {
    return !(rhs < lhs);
  }

  template< typename Key, typename T, typename Compare, typename Allocator >
  bool operator>(const map< Key, T, Compare, Allocator >& lhs,
                 const map< Key, T, Compare, Allocator >& rhs)
  {
    return rhs < lhs;
  }

  template< typename Key, typename T, typename Compare, typename Allocator >
  bool operator>=(const map< Key, T, Compare, Allocator >& lhs,
                  const map< Key, T, Compare, Allocator >& rhs)
  {
    return !(lhs < rhs);
  }

  // Специализация swap
  template< typename Key, typename T, typename Compare, typename Allocator >
  void swap(map< Key, T, Compare, Allocator >& lhs,
            map< Key, T, Compare, Allocator >& rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }
}
#endif
