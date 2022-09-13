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

}  // namespace chalk
