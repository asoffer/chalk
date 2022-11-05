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

size_t ValidateInputs(absl::Span<char const* const> input) {
  if (input.size() != 2) {
    std::cerr << "Must have exactly one input.\n";
    std::abort();
  }

  size_t n = std::atoi(input[1]);
  if (n == 0) {
    std::cerr << "Input must be a positive integer.\n";
    std::abort();
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

    // Skip the first `partition_[0]` up steps and the first required down-step.
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

int main(int argc, char const* argv[]) {
  size_t n = ValidateInputs(absl::MakeConstSpan(argv, argc));

  auto [mapped, inferred, unclassified, inferred_self_dual, mapped_self_dual] =
      Map(n, Conjecture);

  size_t unclassified_count = 0;
  for (auto const& [stats, paths] : unclassified) {
    unclassified_count += paths.size();
  }

  std::cerr << "============================================\n\n";

  for (auto const& [p1, p2] : inferred) {
    std::cerr << chalk::Image::Horizontally(p1, p2, 4) << "\n";
  }

  absl::Format(&std::cerr, R"(
  Mapped:       %u
  Inferred:     %u
  Unclassified: %u
  )",
               2 * mapped.size() - mapped_self_dual,
               2 * inferred.size() - inferred_self_dual, unclassified_count);
  return 0;
}
