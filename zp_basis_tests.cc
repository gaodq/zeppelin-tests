#include <string>
#include <set>
#include <vector>

#include "gtest/gtest.h"
#include "slash/include/base_conf.h"
#include "slash/include/slash_string.h"
#include "slash/include/slash_status.h"
#include "libzp/include/zp_cluster.h"
#include "zp_tests_env.h"

extern ZpTestEnvironment* g_zp_environment;

class ZpBasisTests : public testing::Test {
 public:
  void SetUp() override {
    zp_cluster_ = g_zp_environment->zp_cluster_;
    table_name_ = g_zp_environment->table_name_;
  }

  libzp::Cluster* zp_cluster_;
  std::string table_name_;
};

class ZpNodeTests : public ZpBasisTests {
};

class ZpMetaTests : public ZpBasisTests {
};

static void CountDown(const char* msg, int timeout) {
  printf("%s", msg);
  fflush(stdout);
  int wait_times = timeout;
  do {
    if (wait_times == 10) {
      printf(" %2ds", wait_times);
    } else {
      printf("\b\b\b%2ds", wait_times);
    }
    fflush(stdout);
    sleep(1);
  } while (wait_times--);
  printf("\n");
}

static int64_t NewestEpoch(libzp::Cluster* zp_cluster,
                           const std::string& table_name) {
  int retries = 10;
  int64_t epoch;
  while (retries--) {
    zp_cluster->Pull(table_name);
    epoch = zp_cluster->Epoch();
  }
  return epoch;
}

// TODO(gaodq)
#if 0
TEST_F(ZpBasisTests, CreateTable) {
  ASSERT_TRUE(zp_cluster_->CreateTable(table_name_, 3).ok());

  CountDown("Waiting for zeppelin create table", 10);
  ASSERT_TRUE(zp_cluster_->Pull(table_name_).ok());

  std::vector<libzp::Node> nodes;
  std::vector<std::string> status;
  ASSERT_TRUE(zp_cluster_->ListNode(&nodes, &status).ok());

  std::map<int, libzp::PartitionView> partitions;
  for (auto& node : nodes) {
    partitions.clear();
    zp_cluster_->InfoRepl(node, table_name_, &partitions);
    for (auto& item : partitions) {
      libzp::PartitionView& view = item.second;
      for (auto& slave : view.slaves) {
        ASSERT_NE(view.master.ip, slave.ip);
      }
    }
  }
}
#endif

static void RestoreNode(libzp::Cluster* zp_cluster, const libzp::Node& node) {
  std::vector<libzp::Node> nodes;
  std::vector<std::string> status;
  std::cout << "Waiting for node " << node << " restore";
  std::string cmd = "sh nodecontroller.sh start_node " +
    node.ip + " " + std::to_string(node.port);
  std::string output;
  ASSERT_EQ(g_zp_environment->ExecScript(cmd, &output), 0);
  int max_retries = 20;  // wait for 20 s
  bool node_up = false;
  do {
    sleep(1);
    printf(".");
    fflush(stdout);
    nodes.clear();
    status.clear();
    ASSERT_TRUE(zp_cluster->ListNode(&nodes, &status).ok());
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i] == node && status[i] == "up") {
        node_up = true;
      }
    }
  } while (!node_up && max_retries--);
  printf("\n");
  ASSERT_TRUE(node_up);
}

static void StopNode(libzp::Cluster* zp_cluster, const libzp::Node& node) {
  std::vector<libzp::Node> nodes;
  std::vector<std::string> status;
  std::cout << "Waiting for node " << node << " down";
  std::string cmd = "sh nodecontroller.sh stop_node " +
    node.ip + " " + std::to_string(node.port);
  std::string output;
  ASSERT_EQ(g_zp_environment->ExecScript(cmd, &output), 0);
  int max_retries = 30;  // wait for 30 s
  bool node_down = false;
  do {
    sleep(1);
    printf(".");
    fflush(stdout);
    nodes.clear();
    status.clear();
    ASSERT_TRUE(zp_cluster->ListNode(&nodes, &status).ok());
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i] == node && status[i] == "down") {
        node_down = true;
      }
    }
  } while (!node_down && max_retries--);
  printf("\n");
  ASSERT_TRUE(node_down);
}

#if 1
TEST_F(ZpNodeTests, SlaveExceptional) {
  // Find node with out master
  ASSERT_TRUE(zp_cluster_->Pull(table_name_).ok());
  libzp::Table ori_meta;
  ASSERT_TRUE(zp_cluster_->FetchMetaInfo(table_name_, &ori_meta).ok());
  std::map<libzp::Node, std::vector<const libzp::Partition*>> loads;
  ori_meta.GetNodesLoads(&loads);
  ASSERT_GT(loads.size(), 0);
  libzp::Node slave_node;
  for (auto& item : loads) {
    bool finded = true;
    slave_node = item.first;
    for (auto p : item.second) {
      if (p->master() == item.first) {
        finded = false;
      }
    }
    if (finded) {
      break;
    }
  }

  // Slave down
  int64_t ori_epoch = NewestEpoch(zp_cluster_, table_name_);

  // Shutdown this slave
  StopNode(zp_cluster_, slave_node);

  // epoch should not change
  ASSERT_TRUE(zp_cluster_->Pull(table_name_).ok());
  ASSERT_EQ(NewestEpoch(zp_cluster_, table_name_), ori_epoch);
  // TODO(gaodq) check log

  // Slave restore
  RestoreNode(zp_cluster_, slave_node);
  // epoch should not change
  ASSERT_EQ(NewestEpoch(zp_cluster_, table_name_), ori_epoch);

  // Slave fast restart
  std::string cmd = "sh nodecontroller.sh restart_node " +
    slave_node.ip + " " + std::to_string(slave_node.port);
  std::string output;
  ASSERT_EQ(g_zp_environment->ExecScript(cmd, &output), 0);
  CountDown("Waiting for cluster updating", 10);
  std::vector<libzp::Node> nodes;
  std::vector<std::string> status;
  ASSERT_TRUE(zp_cluster_->ListNode(&nodes, &status).ok());
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i] == slave_node) {
      ASSERT_EQ(status[i], "up");
    }
  }

  // epoch should not change
  ASSERT_EQ(NewestEpoch(zp_cluster_, table_name_), ori_epoch);
}
#endif

#if 1
TEST_F(ZpNodeTests, MasterExceptional) {
  // Find one node has master partition
  ASSERT_TRUE(zp_cluster_->Pull(table_name_).ok());
  libzp::Table ori_meta;
  ASSERT_TRUE(zp_cluster_->FetchMetaInfo(table_name_, &ori_meta).ok());
  std::set<libzp::Node> master_nodes;
  ori_meta.GetAllMasters(&master_nodes);
  ASSERT_GT(master_nodes.size(), 0);
  std::map<libzp::Node, std::vector<const libzp::Partition*>> loads;
  ori_meta.GetNodesLoads(&loads);
  ASSERT_GT(loads.size(), 0);

  libzp::Node master_node = *master_nodes.begin();
  std::vector<const libzp::Partition*> node_load = loads[master_node];

  // Master down
  int64_t ori_epoch = NewestEpoch(zp_cluster_, table_name_);
  StopNode(zp_cluster_, master_node);

  // Check
  int64_t new_epoch = NewestEpoch(zp_cluster_, table_name_);
  ASSERT_GT(new_epoch, ori_epoch);
  libzp::Table new_meta;
  ASSERT_TRUE(zp_cluster_->FetchMetaInfo(table_name_, &new_meta).ok());
  for (auto p : node_load) {
    auto new_p = new_meta.GetPartitionById(p->id());
    if (p->master() == master_node) {
      ASSERT_NE(new_p->master(), master_node);
    }
  }

  // Master restore
  RestoreNode(zp_cluster_, master_node);
  ASSERT_EQ(NewestEpoch(zp_cluster_, table_name_), new_epoch);

  // Master fast restart
  // Find new master node
  ASSERT_TRUE(zp_cluster_->FetchMetaInfo(table_name_, &new_meta).ok());
  master_nodes.clear();
  new_meta.GetAllMasters(&master_nodes);
  ASSERT_GT(master_nodes.size(), 0);
  master_node = *master_nodes.begin();

  std::string cmd = "sh nodecontroller.sh restart_node " +
    master_node.ip + " " + std::to_string(master_node.port);
  std::string output;
  ASSERT_EQ(g_zp_environment->ExecScript(cmd, &output), 0);
  CountDown("Waiting for cluster updating", 10);
  std::vector<libzp::Node> nodes;
  std::vector<std::string> status;
  ASSERT_TRUE(zp_cluster_->ListNode(&nodes, &status).ok());
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i] == master_node) {
      ASSERT_EQ(status[i], "up");
    }
  }

  // epoch should not change
  ASSERT_EQ(NewestEpoch(zp_cluster_, table_name_), new_epoch);
}
#endif

static void ParseMetaStatus(const std::string& meta_status,
                            std::map<std::string, std::string>* status) {
                                      // ip          role
  status->clear();
  std::istringstream input(meta_status);
  for (std::string line; std::getline(input, line); ) {
    if (line.find(":") == std::string::npos) {
      // Skip header row
      continue;
    }
    std::string ip;
    std::istringstream line_s(line);
    for (std::string word; std::getline(line_s, word, ' '); ) {
      int port;
      if (word.find(":") != std::string::npos) {
        slash::ParseIpPortString(word, ip, port);
      } else if (word == "leader" || word == "follower") {
        status->insert(std::make_pair(ip, word));
      }
    }
  }
}

static void StopMetaNode(
    libzp::Cluster* zp_cluster, const libzp::Node& node) {
  std::cout << "Stopping meta " << node;
  std::string cmd = "sh nodecontroller.sh stop_meta " +
    node.ip + " " + std::to_string(node.port);
  std::string output;
  ASSERT_EQ(g_zp_environment->ExecScript(cmd, &output), 0);
  int max_retries = 10;  // wait for 20 s
  std::map<std::string, std::string> status;
  while (max_retries--) {
    sleep(1);
    printf(".");
    fflush(stdout);
    std::string meta_status;
    ASSERT_TRUE(zp_cluster->MetaStatus(&meta_status).ok());
    ParseMetaStatus(meta_status, &status);
  } 
  printf("\n");
  ASSERT_TRUE(status.find(node.ip) == status.end());
}

static void RestoreMetaNode(
    libzp::Cluster* zp_cluster, const libzp::Node& node) {
  std::cout << "Restoring meta " << node;
  std::string cmd = "sh nodecontroller.sh start_meta " +
    node.ip + " " + std::to_string(node.port);
  std::string output;
  ASSERT_EQ(g_zp_environment->ExecScript(cmd, &output), 0);
  int max_retries = 20;  // wait for 20 s
  bool node_up = false;
  do {
    sleep(1);
    printf(".");
    fflush(stdout);
    std::string meta_status;
    ASSERT_TRUE(zp_cluster->MetaStatus(&meta_status).ok());
    std::map<std::string, std::string> status;
    ParseMetaStatus(meta_status, &status);
    if (status.find(node.ip) != status.end()) {
      node_up = true;
    }
  } while (!node_up && max_retries--);
  printf("\n");
  ASSERT_TRUE(node_up);
}

static void CheckClusterAfterMetaDown(
    libzp::Cluster* zp_cluster, const std::string& table_name) {
  // Generate keys
  std::vector<std::string> keys;
  int candidate_key = 0;
  const int partition_size = 3;
  for (int p_id = 0; p_id < partition_size; p_id++) {
    int par_num = -1;
    std::string key;
    while (par_num != p_id) {
      key = std::to_string(candidate_key++);
      par_num = std::hash<std::string>()(key) % partition_size;
    }
    keys.push_back(key);
  }

  for (auto& key : keys) {
    ASSERT_TRUE(zp_cluster->Set(table_name, key, "testvalue").ok());
    std::string value;
    ASSERT_TRUE(zp_cluster->Get(table_name, key, &value).ok());
    ASSERT_EQ(value, "testvalue");
    ASSERT_TRUE(zp_cluster->Delete(table_name, key).ok());
  }
}

#if 1
TEST_F(ZpMetaTests, SlaveExceptional) {
  libzp::Node meta_master;
  std::vector<libzp::Node> meta_slaves;
  ASSERT_TRUE(zp_cluster_->ListMeta(&meta_master, &meta_slaves).ok());

  ASSERT_TRUE(zp_cluster_->Pull(table_name_).ok());
  int64_t ori_epoch = NewestEpoch(zp_cluster_, table_name_);

  for (auto& slave : meta_slaves) {
    // Slave down
    StopMetaNode(zp_cluster_, slave);
    ASSERT_EQ(NewestEpoch(zp_cluster_, table_name_), ori_epoch);

    CheckClusterAfterMetaDown(zp_cluster_, table_name_);

    RestoreMetaNode(zp_cluster_, slave);
    ASSERT_EQ(NewestEpoch(zp_cluster_, table_name_), ori_epoch);

    libzp::Node new_master;
    std::vector<libzp::Node> new_slaves;
    ASSERT_TRUE(zp_cluster_->ListMeta(&new_master, &new_slaves).ok());
    ASSERT_EQ(new_master, meta_master);
  }
}
#endif

#if 1
TEST_F(ZpMetaTests, MasterExceptional) {
  libzp::Node old_master;
  std::vector<libzp::Node> meta_slaves;
  ASSERT_TRUE(zp_cluster_->ListMeta(&old_master, &meta_slaves).ok());

  int64_t ori_epoch = NewestEpoch(zp_cluster_, table_name_);

  // Master down
  StopMetaNode(zp_cluster_, old_master);
  CountDown("Waiting for new master", 5);
  libzp::Node new_master;
  std::map<std::string, std::string> status;
  int max_retries = 10;
  while (max_retries--) {
    std::string meta_status;
    ASSERT_TRUE(zp_cluster_->MetaStatus(&meta_status).ok());
    ParseMetaStatus(meta_status, &status);
    for (auto& item : status) {
      if (item.second == "leader") {
        new_master.ip = item.first;
        new_master.port = 9221;
        max_retries = 0;
        break;
      }
    }
  } 
  ASSERT_NE(new_master, old_master);
  libzp::Node meta_master;
  ASSERT_TRUE(zp_cluster_->ListMeta(&meta_master, &meta_slaves).ok());
  ASSERT_EQ(new_master, meta_master);
  CheckClusterAfterMetaDown(zp_cluster_, table_name_);

  // Master restore
  RestoreMetaNode(zp_cluster_, old_master);
  CountDown("Waiting for meta restore", 3);
  max_retries = 10;
  bool meta_restored = false;
  while (max_retries--) {
    std::string meta_status;
    ASSERT_TRUE(zp_cluster_->MetaStatus(&meta_status).ok());
    ParseMetaStatus(meta_status, &status);
    if (status.find(old_master.ip) != status.end()) {
      meta_restored = true;
      break;
    }
  }
  ASSERT_TRUE(meta_restored);
  ASSERT_EQ(NewestEpoch(zp_cluster_, table_name_), ori_epoch);
}
#endif

#if 0
TEST_F(ZpBasisTests, Test) {
  std::string meta_status;
  std::map<std::string, std::string> status;
  ASSERT_TRUE(zp_cluster_->MetaStatus(&meta_status).ok());
  ParseMetaStatus(meta_status, &status);
  for (auto& item : status) {
    std::cout << item.first << ", " << item.second << std::endl;
  }
}
#endif
