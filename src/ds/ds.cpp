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
}  // namespace bcx
