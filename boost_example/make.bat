cd build64
cmake -DCMAKE_PREFIX_PATH="E:\usr" -A x64  ..
pause

protoc --cpp_out=./include *.proto