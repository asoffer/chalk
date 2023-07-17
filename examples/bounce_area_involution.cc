#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <iterator>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/functional/function_ref.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "chalk/combinatorics/dyck_path.h"
#include "chalk/combinatorics/image.h"
#include "chalk/combinatorics/partition.h"
#include "chalk/combinatorics/sequence.h"

[[noreturn]] void Abort(std::string_view message) {
  std::cerr << message << "\n";
  std::abort();
}

size_t ValidateInteger(char const* cstr) {
  size_t n = std::atoi(cstr);
  if (n == 0) {
    Abort(absl::StrCat("Input must be a positive integer, but you provided ",
                       cstr, "\n"));
  }
  return n;
}

struct Result {
  absl::flat_hash_map<chalk::DyckPath, chalk::DyckPath> mapped;
  absl::flat_hash_map<chalk::DyckPath, chalk::DyckPath> inferred;
  absl::flat_hash_map<std::pair<size_t, size_t>,
                      absl::flat_hash_set<chalk::DyckPath>>
      unclassified;
  size_t inferred_self_dual = 0;
  size_t mapped_self_dual   = 0;
};
Result Map(
    size_t n,
    absl::FunctionRef<std::optional<chalk::DyckPath>(chalk::DyckPath const&)>
        partial_mapping) {
  Result result;

  absl::flat_hash_map<std::pair<size_t, size_t>,
                      absl::flat_hash_set<chalk::DyckPath>>
      paths_by_stats;
  absl::flat_hash_set<chalk::DyckPath> seen;
  for (auto& path : chalk::DyckPath::All(n)) {
    if (seen.contains(path)) { continue; }

    auto maybe_mapped = partial_mapping(path);

    size_t a = chalk::Area(path);
    size_t b = chalk::Bounce(path);
    if (not maybe_mapped) {
      paths_by_stats[std::pair(a, b)].insert(std::move(path));
      continue;
    }

    if (a == b) { ++result.mapped_self_dual; }
    seen.insert(*maybe_mapped);

    if (a < b) {
      result.mapped.emplace(std::move(path), *std::move(maybe_mapped));
    } else {
      result.mapped.emplace(*std::move(maybe_mapped), std::move(path));
    }
  }
  for (auto& [stats, paths] : paths_by_stats) {
    auto [area, bounce] = stats;
    if (area > bounce) { continue; }

    if (paths.size() == 1) {
      if (area == bounce) {
        ++result.inferred_self_dual;
        result.inferred.emplace(*paths.begin(), *paths.begin());
      } else {
        result.inferred.emplace(
            std::move(*paths.begin()),
            std::move(*paths_by_stats[std::pair(bounce, area)].begin()));
      }
    } else {
      result.unclassified[std::pair(area, bounce)] = std::move(paths);
      if (area != bounce) {
        result.unclassified[std::pair(bounce, area)] =
            std::move(paths_by_stats[std::pair(bounce, area)]);
      }
    }
  }
  return result;
}

chalk::DyckPath DyckPathFromPartition(chalk::Partition const& partition) {
  chalk::DyckPath path;
  for (auto part : partition) { path += chalk::DyckPath::Peak(part); }
  return path;
}

struct TwoPartPath {
  explicit TwoPartPath(const chalk::DyckPath& path, chalk::Partition partition)
      : partition_(std::move(partition)) {
    assert(partition_.parts() == 2);

    // Skip the first `partition_[0]` up steps and the first required
    // down-step.
    auto iter = path.begin() + partition_[0] + 1;

    // Ignore the last `partition_[1]` down-steps.
    auto end_iter = path.end() - partition_[1];

    steps_.assign(iter, end_iter);
  }

  // Returns the sequence of gaps between up steps in the starting part of the
  // two-part path.
  absl::Span<chalk::DyckPath::Step const> first_steps() const {
    return absl::Span(steps_.data(), partition_[1] - 1);
  }

  // Returns the sequence of gaps between up steps in the ending part of the
  // two-part path.
  std::vector<size_t> remaining_gaps() const {
    return chalk::GapsBetween(absl::Span(steps_.data() + (partition_[1] - 1),
                                         steps_.size() - (partition_[1] - 1)),
                              chalk::DyckPath::Step::Up);
  }

 private:
  chalk::Partition partition_;
  std::vector<chalk::DyckPath::Step> steps_;
};

std::optional<chalk::DyckPath> Conjecture(chalk::DyckPath const& path) {
  chalk::Composition comp         = chalk::BouncePath(path);
  auto [partition, was_partition] = chalk::Partition::From(std::move(comp));
  if (not was_partition) { return std::nullopt; }

  auto partition_path = DyckPathFromPartition(partition);
  if (path == partition_path) {
    return DyckPathFromPartition(partition.conjugate());
  } else if (partition.parts() == 2) {
    TwoPartPath two_part_path(path, partition);

    auto first_steps = two_part_path.first_steps();
    std::vector<size_t> start_gaps =
        chalk::GapsBetween(first_steps, chalk::DyckPath::Step::Up);

    std::vector<size_t> end_gaps = two_part_path.remaining_gaps();
    // Ignore the last gap (corresponding to the remaining down-steps that are
    // forced), and read them back to front.
    end_gaps.pop_back();
    std::reverse(end_gaps.begin(), end_gaps.end());

    if (start_gaps.size() <= 2) {
      chalk::DyckPath result;

      std::vector<chalk::DyckPath> pieces(
          end_gaps.size(),
          chalk::DyckPath::Lifted(chalk::DyckPath::Minimal(1)));
      if (start_gaps.size() == 2) {
        pieces[start_gaps[0]] =
            chalk::DyckPath::Lifted(chalk::DyckPath::Minimal(2));
      }

      auto iter = pieces.begin();
      for (size_t gap : end_gaps) {
        result += *iter;
        result += chalk::DyckPath::Minimal(gap);
        ++iter;
      }

      // Pad the beginning with "/\.../\" until it is the appropriate size.
      size_t padding_peaks = (path.size() - result.size()) / 2;
      result               = chalk::DyckPath::Minimal(padding_peaks) + result;

      return result;

    } else if (start_gaps.size() == 3) {
      if (partition[1] != 3) { return std::nullopt; }

      chalk::DyckPath result;
      if (partition[0] % 2 == 0) {
        result = chalk::DyckPath::Lifted(chalk::DyckPath::Minimal(3)) +
                 chalk::DyckPath::Minimal(partition[0] / 2 - 1);
      } else {
        result = chalk::DyckPath::Peak(3) +
                 chalk::DyckPath::Minimal((partition[0] - 1) / 2);
      }
      for (size_t i = 0; i < partition[0] - 1 - end_gaps[0]; ++i) {
        result.topple();
      }

      // Pad the beginning with "/\.../\" until it is the appropriate size.
      size_t padding_peaks = (path.size() - result.size()) / 2;
      result               = chalk::DyckPath::Minimal(padding_peaks) + result;
      return result;
    } else {
      absl::Format(&std::cerr, "Should be able to process: { ");
      for (auto n : end_gaps) { std::cerr << n << " "; }
      std::cerr << "} (gaps)\n\n";

      std::cerr << chalk::Image(path)
                << "\n--------------------------------------------\n\n";
    }
  }
  return std::nullopt;
}

auto Classify(size_t n) {
  absl::flat_hash_map<std::pair<size_t, size_t>, std::vector<chalk::DyckPath>>
      map;
  for (auto& path : chalk::DyckPath::All(n)) {
    map[{chalk::Area(path), chalk::Bounce(path)}].emplace_back(path);
  }
  return map;
}

void ShowChain(size_t n, size_t bounce_area_sum) {
  std::cerr
      << "\n\n\033[1;31m======================================================="
         "==="
         "==================================\nShowing all paths of length "
      << n << " with bounce and area summing to " << bounce_area_sum
      << "\n=========================================================="
         "==================================\033[1;0m\n\n";
  auto map = Classify(n);
  for (size_t b = 0; b <= bounce_area_sum; ++b) {
    size_t a = bounce_area_sum - b;
    std::cerr << "\033[1;33mPaths with area " << a << " and bounce " << b
              << ":";
    auto iter = map.find({a, b});
    if (iter == map.end()) {
      std::cerr << " (none)\n\033[0m";
      continue;
    }
    std::cerr << "\033[0m";
    ::winsize w;
    ::ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    size_t number_in_row = std::max(size_t{1}, (w.ws_col / 2) / (n + 2));
    size_t i             = 0;
    auto& ps             = iter->second;
    for (size_t i = 0; i < ps.size(); i += number_in_row) {
      chalk::Image img(ps[i]);
      for (size_t j = i + 1; j < std::min(i + number_in_row, ps.size()); ++j) {
        img = chalk::Image::Horizontally(ps[j], img, 4);
      }
      std::cerr << "\n\033[1;37m" << img << "\033[0m\n";
    }
  }
}

void ApplyConjecture(size_t n) {
  auto [mapped, inferred, unclassified, inferred_self_dual, mapped_self_dual] =
      Map(n, Conjecture);

  size_t unclassified_count = 0;
  for (auto const& [stats, paths] : unclassified) {
    unclassified_count += paths.size();
  }

  std::cerr << "============================================\nMapped:\n\n";

  for (auto const& [p1, p2] : mapped) {
    std::cerr << chalk::Image::Horizontally(p1, p2, 4) << "\n";
  }

  std::cerr << "Inferred:\n\n";
  for (auto const& [p1, p2] : inferred) {
    std::cerr << chalk::Image::Horizontally(p1, p2, 4) << "\n";
  }

  std::cerr << "Unclassified:\n\n";
  for (auto const& [ab, ps] : unclassified) {
    auto [a, b] = ab;
    std::cerr << "  => " << a << ", " << b << "\n";
    for (auto const& p : ps) { std::cerr << chalk::Image(p) << "\n"; }
  }

  absl::Format(&std::cerr, R"(
  Mapped:       %u
  Inferred:     %u
  Unclassified: %u
  )",
               2 * mapped.size() - mapped_self_dual,
               2 * inferred.size() - inferred_self_dual, unclassified_count);
}

void Dispatch(absl::Span<char const* const> input) {
  constexpr std::string_view InvalidCommand =
      "Must start with one of these commands: 'chains', 'conjecture'";
  if (input.size() < 2) { Abort(InvalidCommand); }

  if (input[1] == std::string_view("chains")) {
    if (input.size() != 4) {
      Abort(
          "'chains' requires two integer arguments representing the length of "
          "the path, and the sum of bounce and area.");
    }
    ShowChain(ValidateInteger(input[2]), ValidateInteger(input[3]));

  } else if (input[1] == std::string_view("conjecture")) {
    ApplyConjecture(ValidateInteger(input[2]));

  } else {
    Abort(InvalidCommand);
  }
}

int main(int argc, char const* argv[]) {
  Dispatch(absl::MakeConstSpan(argv, argc));
  return 0;
}
