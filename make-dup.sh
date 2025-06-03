#!/bin/bash
cd ../archive/
rm -rf ./src$1
rm -f ./src$1.tar.gz.dup
mv ./src$1.tar.gz ./src$1.tar.gz.dup
cp -rf ../src ./src$1
tar czf src$1.tar.gz src$1
ls
ls src$1/
cd ../src
