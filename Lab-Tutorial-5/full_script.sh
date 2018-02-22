#!/bin/bash

source_file=$1
dest_file=$2
port1=1729
port2=3141

g++ --std=c++11 server.cpp -lpthread -o server
g++ --std=c++11 client.cpp -o client

./client $port1 $port2 $dest_file & ./server $port1 $port2 $source_file
