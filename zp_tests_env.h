#include <cstdio>
#include <vector>
#include <map>
#include <string>

#include "gtest/gtest.h"
#include "libzp/include/zp_cluster.h"
#include "libzp/include/zp_option.h"
#include "slash/include/base_conf.h"

const std::string kConfFile = "zep-server.conf";
const std::string kMetaMaster = "meta-master";
const std::string kMetaSlave1 = "meta-slave1";
const std::string kMetaSlave2 = "meta-slave2";
const std::string kDataServerCount = "data-server-count";
const std::string kDataServer = "data-server";
const std::string kDataServerPath = "data-server-path";

class ZpTestEnvironment : public ::testing::Environment {
 public:
  ZpTestEnvironment();

  void SetUp() override;
  void TearDown() override;

  int ExecScript(const std::string& cmd, std::string* output);

  void Debug();

  libzp::Cluster* zp_cluster_;
  std::string table_name_;
  slash::BaseConf conf_;
  libzp::Node meta_master_;
  libzp::Node meta_slave1_;
  libzp::Node meta_slave2_;
  int data_server_count_;
  std::map<std::string, std::vector<libzp::Node>> data_server_;
  std::map<std::string, std::vector<std::string>> data_server_path_;
};

extern ZpTestEnvironment* g_zp_environment;
