#include "chalk/combinatorics/dyck_path.h"

#include <iostream>

#include "absl/functional/function_ref.h"

namespace chalk {
namespace {

void BounceImpl(DyckPath const& path,
                absl::FunctionRef<void(size_t)> handle_part_size) {
  size_t height        = 0;
  size_t bounce_height = 0;
  bool bouncing_up     = true;

  size_t last_bounce_height = 0;
  for (auto iter = path.begin(); iter != path.end(); ++iter) {
    switch (*iter) {
      case DyckPath::Step::Up: ++height; break;
      case DyckPath::Step::Down: --height; break;
    }
    if (bouncing_up) {
      if (bounce_height > height) {
        last_bounce_height = bounce_height;
        bouncing_up        = false;
        --bounce_height;
      } else {
        ++bounce_height;
      }
    } else {
      if (bounce_height == 0) {
        bouncing_up = true;
        ++bounce_height;
        handle_part_size(last_bounce_height);
      } else {
        --bounce_height;
      }
    }
  }
  handle_part_size(last_bounce_height);
}

}  // namespace

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
  size_t result     = 0;
  size_t total_left = path.size() / 2;
  BounceImpl(path, [&](size_t part) {
    total_left -= part;
    result += total_left;
  });
  return result;
}

Composition BouncePath(DyckPath const& path) {
  Composition result;
  if (path.empty()) { return result; }
  BounceImpl(path, [&](size_t part) { result.append(part); });
  return result;
}

DyckPath DyckPath::ConcatenateImpl(DyckPath lhs, DyckPath const& rhs) {
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

DyckPath DyckPath::Peak(size_t height) {
  DyckPath p;
  p.implementation_.reserve(height * 2);
  p.implementation_.resize(height, static_cast<bool>(Step::Up));
  p.implementation_.resize(height * 2, static_cast<bool>(Step::Down));
  return p;
}

DyckPath DyckPath::Minimal(size_t peaks) {
  DyckPath p;
  p.implementation_.reserve(peaks * 2);
  for (size_t i = 0; i < peaks; ++i) {
    p.implementation_.push_back(static_cast<bool>(Step::Up));
    p.implementation_.push_back(static_cast<bool>(Step::Down));
  }
  return p;
}

std::vector<DyckPath> DyckPath::All(size_t n) {
  if (n == 0) { return {DyckPath{}}; }
  if (n == 1) { return {DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}}; }

  std::vector<DyckPath> results = All(n - 1);
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
