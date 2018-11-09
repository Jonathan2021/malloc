#!/bin/sh

RED='\033[0;31;1m'
GREEN='\033[0;32;1m'
END='\e[0;1m'
STATUS=0
# Check that output is equivalent to that of the real find.



while read args ; do
    T1="$(LD_PRELOAD=$1 $args 2>&1)"
    T2=$($args 2>&1)
    if [ "$T1" = "$T2" ]; then
        printf "\n----------------------------------------------\n"
        printf "\n$GREEN$args$END\n"
        printf "\n----------------------------------------------\n"
    else
        printf "\n----------------------------------------------\n"
        printf "$RED$args$END\n"
        printf "\n----------------------------------------------\n"
        #echo "got\n$T1"
        #echo "expected\n$T2"
        STATUS=1
    fi

done <<EOF
ls
ls /
cat -e Makefile
ls -la
ls -la ..
ping a
cat Makefile
find .
find ..
tree .
tree ..
clang src/malloc.c
od libmalloc.so
factor 46
ls ..
ls ~
cat -e src/malloc.c
ls -la tests/
ls -la src/
cat src/malloc.c
less Makefile
find src
find ../..
tree src
tree ../..
factor 80
ls ../..
cat -e src/malloc.c
ls -la /
ls -la ~
cat tests/runtests
find ~/..
find ~
tree ~
tree ~/..
make all
od tests/runtests
factor 4
ls ~/..
ls ./../../..
cat -e tests/testsimple.sh
less src/malloc.c
ls -la src/malloc.c
cat Makefile
find .
find ../../../../..
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
factor 34
find /
EOF

exit $STATUS
