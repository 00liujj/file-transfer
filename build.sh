set -ex

g++ -g -std=c++11 -pthread file_server_upload.cpp -o file_server_upload
g++ -g -std=c++11 -pthread  file_client_upload.cpp -o file_client_upload

