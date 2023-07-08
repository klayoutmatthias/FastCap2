#!/bin/bash -e

temp=$(pwd)/.tmp
dest=$(pwd)/../docs
src=$(pwd)/..

rm -rf $temp
mkdir $temp

pushd $src
python3 setup.py bdist
export PYTHONPATH=$(echo $(pwd)/build/lib.*)
popd 

sphinx-build -M html $(pwd) $temp

rm -rf $dest
mkdir $dest
cp -R $temp/html/* $dest

touch $dest/.nojekyll

rm -rf $temp

echo "Make HTML pages in $dest."

