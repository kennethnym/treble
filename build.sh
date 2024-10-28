#/bin/bash

set -eu
script_path="$(realpath $0)"
root="$(dirname $script_path)"

pushd "$(dirname $0)"

compiler=${CC:-g++}

src_files=(
    src/main.cxx
	src/module.cxx
	src/runtime.cxx
)

common_opts="-I$root/src -Wall --std=c++20"
debug_opts="--debug -g -DDEBUG $common_opts"

popd >> /dev/null

mkdir -p build
pushd build >> /dev/null

all_src=""
for p in "${src_files[@]}"; do
	all_src+=" ../${p}"
done

compile="$compiler $all_src -o treble $debug_opts"

echo $compile
$compile

popd >> /dev/null

