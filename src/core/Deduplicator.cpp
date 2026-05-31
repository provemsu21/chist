#include "Deduplicator.hpp"
#include "Log.hpp"
#include "Tree.hpp"
#include <string>
#include <unordered_map>

namespace deduplicator {

namespace {

template <typename Cont> size_t countFiles(const Cont &cont) {
  size_t total = 0;
  for (const auto &[f, vec] : cont) {
    total += vec.size();
  }
  return total;
}

template <typename Key>
std::vector<std::pair<Key, fs::path>>
toVector(const std::unordered_map<Key, std::vector<fs::path>> &map) {
  std::vector<std::pair<Key, fs::path>> vec;

  for (const auto &[key, v] : map) {
    for (const auto &p : v) {
      vec.emplace_back(key, p);
    }
  }

  return vec;
}

} // namespace

HashTable findDuplicates(const fs::path &path, hasher::HashType type,
                         progress::IProgress *prog) {
  LOG_DEBUG("findDuplicates: starting tree-based scan of " << path);

  std::shared_ptr<tree::TreeNode> root;
  {
    TIME_SCOPE("stage 1: build tree");
    root = tree::buildTree(path, prog);
  }

  {
    TIME_SCOPE("stage 2: hash tree");
    tree::hashTree(root, type, prog);
  }

  std::vector<tree::DuplicateGroup> groups;
  {
    TIME_SCOPE("stage 3: group by hash");
    groups = tree::findGroups(root);
  }
  LOG_DEBUG("raw groups: " << groups.size());

  {
    TIME_SCOPE("stage 4: filter covered");
    groups = tree::filterCovered(groups);
  }
  LOG_DEBUG("visible groups: " << groups.size());

  HashTable table;
  for (const auto &group : groups) {
    if (group.empty())
      continue;
    auto &paths = table[group[0]->content_hash];
    paths.reserve(group.size());
    for (const auto &node : group) {
      paths.push_back(node->path);
    }
  }
  return table;
}
} // namespace deduplicator
