#!/bin/sh

make client
./client 1 5 5 &
./client 2 10 5 &
./client 3 2 10 & 
./client 4 50 20 & 
./client 5 5 2 &
./client 6 1 1 & 
./client 7 10 10 &
./client 8 2 3
./client 9 3 3 & 
./client 10 50 50
