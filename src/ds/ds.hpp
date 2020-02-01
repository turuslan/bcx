#ifndef BCX_DS_DS_HPP
#define BCX_DS_DS_HPP

#include <string_view>
#include <unordered_set>
#include <vector>

#include "types.hpp"

namespace std {
  template <typename T, size_t N>
  struct hash<array<T, N>> {
    auto operator()(const array<T, N> &x) const {
      return hash<string_view>{}({reinterpret_cast<const char *>(x.data()), x.size() * sizeof(T)});
    }
  };
}  // namespace std

namespace bcx::ds {
  class Len {
   public:
    size_t size() const;
    size_t size_bytes() const;
    size_t size(size_t i) const;
    size_t offset(size_t i) const;
    size_t index(size_t offset) const;
    void push_back(size_t n);
    void truncate(size_t n);

   private:
    std::vector<size_t> offset_{0};
  };

  struct Strings {
    using value_type = std::string_view;

    size_t size() const;
    size_t size_bytes() const;
    std::string_view operator[](size_t i) const;
    void push_back(const std::string_view &str);
    void truncate(size_t n);

    std::vector<Byte> bytes;
    Len len;
  };

  template <bool copy, typename Vector>
  struct Indirect {
    using T = typename Vector::value_type;

    Indirect(const Vector &vector) : vector{vector} {}

    std::conditional_t<copy, T, const T &> get(size_t index) const {
      if (index == key_index) {
        if constexpr (copy) {
          return *key;
        } else {
          return key->get();
        }
      }
      return vector[index];
    }

    struct HashEq {
      using Set = std::unordered_set<size_t, HashEq, HashEq>;

      HashEq(const Indirect &indirect) : indirect{indirect} {}

      auto set() {
        return Set{0, *this, *this};
      }

      auto operator()(size_t index) const {
        return std::hash<T>{}(indirect.get(index));
      }

      auto operator()(size_t lhs, size_t rhs) const {
        return indirect.get(lhs) == indirect.get(rhs);
      }

      const Indirect &indirect;
    };

    auto find(const typename HashEq::Set &set, const T &value) {
      if constexpr (copy) {
        key = value;
      } else {
        key = std::cref(value);
      }
      auto it = set.find(key_index);
      std::optional<size_t> result;
      if (it != set.end()) {
        result = *it;
      }
      key.reset();
      return result;
    }

    struct Hashed {
      Vector vector;
      Indirect indirect{vector};
      HashEq hash_eq{indirect};
      typename HashEq::Set set{hash_eq.set()};

      auto size() const {
        return vector.size();
      }

      decltype(auto) operator[](size_t index) const { 
        return vector[index];
      }

      auto find(const T &value) {
        return indirect.find(set, value);
      }

      void push_back(const T &value) {
        auto index = vector.size();
        vector.push_back(value);
        set.insert(index);
      }
    };

    static constexpr size_t key_index{SIZE_T_MAX};
    const Vector &vector;
    std::optional<std::conditional_t<copy, T, std::reference_wrapper<const T>>> key;
  };

  template <typename T>
  struct Linked {
    static constexpr size_t null = SIZE_T_MAX;

    auto add(const T &value, size_t next = null) {
      size_t i = nodes.size();
      nodes.push_back({value, next});
      return i;
    }

    struct Iterator {
      auto &operator++() {
        head = linked.nodes[head].second;
        return *this;
      }

      auto operator!=(const Iterator &other) const {
        return head != other.head;
      }

      decltype(auto) operator*() const{
        return linked.nodes[head].first;
      }

      Linked &linked;
      size_t head;
    };

    struct Range {
      Iterator begin() const {
        return iterator;
      }

      Iterator end() const {
        return {iterator.linked, null};
      }

      const Iterator iterator;
    };

    auto range(size_t head) {
      return Range{{*this, head}};
    }

    struct Vector {
      void add(size_t index, const T &value) {
        if (index >= heads.size()) {
          heads.resize(index + 1, null);
        }
        heads[index] = linked.add(value, heads[index]);
      }

      auto range(size_t index) {
        return linked.range(index < heads.size() ? heads[index] : null);
      }

      std::vector<size_t> heads;
      Linked<T> linked;
    };

    std::vector<std::pair<T, size_t>> nodes;
  };
}  // namespace bcx::ds

#endif  // BCX_DS_DS_HPP
