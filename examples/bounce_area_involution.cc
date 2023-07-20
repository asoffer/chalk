#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "chalk/combinatorics/composition.h"
#include "chalk/combinatorics/dyck_path.h"
#include "chalk/combinatorics/image.h"
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

struct BouncePathAndArea {
  chalk::Composition bounce_path;
  size_t area;

  friend bool operator==(BouncePathAndArea const&,
                         BouncePathAndArea const&) = default;
  friend bool operator!=(BouncePathAndArea const&,
                         BouncePathAndArea const&) = default;

  std::vector<BouncePathAndArea> Neighbors() const {
    std::vector<BouncePathAndArea> result;
    for (auto iter = bounce_path.begin(); std::next(iter) != bounce_path.end();
         ++iter) {
      if (*iter == 1) { continue; }
      auto& [p, a] = result.emplace_back(*this);
      --a;
      size_t index = std::distance(bounce_path.begin(), iter);
      --p[index];
      ++p[index + 1];
    }
    if (*bounce_path.rbegin() != 1) {
      auto& [p, a] = result.emplace_back(*this);
      --a;
      --*p.rbegin();
      p.append(1);
    }
    return result;
  }

  template <typename H>
  friend H AbslHashValue(H h, BouncePathAndArea const& bpa) {
    return H::combine(std::move(h), bpa.bounce_path, bpa.area);
  }
};

absl::flat_hash_map<BouncePathAndArea, absl::flat_hash_set<chalk::DyckPath>>
ClassifiedPaths(size_t path_size) {
  absl::flat_hash_map<BouncePathAndArea, absl::flat_hash_set<chalk::DyckPath>>
      result;
  for (auto&& path : chalk::DyckPath::All(path_size)) {
    auto& entry = result[BouncePathAndArea{
        .bounce_path = chalk::BouncePath(path),
        .area        = chalk::Area(path),
    }];
    entry.insert(std::move(path));
  }
  return result;
}

int main(int argc, char const* argv[]) {
  if (argc != 2) { Abort("Must provide a size."); }
  auto paths = ClassifiedPaths(ValidateInteger(argv[1]));
  std::cout << "digraph {\n  compound=true;\n";

  for (auto const& [bpa, ps] : paths) {
    std::cout << "  subgraph cluster_" << reinterpret_cast<uintptr_t>(&bpa) << " {\n"
              << "    node [style=filled];\n"
              << "    label = \"" << bpa.bounce_path << " area = " << bpa.area
              << "\";\n";
    for (auto const& p : ps) {
      std::stringstream ss;
      ss << chalk::Image(p);
      std::cout << "    p" << reinterpret_cast<uintptr_t>(&p)
                << " [label=" << std::quoted(ss.str()) << "];\n";
    }
    std::cout << "  }\n";
  }
  for (auto const& [bpa, ps] : paths) {
    for (auto const& n : bpa.Neighbors()) {
      auto iter = paths.find(n);
      if (iter == paths.end()) { continue; }
      std::cout << "  p" << reinterpret_cast<uintptr_t>(&*ps.begin()) << " -> p"
                << reinterpret_cast<uintptr_t>(&*iter->second.begin())
                << " [ltail=cluster_" << reinterpret_cast<uintptr_t>(&bpa)
                << ", lhead=cluster_"
                << reinterpret_cast<uintptr_t>(&iter->first) << "];\n";
    }
  }
  std::cout << "}\n";
  return 0;
}
