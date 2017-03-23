#!/bin/bash
cd src
../libraries/bin/protoc --cpp_out=./ cncstatebuf.proto
cd ..
