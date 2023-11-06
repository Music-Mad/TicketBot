FROM ubuntu:latest AS build

# Install dependencies
RUN apt-get update && \
    apt-get upgrade -y 
#install git for cloning dpp library
RUN apt-get install git -y
#install build dependencies for dpp
RUN apt-get install -y cmake
RUN apt-get install build-essential -y
RUN apt-get install zlib1g-dev libsodium-dev -y
RUN apt-get install libssl-dev -y
#install gcc for compilation
RUN apt-get install -y gcc 

# Create directories and copy source
WORKDIR /TicketBot
RUN mkdir dep
RUN mkdir src
COPY src /TicketBot/src

#clone dpp library to /dep
WORKDIR /TicketBot/dep
RUN git clone https://github.com/brainboxdotcc/DPP
WORKDIR /TicketBot/dep/DPP
#build dpp
RUN cmake -B ./build
RUN cmake --build ./build -j6
RUN cd build; make install
#install rapidjson
RUN cd ..; cd dep
RUN git clone https://github.com/Tencent/rapidjson
RUN cd ./rapidjson
RUN git submodule update --init
#build rapidjson
RUN mkdir build && cd ./build
RUN cmake ..
RUN make install
RUN cd ../../../
#compile and run
WORKDIR /TicketBot/src
RUN g++ -std=c++17 mybot.cxx -o ticketbot -ldpp
CMD ["./ticketbot"]
