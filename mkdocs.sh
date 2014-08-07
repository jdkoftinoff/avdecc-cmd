relative_dir="`dirname "$0"`"
TOP="`cd "$relative_dir" && pwd`"
export TOP
doxygen "$TOP"/docs-dev/Doxyfile
