@echo off
set _3RDPARTY_PATH=xxxxxxxxxxxxxxxxxxx
set ANACONDA_PATH=xxxxxxxxxxxxxxxxxxx

echo copy _dragon.pyd .....
copy /y .\lib\_dragon.dll .\ext\python\dragon\_dragon.pyd
echo copy cv2.so .....
copy /y %_3RDPARTY_PATH%\bin\cv2.pyd %ANACONDA_PATH%\lib\cv2.pyd
set SRC_DIR=.\src\protos
set DST_DIR=.\ext\python\dragon
set PROTO_NAME=dragon
echo Protocol Buffer£ºCompliing for dragon.proto.....
start protoc -I=%SRC_DIR% --python_out=%DST_DIR% %SRC_DIR%\%PROTO_NAME%.proto
echo Protocol Buffer£ºCompliing complete!
echo install pydragon to %ANACONDA_PATH%/lib/python2.7/site-packages .....
xcopy /s /y .\ext\python\dragon %ANACONDA_PATH%\lib\site-packages\dragon\
pause