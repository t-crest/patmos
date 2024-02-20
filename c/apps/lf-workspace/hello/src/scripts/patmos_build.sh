# This bash file overwrites platform.h and copies lf_patmos_support couple c/h files to the correct direcotry
# Also, it copies Makefile to the src-gen directory which can later be used by patmos-clang .
# It can later be merged into main Makefile
PROJECT_ROOT="$1"
PROJECT_NAME="$2"

echo root is $PROJECT_ROOT and name is $PROJECT_NAME

cp "$PROJECT_ROOT/src/platform/lf_patmos_support.h"     "$PROJECT_ROOT/src-gen/$PROJECT_NAME/include/core/platform/"
cp "$PROJECT_ROOT/src/platform/platform.h"              "$PROJECT_ROOT/src-gen/$PROJECT_NAME/include/core/"
cp "$PROJECT_ROOT/src/platform/lf_patmos_support.c"     "$PROJECT_ROOT/src-gen/$PROJECT_NAME/core/platform/"
cp "$PROJECT_ROOT/src/platform/lf_atomic_patmos.c"      "$PROJECT_ROOT/src-gen/$PROJECT_NAME/core/platform/"
cp "$PROJECT_ROOT/src/platform/lf_patmos_support.h"     "$PROJECT_ROOT/include/core/platform/"
cp "$PROJECT_ROOT/src/platform/platform.h"              "$PROJECT_ROOT/include/core"
cp "$PROJECT_ROOT/src/Makefile"                         "$PROJECT_ROOT/src-gen/$PROJECT_NAME/" 
