#include <cstdio>

#include "libzp/include/zp_cluster.h"
#include "gtest/gtest.h"

class ZpTestEnvironment : public ::testing::Environment {
 public:
  void SetUp() override {
    zp_cluster_ = new libzp::Cluster("127.0.0.1", 9221);
  }

  void TearDown() override {
    delete zp_cluster_;
  }

  libzp::Cluster* zp_cluster_;
};

extern ZpTestEnvironment* g_zp_environment;
