#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_dir="$(cd -- "${script_dir}/.." && pwd)"
cxx="${CXX:-g++}"

"${cxx}" -std=c++98 -o "${script_dir}/basic" -I"${project_dir}/src" \
	"${project_dir}/src/wrench.cpp" "${script_dir}/basic.cpp"
"${cxx}" -std=c++98 -o "${script_dir}/multi" -I"${project_dir}/src" \
	"${project_dir}/src/wrench.cpp" "${script_dir}/multi_context.cpp"
