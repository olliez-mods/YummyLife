FROM ubuntu:26.04

WORKDIR /app

# Install needed packages to build YummyLife
# Keep this list in step with the 'Packages' step of
# .github/workflows/release.yml, which installs the same set on the runner.
RUN apt-get update && \
    apt-get install -y g++ make cmake libsdl1.2-dev libglu-dev libgl-dev \
    libpng-dev g++-mingw-w64-i686-win32 curl libssl-dev libcurl4-openssl-dev wget && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# I can't stand not being able to use 'cls' when in shell
RUN echo "alias cls='clear'" >> ~/.bashrc
