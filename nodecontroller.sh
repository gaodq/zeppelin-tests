#!/bin/bash

option=$1
ip=$2
port=$3
node_id=${port: -2:2}

main() {
  case $option in
    "start_node")
      start_node
      ;;
    "stop_node")
      stop_node
      ;;
    "restart_node")
      restart_node
      ;;
    "start_meta")
      start_meta
      ;;
    "stop_meta")
      stop_meta
      ;;
    "restart_meta")
      restart_meta
      ;;
    "node_status")
      node_status
      ;;
    "meta_status")
      meta_status
      ;;
  esac
}

start_node() {
  ret=$(ssh -t ${USER}@${ip} "sudo systemctl start zep-node@${node_id}.service")
  echo $ret
}

stop_node() {
  ret=$(ssh -t ${USER}@${ip} "sudo systemctl stop zep-node@${node_id}.service")
  echo $ret
}

restart_node() {
  ret=$(ssh -t ${USER}@${ip} "sudo systemctl restart zep-node@${node_id}.service")
  echo $ret
}

node_status() {
  ret=$(ssh -t ${USER}@${ip} "sudo systemctl status zep-node@${node_id}.service")
  echo $ret
}

start_meta() {
  ret=$(ssh -t ${USER}@${ip} "sudo systemctl start zep-meta.service")
  echo $ret
}

stop_meta() {
  ret=$(ssh -t ${USER}@${ip} "sudo systemctl stop zep-meta.service")
  echo $ret
}

restart_meta() {
  ret=$(ssh -t ${USER}@${ip} "sudo systemctl restart zep-meta.service")
  echo $ret
}

meta_status() {
  ret=$(ssh -t ${USER}@${ip} "sudo systemctl status zep-meta.service")
  echo $ret
}

main
