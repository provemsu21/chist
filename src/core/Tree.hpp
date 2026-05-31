#ifndef CORE_TREE_HPP_
#define CORE_TREE_HPP_

#include "Hasher.hpp"
#include "cli/Progress.hpp"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace tree {

struct TreeNode;

namespace fs = std::filesystem;

using DuplicateGroup = std::vector<std::shared_ptr<TreeNode>>;

struct TreeNode {
  fs::path path;
  uintmax_t size;
  bool is_directory;

  std::string content_hash;
  std::weak_ptr<TreeNode> parent;
  std::vector<std::shared_ptr<TreeNode>> children;
};

std::shared_ptr<TreeNode> buildTree(const fs::path &root,
                                    progress::IProgress *prog = nullptr);
void hashTree(std::shared_ptr<TreeNode> root, hasher::HashType type,
              progress::IProgress *prog = nullptr);

std::vector<DuplicateGroup> findGroups(const std::shared_ptr<TreeNode> &root,
                                       progress::IProgress *prog = nullptr);
std::vector<DuplicateGroup>
filterCovered(const std::vector<DuplicateGroup> &groups,
              progress::IProgress *prog = nullptr);

} // namespace tree

#endif // CORE_TREE_HPP_
