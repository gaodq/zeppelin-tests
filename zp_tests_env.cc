#include "zp_tests_env.h"

#include "gtest/gtest.h"
#include "libzp/include/zp_cluster.h"

ZpTestEnvironment* g_zp_environment = nullptr;

GTEST_API_ int main(int argc, char **argv) {
  g_zp_environment = dynamic_cast<ZpTestEnvironment*>(
      testing::AddGlobalTestEnvironment(new ZpTestEnvironment));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
