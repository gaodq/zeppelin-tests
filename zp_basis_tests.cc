#include <gtest/gtest.h>
#include <slash/include/base_conf.h>
#include "zp_tests_env.h"

extern ZpTestEnvironment* g_zp_environment;

class ZpBasisTests : public testing::Test {
 protected:
  void SetUp() override {
    zp_cluster_ = g_zp_environment->zp_cluster_;
  }

  libzp::Cluster* zp_cluster_;
};


TEST_F(ZpBasisTests, CreateTable) {
  ASSERT_EQ(0, 0);
}
