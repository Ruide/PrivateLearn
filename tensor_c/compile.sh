gcc -std=c99 -I clib/include -L clib/lib train.c -ltensorflow -ltensorflow_framework
LD_LIBRARY_PATH=clib/lib ./a.out

