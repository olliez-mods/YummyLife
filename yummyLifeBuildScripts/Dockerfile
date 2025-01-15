FROM ubuntu:latest

WORKDIR /app

# Install needed packages to build YummyLife
RUN apt-get update && \
    apt-get install -y g++ make cmake libsdl1.2-dev libglu-dev libgl-dev g++-mingw-w64-i686-win32 curl && \
    apt-get clean

# I can't stand not being able to use 'cls' when in shell
RUN echo "alias cls='clear'" >> ~/.bashrc
