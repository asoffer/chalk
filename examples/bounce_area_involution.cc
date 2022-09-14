#include <iostream>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/functional/function_ref.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "combinatorics/dyck_path.h"

size_t ValidateInputs(absl::Span<char const * const> input) {
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

std::vector<std::string> PathAsImage(chalk::DyckPath const & path) {
  std::vector<std::string> result{""};
  size_t height = 0;
  size_t counter = 0;
  for (chalk::DyckPath::Step step : path) {
    ++counter;
    bool is_up = (step == chalk::DyckPath::Step::Up);
    size_t write_height = height - (is_up ? 0 : 1);

    for (size_t i = 0; i < write_height; ++i) {
      result[i].push_back(((i + height) % 2 == 0) ? '/' : '\\');
    }
    result[write_height].push_back(is_up ? '/' : '\\');
    for (size_t i = write_height + 1; i < result.size(); ++i) {
      result[i].push_back(' ');
    }
    height += (is_up ? 1 : -1);
    if (height == result.size()) { result.emplace_back(counter, ' '); }
  }
  std::reverse(result.begin(), result.end());
  return result;
}

std::vector<std::string> MergeImages(std::vector<std::string> const& lhs,
    std::vector<std::string> const& rhs,
    size_t separator = 4) {
  std::vector<std::string> result;

  auto l = lhs.rbegin();
  auto r = rhs.rbegin();
  for (; l != lhs.rend() and r != rhs.rend(); ++l, ++r) {
    result.push_back(*l + std::string(separator, ' ') + *r);
  }

  for (; l != lhs.rend(); ++l) { result.push_back(*l); }

  for (; r != rhs.rend(); ++r) {
    result.push_back(std::string(separator + lhs.begin()->size(), ' ') + *r);
  }

  std::reverse(result.begin(), result.end());
  return result;
}

struct Result {
  absl::flat_hash_map<chalk::DyckPath, chalk::DyckPath> mapped;
  absl::flat_hash_map<chalk::DyckPath, chalk::DyckPath> inferred;
  absl::flat_hash_map<std::pair<size_t, size_t>,
                      absl::flat_hash_set<chalk::DyckPath>>
      unclassified;
  size_t inferred_self_dual = 0;
  size_t mapped_self_dual = 0;
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
  for (auto & [stats, paths] : paths_by_stats) {
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

std::optional<chalk::DyckPath> Conjecture(chalk::DyckPath const& path) {
  return std::nullopt;
}

int main(int argc, char const *argv[]) {
  size_t n = ValidateInputs(absl::MakeConstSpan(argv, argc));

  auto [mapped, inferred, unclassified, inferred_self_dual, mapped_self_dual] =
      Map(n, Conjecture);

  size_t unclassified_count = 0;
  for (auto const& [stats, paths] : unclassified) {
    unclassified_count += paths.size();
  }
  absl::Format(&std::cerr, R"(
  Mapped:       %u
  Inferred:     %u
  Unclassified: %u
  )",
               2 * mapped.size() - mapped_self_dual,
               2 * inferred.size() - inferred_self_dual, unclassified_count);
  for (auto const& [p1, p2] : inferred) {
    for (auto const& line : MergeImages(PathAsImage(p1), PathAsImage(p2))) {
      std::cerr << line << "\n";
    }
  }

  return 0;
}
