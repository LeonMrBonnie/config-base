@echo off

IF NOT EXIST build\ (
    mkdir build
)
pushd build
cmake . -A x64 ..
popd
