#pragma once
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
  int pos;
  Node(int pos) : failure(NULL), pos(pos) { memset(to, 0, sizeof(to)); }
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


struct Random {
  unsigned int x;
  unsigned int y;
  unsigned int z;
  unsigned int w;
  Random() : x(0x34fb2383), y(0x327328fa), z(0xabd4b54a), w(0xa9dba8d1) {;}
  Random(int s) : x(0x34fb2383), y(0x327328fa), z(0xabd4b54a), w(s) {
    for (int i = 0; i < 100; i++) { Xor128(); }
  }
  void Seed(int s) {
    *this = Random(s);
  }
  unsigned int Xor128() {
    unsigned int t;
    t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
  }
  int next(int r) { return Xor128() % r; }
  int next(int l, int r) { return next(r - l + 1) + l; }
  long long next(long long r) { return (long long)((((unsigned long long)Xor128() << 32) + (unsigned long long)Xor128()) % r); }
  long long next(long long l, long long r) { return next(r - l + 1) + l; }
  double next(double r) { return (double)Xor128() / 0xffffffff * r; }
  double next(double l, double r) { return next(r - l) + l; }
};
}  // namespace util
