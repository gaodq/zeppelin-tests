#include <stdlib.h>

#include "zp_tests_env.h"

#include "gtest/gtest.h"
#include "libzp/include/zp_cluster.h"

ZpTestEnvironment* g_zp_environment = nullptr;

ZpTestEnvironment::ZpTestEnvironment()
    : zp_cluster_(nullptr),
      conf_(kConfFile),
      data_server_count_(0) {
  conf_.LoadConf();
  std::string res;
  conf_.GetConfStr(kMetaMaster, &res);
  slash::ParseIpPortString(res, meta_master_.ip, meta_master_.port);
  conf_.GetConfStr(kMetaSlave1, &res);
  slash::ParseIpPortString(res, meta_slave1_.ip, meta_slave1_.port);
  conf_.GetConfStr(kMetaSlave2, &res);
  slash::ParseIpPortString(res, meta_slave2_.ip, meta_slave2_.port);
  conf_.GetConfInt(kDataServerCount, &data_server_count_);
  for (int i = 1; i <= data_server_count_; i++) {
    std::vector<std::string> vec_res;
    std::string d_server = kDataServer + std::to_string(i);
    std::string d_server_path = kDataServerPath + std::to_string(i);

    conf_.GetConfStrVec(d_server, &vec_res);
    std::vector<libzp::Node> nodes;
    std::string host_ip;
    for (auto& n : vec_res) {
      libzp::Node dn;
      slash::ParseIpPortString(n, dn.ip, dn.port);
      nodes.push_back(dn);
      host_ip = dn.ip;
    }
    data_server_.insert(std::make_pair(host_ip, nodes));

    vec_res.clear();
    conf_.GetConfStrVec(d_server_path, &vec_res);
    std::vector<std::string> paths;
    data_server_path_.insert(std::make_pair(host_ip, vec_res));
  }
#if 0
  table_name_.assign("test_" + std::to_string(time(nullptr)));
#else
  table_name_.assign("test1");
#endif
}

void ZpTestEnvironment::SetUp() {
  libzp::Options options;
  options.meta_addr.push_back(meta_master_);
  options.meta_addr.push_back(meta_slave1_);
  options.meta_addr.push_back(meta_slave2_);
  options.op_timeout = 5000;  // 5s
  zp_cluster_ = new libzp::Cluster(options);
}

void ZpTestEnvironment::TearDown() {
#if 0
  zp_cluster_->DropTable(table_name_);
#endif
  delete zp_cluster_;
}

int ZpTestEnvironment::ExecScript(const std::string& cmd, std::string* output) {
  char buf[1024] = {0};
  FILE* out = popen(cmd.c_str(), "r");
  fread(buf, sizeof(char), sizeof(buf), out);
  output->assign(buf);
  return pclose(out);
}

void ZpTestEnvironment::Debug() {
  std::cout << "zp_cluster_: " << zp_cluster_ << std::endl;
  std::cout << "Conf dump: " << std::endl;
  conf_.DumpConf();
  std::cout << "Meta master: " << meta_master_ << std::endl;
  std::cout << "Meta slave1: " << meta_slave1_ << std::endl;
  std::cout << "Meta slave2: " << meta_slave2_ << std::endl;
  std::cout << "Data servers: " << data_server_count_ << std::endl;
  for (auto& ds : data_server_) {
    std::cout << "  Host: " << ds.first << std::endl;
    for (auto& n : ds.second) {
      std::cout << "    Node: " << n << std::endl;
    }
    auto paths = data_server_path_.find(ds.first);
    assert(paths != data_server_path_.end());
    for (auto& p : paths->second) {
      std::cout << "    Path: " << p << std::endl;
    }
  }
}

GTEST_API_ int main(int argc, char **argv) {
  g_zp_environment = dynamic_cast<ZpTestEnvironment*>(
      testing::AddGlobalTestEnvironment(new ZpTestEnvironment));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
