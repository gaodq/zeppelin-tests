#include "gtest/gtest.h"
#include "slash/include/base_conf.h"
#include "libzp/include/zp_cluster.h"
#include "zp_tests_env.h"

extern ZpTestEnvironment* g_zp_environment;

class ZpSyncTests : public testing::Test {
 protected:
  void SetUp() override {
    zp_cluster_ = g_zp_environment->zp_cluster_;
  }

  libzp::Cluster* zp_cluster_;
};

TEST_F(ZpSyncTests, WriteKey) {
  ASSERT_EQ(zp_cluster_->Set("test", "testkey1", "testvalue1").ok(), true);
}

TEST_F(ZpSyncTests, ReadKey) {
  std::string value;
  ASSERT_EQ(zp_cluster_->Get("test", "testkey1", &value).ok(), true);
  ASSERT_EQ(value, "testvalue1");
}
