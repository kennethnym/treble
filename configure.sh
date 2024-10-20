#/bin/bash

set -eu

script_path="$(realpath $0)"
root="$(dirname $script_path)"
pushd "$(dirname "$0")" > /dev/null

cat > .clangd <<- EOF
CompileFlags:
  Add:
    - -I$root/src
      - --std=c++20
      - -Wall
EOF

