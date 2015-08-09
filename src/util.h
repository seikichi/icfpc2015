#include <bitset>
#include <cstring>
#include <string>
#include <vector>

// Pattern Matching Automaton
namespace util {

typedef std::bitset<20> AcceptIndex;
struct Node {
  Node* to[128];
  AcceptIndex ac;
  Node* failure;
  Node() : failure(NULL) { memset(to, 0, sizeof(to)); }
};

struct PMA {
  Node* root;

  // Build PMA
  void Build(std::vector<std::string> &words);

  // Get initial node
  Node* GetInitialNode() const { return root; }

  // Return accepted IDs
  AcceptIndex UpdateNode(char c, Node*& node) const;

  // Internal function
  Node* Go(Node* n, int k) const;
};

}  // namespace util
