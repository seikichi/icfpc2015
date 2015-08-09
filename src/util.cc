#include <queue>
#include "util.h"

using namespace std;

namespace util {

void PMA::Build(std::vector<std::string> &words) {
  root = new Node(0);

  // make trie
  for (int i = 0; i < (int)words.size(); ++i) {
    Node* node = root;
    for (auto& ct : words[i]) {
      int k = (int)ct;
      if (node->to[k] == NULL) node->to[k] = new Node(i + 1);
      node = node->to[k];
    }
    node->ac.set(i, 1);
  }

  // make failure links by bfs
  queue<Node*> q;
  q.push(root);
  while (!q.empty()) {
    Node *node = q.front(); q.pop();
    for (int k = 0; k < 128; ++k) if (node->to[k]) {
      Node *next = Go(node, k);
      q.push(next);

      if (node != root) {
        Node *f = node->failure;
        for (; Go(f, k) == NULL; f = f->failure) ;
        next->failure = Go(f, k);
        next->ac |= next->failure->ac;
      } else {
        next->failure = root;
      }
    }
  }
}

AcceptIndex PMA::UpdateNode(char c, Node*& node) const {
  int k = (int)c;
  while (Go(node, k) == NULL) node = node->failure;
  node = Go(node, k);
  return node->ac;
}

Node* PMA::Go(Node* n, int k) const {
  if (n->to[k] != NULL) return n->to[k];
  if (n == root) return root;
  return NULL;
}

std::vector<std::vector<char> > command_map;
std::unordered_map<char, Command> rev_command_map;
  void Init() {
    if (rev_command_map.size() != 0) { return; }
    command_map =  {
      { 'p', '\'', '!', '.', '0', '3', },
      { 'b', 'c', 'e', 'f', 'y', '2', },
      { 'a', 'g', 'h', 'i', 'j', '4', },
      { 'l', 'm', 'n', 'o', ' ', '5', },
      { 'd', 'q', 'r', 'v', 'z', '1', },
      { 'k', 's', 't', 'u', 'w', 'x', },
      { '\t', '\n', '\r', },
    };
    for (int i = 0; i < (int)Command::SIZE; i++) {
      for (char c : command_map[i]) {
        rev_command_map[c] = (Command)i;
      }
    }
  }
}  // namespace util
