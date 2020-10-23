# This script is meant to be sourced from other scripts

# Check for clang-format, prefer 6.0 if available
if [[ -x "$(command -v clang-format-6.0)" ]]; then
  clang_format=clang-format-6.0
elif [[ -x "$(command -v clang-format)" ]]; then
  clang_format=clang-format
else
  echo "clang-format or clang-format-6.0 must be installed"
  exit 1
fi

# Find all source files in the project minus those that are auto-generated or we do not maintain
src_files=`find . -not -path "*/imgui/*" -not -path "*/raylib/*" -not -path "*/build/*" -not -path "*/raylib-3D/*" -name '*.h' -o -not -path "*/imgui/*" -not -path "*/raylib/*" -not -path "*/build/*" -not -path "*/raylib-3D/*" -name '*.cpp'`
