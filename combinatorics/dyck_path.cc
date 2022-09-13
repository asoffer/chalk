#include <iostream>
#include "combinatorics/dyck_path.h"

namespace chalk {

size_t Area(DyckPath const& path) {
  size_t result  = 0;
  size_t counter = 0;
  for (DyckPath::Step step : path) {
    switch (step) {
      case DyckPath::Step::Up: result += ++counter; break;
      case DyckPath::Step::Down: --counter; break;
    }
  }

  return result - path.size() / 2;
}

size_t Bounce(DyckPath const& path) {
  size_t result = 0;

  size_t height = 0;
  size_t bounce_height = 0;
  bool bouncing_up = true;
  for (auto iter = path.begin(); iter != path.end(); ++iter) {
    DyckPath::Step step = *iter;
    switch (step) {
      case DyckPath::Step::Up: ++height; break;
      case DyckPath::Step::Down: --height; break;
    }
    if (bouncing_up) {
      if (bounce_height > height) {
        bouncing_up = false;
        --bounce_height;
      } else {
        ++bounce_height;
      }
    } else {
      if (bounce_height == 0) {
        bouncing_up = true;
        ++bounce_height;
        result += (path.end() - iter) / 2;
      } else {
        --bounce_height;
      }
    }
  }

  return result;
}

DyckPath DyckPath::Concatenate(DyckPath lhs, DyckPath const& rhs) {
  lhs.implementation_.insert(lhs.implementation_.end(),
                             rhs.implementation_.begin(),
                             rhs.implementation_.end());
  return lhs;
}

DyckPath DyckPath::Lift(DyckPath const& path) {
  auto result = path;
  result.implementation_.insert(result.implementation_.begin(),
                                static_cast<bool>(Step::Up));
  result.implementation_.push_back(static_cast<bool>(Step::Down));
  return result;
}

std::vector<DyckPath> DyckPath::All(size_t n) {
  if (n == 0) { return {DyckPath{}}; }
  if (n == 1) { return {DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}}; }

  std::vector<DyckPath> results = All(n -1);
  for (auto& path : results) { path = Lift(path); }

  for (size_t i = 0; i + 1 < n; ++i) {
    for (auto const& lhs : DyckPath::All(i)) {
      for (auto const& rhs : DyckPath::All(n - i - 1)) {
        results.push_back(DyckPath::Concatenate(DyckPath::Lift(lhs), rhs));
      }
    }
  }
  return results;
}

}  // namespace chalk
