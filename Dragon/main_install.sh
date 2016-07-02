#!/bin/sh

# set your 3rdparty_path
_3RDPARTH_PATH=xxxxxxxxxxxxxxxxxxx

mkdir build
sudo cp $_3RDPARTH_PATH/bin/protoc /usr/bin
sudo chmod +x bin/gen_proto.sh
sudo ./bin/gen_proto.sh
cd build
cmake ..
make install -j16