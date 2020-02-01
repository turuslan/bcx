#include "ds/ds.hpp"
#include "format/format.hpp"

namespace bcx::ds {
  size_t Len::size() const {
    return offset_.size() - 1;
  }

  size_t Len::size_bytes() const {
    return offset_.back();
  }

  size_t Len::size(size_t i) const {
    return offset_[i + 1] - offset_[i];
  }

  size_t Len::offset(size_t i) const {
    return offset_[i];
  }

  void Len::push_back(size_t n) {
    offset_.push_back(size_bytes() + n);
  }

  void Len::truncate(size_t n) {
    if (n > size()) {
      fatal("Len::truncate invalid argument");
    }
    offset_.resize(n + 1);
  }

  String::String(const Byte *ptr, size_t size) : ptr_{ptr}, size_{size} {
  }

  std::string String::str() const {
    return {b2c(ptr_), size_};
  }

  size_t Strings::size() const {
    return len.size();
  }

  size_t Strings::size_bytes() const {
    return len.size_bytes();
  }

  String Strings::operator[](size_t i) const {
    return {bytes.data() + len.offset(i), len.size(i)};
  }

  void Strings::push_back(const std::string &str) {
    len.push_back(str.size());
    bytes.insert(bytes.end(), c2b(str.data()), c2b(str.data()) + str.size());
  }

  void Strings::truncate(size_t n) {
    len.truncate(n);
    bytes.resize(size_bytes());
  }
}  // namespace bcx
