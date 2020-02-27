set -ex

CC=/home/jianjun/data188/data/public-space/toolchains/arm-himix100-linux/arm-himix100-linux/bin/arm-himix100-linux-g++

$CC -g -std=c++11 -pthread file_server_upload.cpp -o file_server_upload-himix100
$CC -g -std=c++11 -pthread file_client_upload.cpp -o file_client_upload-himix100

