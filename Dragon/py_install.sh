#!/bin/sh

# set your 3rdparty path
_3RDPARTY_PATH=xxxxxxxxxxxxxxxxxxx
# set your anaconda_path
ANACONDA_PATH=xxxxxxxxxxxxxxxxxxx

echo copy _dragon.so .....
cp lib/lib_dragon.so ext/python/dragon/_dragon.so
echo copy cv2.so .....
cp $_3RDPARTY_PATH/bin/cv2.so $ANACONDA_PATH/lib/python2.7/site-packages/cv2.so
SRC_DIR=./src/protos
DST_DIR=./ext/python/dragon
PROTO_NAME=dragon
echo Protocol Buffer Compliing for dragon.proto .....
protoc -I=$SRC_DIR --python_out=$DST_DIR $SRC_DIR/$PROTO_NAME.proto
echo install pydragon to $ANACONDA_PATH/lib/python2.7/site-packages .....
cp -r ext/python/dragon $ANACONDA_PATH/lib/python2.7/site-packages