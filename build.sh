#!/bin/sh
cd src/
g++ -std=c++17 *.cxx -o ../../ticketbot -ldpp
cd ../../
./ticketbot
