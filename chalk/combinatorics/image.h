#ifndef CHALK_COMBINATORICS_IMAGE_H
#define CHALK_COMBINATORICS_IMAGE_H

#include <concepts>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace chalk {
struct Image;

template <typename T>
concept Visualizable = requires(T t) {
  { ChalkVisualize(t) } -> std::same_as<Image>;
};

struct Image {
  struct Dimensions {
    uint16_t height;
    uint16_t width;
  };

  explicit Image(Visualizable auto const& v) : Image(ChalkVisualize(v)) {}

  explicit Image(std::vector<std::string> lines)
      : lines_(std::move(lines)),
        max_line_length_(
            lines_.empty()
                ? 0
                : std::max_element(lines_.begin(), lines_.end())->size()) {}

  Dimensions dimensions() const {
    return {
        .height = static_cast<uint16_t>(lines_.size()),
        .width  = static_cast<uint16_t>(max_line_length_),
    };
  }

  static Image Horizontally(Visualizable auto const& lhs,
                            Visualizable auto const& rhs, size_t padding = 0) {
    return Horizontally(ChalkVisualize(lhs), ChalkVisualize(rhs), padding);
  }
  static Image Horizontally(Visualizable auto const& lhs, Image const& rhs,
                            size_t padding = 0) {
    return Horizontally(ChalkVisualize(lhs), rhs, padding);
  }
  static Image Horizontally(Image const& lhs, Visualizable auto const& rhs,
                            size_t padding = 0) {
    return Horizontally(lhs, ChalkVisualize(rhs), padding);
  }
  static Image Horizontally(Image const& lhs, Image const& rhs,
                            size_t padding = 0);

  static Image Vertically(Visualizable auto const& lhs,
                          Visualizable auto const& rhs, size_t padding = 0) {
    return Vertically(ChalkVisualize(lhs), ChalkVisualize(rhs), padding);
  }
  static Image Vertically(Visualizable auto const& lhs, Image const& rhs,
                          size_t padding = 0) {
    return Vertically(ChalkVisualize(lhs), rhs, padding);
  }
  static Image Vertically(Image lhs, Visualizable auto const& rhs,
                          size_t padding = 0) {
    return Vertically(std::move(lhs), ChalkVisualize(rhs), padding);
  }
  static Image Vertically(Image lhs, Image const& rhs, size_t padding = 0);

  friend std::ostream& operator<<(std::ostream& os, Image const& image) {
    for (auto const& line : image.lines_) { os << line << "\n"; }
    return os;
  }

 private:
  Image() = default;

  std::vector<std::string> lines_;
  size_t max_line_length_;
};

}  // namespace chalk

#endif  // CHALK_COMBINATORICS_IMAGE_H
