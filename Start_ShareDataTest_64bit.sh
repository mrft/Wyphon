#!/bin/bash

echo "Copy all relevant dlls and start the 32 bit version!"

# version = x64/Debug or x64/Release
VERSION="x64/Debug"
BIN_PATH="bin/${VERSION}"
APP_PATH="./ShareDataTest/${BIN_PATH}"

cp ./${VERSION}/*.dll ${APP_PATH}
cp ./WyphonDotNet/${BIN_PATH}/*.dll ${APP_PATH}
cp ./LocalMessageBroadcast/${BIN_PATH}/*.dll ${APP_PATH}

${APP_PATH}/ShareDataTest.exe
