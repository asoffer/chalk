#include "chalk/combinatorics/image.h"

#include <algorithm>

namespace chalk {

Image Image::Horizontally(Image const& lhs, Image const& rhs, size_t padding) {
  Image image;
  auto l_iter = lhs.lines_.begin();
  auto r_iter = rhs.lines_.begin();
  if (lhs.lines_.size() > rhs.lines_.size()) {
    auto end = l_iter + (lhs.lines_.size() - rhs.lines_.size());
    for (; l_iter != end; ++l_iter) { image.lines_.push_back(*l_iter); }
  } else {
    auto end = r_iter + (rhs.lines_.size() - lhs.lines_.size());
    for (; r_iter != end; ++r_iter) {
      image.lines_.emplace_back(lhs.max_line_length_ + padding, ' ') += *r_iter;
    }
  }
  for (; l_iter != lhs.lines_.end(); ++l_iter, ++r_iter) {
    auto& line = image.lines_.emplace_back(*l_iter);
    line.resize(lhs.max_line_length_ + padding, ' ');
    line += *r_iter;
  }
  image.max_line_length_ = lhs.max_line_length_ + rhs.max_line_length_;
  return image;
}

Image Image::Vertically(Image lhs, Image const& rhs, size_t padding) {
  lhs.max_line_length_ = std::max(lhs.max_line_length_, rhs.max_line_length_);
  lhs.lines_.resize(lhs.lines_.size() + padding);
  lhs.lines_.insert(lhs.lines_.end(), rhs.lines_.begin(), rhs.lines_.end());
  return lhs;
}

}  // namespace chalk
