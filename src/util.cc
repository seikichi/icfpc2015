#include <queue>
#include "util.h"

using namespace std;

namespace util {

void PMA::Build(std::vector<std::string> &words) {
  root = new Node();

  // make trie
  for (int i = 0; i < (int)words.size(); ++i) {
    Node* node = root;
    for (auto& ct : words[i]) {
      int k = (int)ct;
      if (node->to[k] == NULL) node->to[k] = new Node();
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

}  // namespace util
