#include "FsWalker.hpp"
#include "ThreadPool.hpp"
#include "Tree.hpp"
#include "cli/Style.hpp"
#include "core/Hasher.hpp"
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace tree {

namespace {

constexpr size_t HEAD_BYTES = 4096;

void collectFileNodes(std::shared_ptr<TreeNode> node,
                      std::vector<std::shared_ptr<TreeNode>> &out) {
  if (!node->is_directory) {
    out.push_back(node);
    return;
  }

  for (const auto &child : node->children) {
    collectFileNodes(child, out);
  }
}

std::string computeFileHash(const TreeNode &node, hasher::HashType type) {
  std::string head = hasher::getHeadHash(node.path, HEAD_BYTES, type);
  if (head.empty()) {
    return {};
  }

  return head + ":" + std::to_string(node.size);
}

void hashDirs(const std::shared_ptr<TreeNode> node, hasher::HashType type) {
  if (!node->is_directory)
    return;

  for (const auto &child : node->children) {
    hashDirs(child, type);
  }

  std::vector<std::pair<std::string, std::string>> entries;
  entries.reserve(node->children.size());
  for (const auto &child : node->children) {
    if (child->content_hash.empty())
      continue;
    entries.emplace_back(child->path.filename().string(), child->content_hash);
  }

  if (entries.empty()) {
    return;
  }

  std::sort(entries.begin(), entries.end());

  std::string buffer;
  for (const auto &[name, hash] : entries) {
    buffer.append(name);
    buffer.push_back('\0');
    buffer.append(hash);
    buffer.push_back('\0');
  }

  node->content_hash = hasher::hashBytes(buffer.data(), buffer.size(), type);
}

void collectAll(const std::shared_ptr<TreeNode> &node,
                std::unordered_map<std::string, DuplicateGroup> &by_hash) {
  if (!node->content_hash.empty()) {
    by_hash[node->content_hash].push_back(node);
  }
  for (const auto &child : node->children) {
    collectAll(child, by_hash);
  }
}

bool isCoveredByParents(const DuplicateGroup &group) {
  std::vector<TreeNode *> parents;
  parents.reserve(group.size());

  for (const auto &member : group) {
    auto p = member->parent.lock();
    if (!p) {
      return false;
    }
    parents.push_back(p.get());
  }

  std::unordered_set<TreeNode *> unique_parents(parents.begin(), parents.end());
  if (unique_parents.size() != parents.size()) {
    return false;
  }

  const std::string &first_hash = parents[0]->content_hash;
  if (first_hash.empty()) {
    return false;
  }
  for (size_t i = 1; i < parents.size(); ++i) {
    if (parents[i]->content_hash != first_hash) {
      return false;
    }
  }

  return true;
}

} // namespace

std::vector<DuplicateGroup> findGroups(const std::shared_ptr<TreeNode> &root,
                                       progress::IProgress *prog) {
  std::unordered_map<std::string, DuplicateGroup> by_hash;
  collectAll(root, by_hash);

  std::vector<DuplicateGroup> groups;
  for (auto &[hash, group] : by_hash) {
    if (group.size() >= 2) {
      groups.push_back(std::move(group));
    }
  }
  return groups;
}

std::vector<DuplicateGroup>
filterCovered(const std::vector<DuplicateGroup> &groups,
              progress::IProgress *prog) {
  std::vector<DuplicateGroup> visible;
  for (const auto &group : groups) {
    if (!isCoveredByParents(group)) {
      visible.push_back(group);
    }
  }
  return visible;
}

std::shared_ptr<TreeNode> buildTree(const fs::path &root,
                                    progress::IProgress *prog) {
  auto root_node = std::make_shared<TreeNode>();
  root_node->path = root;
  root_node->is_directory = true;

  std::shared_ptr<TreeNode> cur_dir_node = root_node;

  fswalker::Visitor visitor;

  size_t file_cnt = 0;

  visitor.onDir = [&cur_dir_node](const fs::path &p, const struct stat &st) {
    auto node = std::make_shared<TreeNode>();
    node->path = p;
    node->is_directory = true;
    node->parent = cur_dir_node;
    cur_dir_node->children.push_back(node);

    cur_dir_node = node;
  };

  visitor.onFile = [&prog, &file_cnt, &cur_dir_node](const fs::path &p,
                                                     const struct stat &st) {
    auto node = std::make_shared<TreeNode>();
    node->path = p;
    node->is_directory = false;
    node->size = static_cast<uintmax_t>(st.st_size);
    node->parent = cur_dir_node;
    cur_dir_node->children.push_back(node);

    ++file_cnt;
    if (prog) {
      prog->updateLine(style::colored("Scanning: ", style::yellow()),
                       p.string());
    }
  };

  visitor.onDirExit = [&cur_dir_node](const fs::path &) {
    if (auto p = cur_dir_node->parent.lock()) {
      cur_dir_node = p;
    }
  };

  fswalker::walk(root, visitor);
  if (prog) {
    std::string fin =
        style::colored("Scan complete: " + std::to_string(file_cnt) + " files",
                       style::green());
    prog->finish(fin);
  }

  return root_node;
}

void hashTree(std::shared_ptr<TreeNode> root, hasher::HashType type,
              progress::IProgress *prog) {
  std::vector<std::shared_ptr<TreeNode>> file_nodes;
  collectFileNodes(root, file_nodes);

  const size_t total = file_nodes.size();
  std::atomic<size_t> hashed{0};

  threadpool::parallel_for_each(
      file_nodes,
      [&prog, &hashed, type, total](std::shared_ptr<TreeNode> node) {
        node->content_hash = computeFileHash(*node, type);
        size_t done = ++hashed;
        if (prog)
          prog->bar("Hashing ", done, total);
      });

  if (prog) {
    std::string fin = style::colored(
        "Hashed: " + std::to_string(hashed) + " files", style::green());
    prog->finish(fin);
  }
  hashDirs(root, type);
}

} // namespace tree
