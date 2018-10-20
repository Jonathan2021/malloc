#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
END='\e[0m'
STATUS=0
# Check that output is equivalent to that of the real find.



while read args ; do
    T1=$(LD_PRELOAD=$1 $arg 2>&1)
    T2=$($args 2>&1)
    if [ "$T1" = "$T2" ]; then
        echo "$GREEN$args$END"
    else
        echo "$RED$args$END"
        #echo "got\n$T1"
        #echo "expected\n$T2"
        STATUS=1
    fi

done <<EOF
ls
ls /
cat -e Makefile
ls -la
ls -la src/malloc.c
ping a
cat Makefile
find .
find ../../..
tree .
tree ../../..
clang src/malloc.c
od libmalloc.so
factor 46
ls
ls /
cat -e Makefile
ls -la
ls -la src/malloc.c
ping a
cat Makefile
find .
find ../../..
tree .
tree ../../..
clang src/malloc.c
od libmalloc.so
factor 46
ls
ls /
cat -e Makefile
ls -la
ls -la src/malloc.c
ping a
cat Makefile
find .
find ../../..
tree .
tree ../../..
clang src/malloc.c
od libmalloc.so
factor 46
ls
ls /
cat -e Makefile
ls -la
ls -la src/malloc.c
ping a
cat Makefile
find .
find ../../..
tree .
tree ../../..
clang src/malloc.c
od libmalloc.so
factor 46
ls
ls /
cat -e Makefile
ls -la
ls -la src/malloc.c
ping a
cat Makefile
find .
find ../../..
tree .
tree ../../..
clang src/malloc.c
od libmalloc.so
more Makefile
factor 46
EOF

exit $STATUS
