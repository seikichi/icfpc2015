#include <fstream>
#include <map>
#include <string>

#include "gtest/gtest.h"
#include "util.h"

using namespace std;

TEST(PMATest, PMA) {
  vector<string> f = {
    "abc",
    "cccdd",
    "a",
    "cacaca"
  };

  util::PMA pma;
  pma.Build(f);
  util::Node* node = pma.GetInitialNode();

  EXPECT_TRUE(node);

  string s("zabcccdd");
  vector<string> answers = {
    "0000",
    "0010",
    "0000",
    "1000",
    "0000",
    "0000",
    "0000",
    "0100",
  };

  for (int i = 0; i < (int)s.size(); ++i) {
    auto ac = pma.UpdateNode(s[i], node);
    util::AcceptIndex ans;
    for (int j = 0; j < (int)answers[i].size(); ++j) {
      ans[j] = answers[i][j] == '1';
    }
    EXPECT_EQ(ans, ac);
  }
}
