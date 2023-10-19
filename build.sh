#!/bin/sh

killall bot
cd src
g++ -std=c++17 mybot.cxx -o ticketbot -ldpp
./ticketbot
