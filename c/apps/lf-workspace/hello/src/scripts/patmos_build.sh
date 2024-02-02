
PROJECT_ROOT="/home/ehsan/t-crest/patmos/c/apps/lf-workspace/hello/"
PROJECT_NAME="HelloWorld"
cp $PROJECT_ROOT"src/platform/lf_patmos_support.h" $PROJECT_ROOT"src-gen/HelloWorld/include/core/platform/"
cp $PROJECT_ROOT"src/platform/platform.h"          $PROJECT_ROOT"src-gen/HelloWorld/include/core/"
cp $PROJECT_ROOT"src/platform/lf_patmos_support.c" $PROJECT_ROOT"src-gen/HelloWorld/core/platform/"
cp $PROJECT_ROOT"src/platform/lf_patmos_support.h" $PROJECT_ROOT"include/core/platform/"
cp $PROJECT_ROOT"src/platform/platform.h"          $PROJECT_ROOT"include/core"
cp $PROJECT_ROOT"src/Makefile"                     $PROJECT_ROOT"src-gen/HelloWorld/"
# make app APP=lf-workspace/hello/src-gen/HelloWorld comp config download
