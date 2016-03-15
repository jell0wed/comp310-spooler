#!/bin/sh

make clean
make

./server > logs &
