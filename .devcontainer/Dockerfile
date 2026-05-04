#############################################################################################################
# HOW TO BUILD AND PUSH THIS CONTAINER
#
# --- BASIC BUILD (using default tool versions from this Dockerfile) ---
# Build the container with a specific version tag and also tag it as "latest":
#
#   docker build \
#       -t USER_NAME/CONTAINER_NAME:VERSION_NUM \
#       -t USER_NAME/CONTAINER_NAME:latest \
#       .
#
# Example:
#   docker build \
#       -t niwciu/stm32_dev_container:1.0.0 \
#       -t niwciu/stm32_dev_container:latest \
#       .
#
#
# --- PUSH TO REGISTRY ---
# Push both the version tag and the latest tag:
#
#   docker push USER_NAME/CONTAINER_NAME:VERSION_NUM
#   docker push USER_NAME/CONTAINER_NAME:latest
#
# Example:
#   docker push niwciu/stm32_dev_container:1.0.0
#   docker push niwciu/stm32_dev_container:latest
#
#
# --- ADVANCED BUILD (override tool versions using build arguments) ---
# This Dockerfile allows overriding ALL tool versions at build time
# without modifying the file itself.
#
# Each tool version is configurable using:
#
#   --build-arg VAR_NAME=value
#
# Example – build with custom versions:
#
#   docker build \
#       --build-arg ARM_TOOLCHAIN_VERSION=14.0 \
#       --build-arg CPPCHECK_VERSION=2.17.0 \
#       --build-arg DOXYGEN_VERSION=1.14.0 \
#       --build-arg LIZARD_VERSION=1.22.0 \
#       --build-arg GCOVR_VERSION=7.3 \
#       --build-arg JINJA_VERSION=3.1.4 \
#       -t niwciu/stm32_dev_container:2.0.0 \
#       -t niwciu/stm32_dev_container:latest \
#       .
#
# If no build-args are specified, the default versions from the Dockerfile are used.
#
#############################################################################################################

FROM ubuntu:latest

# ----------------------------------------------------------------------------------------------------------------------
# Global configuration (tool versions)
# ----------------------------------------------------------------------------------------------------------------------
ARG ARM_TOOLCHAIN_VERSION=13.2
ARG CPPCHECK_VERSION=2.16.2
ARG DOXYGEN_VERSION=1.13.2
ARG LIZARD_VERSION=1.21.0
ARG GCOVR_VERSION=7.2
ARG JINJA_VERSION=3.1.3

# Computed names based on version args
ARG ARM_TOOLCHAIN_FILE=arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}.rel1-x86_64-arm-none-eabi.tar.xz
ARG ARM_TOOLCHAIN_DIRECTORY=arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}.rel1-x86_64-arm-none-eabi
ARG DOXYGEN_FILE=doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz
ARG DOXYGEN_DIRECTORY=doxygen-${DOXYGEN_VERSION}
ARG TOOLS_DIRECTORY=/opt/tools

# ----------------------------------------------------------------------------------------------------------------------
# Install system dependencies
# ----------------------------------------------------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    cmake \
    g++ \
    git \
    make \
    ninja-build \
    wget \
    xz-utils \
    python3 \
    python3-pip \
    python3-venv \
    clang-format \
    stlink-tools \
    graphviz \
    flex \
    bison \
    libclang-dev \
 && rm -rf /var/lib/apt/lists/*

# ----------------------------------------------------------------------------------------------------------------------
# Install Python CLI tools globally
# ----------------------------------------------------------------------------------------------------------------------
RUN pip install --no-cache-dir --break-system-packages \
        lizard==${LIZARD_VERSION} \
        jinja2==${JINJA_VERSION} \
        gcovr==${GCOVR_VERSION}

# ----------------------------------------------------------------------------------------------------------------------
# Install ARM GNU Toolchain
# ----------------------------------------------------------------------------------------------------------------------
RUN mkdir -p ${TOOLS_DIRECTORY} && \
    wget -q https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_TOOLCHAIN_VERSION}.rel1/binrel/${ARM_TOOLCHAIN_FILE} && \
    tar -xf ${ARM_TOOLCHAIN_FILE} -C ${TOOLS_DIRECTORY} && \
    rm ${ARM_TOOLCHAIN_FILE}

ENV PATH="${PATH}:${TOOLS_DIRECTORY}/${ARM_TOOLCHAIN_DIRECTORY}/bin"

# ----------------------------------------------------------------------------------------------------------------------
# Build cppcheck from source
# ----------------------------------------------------------------------------------------------------------------------
RUN git clone https://github.com/danmar/cppcheck.git /tmp/cppcheck && \
    cd /tmp/cppcheck && git checkout ${CPPCHECK_VERSION} && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_MATCHCOMPILER=ON -G Ninja && \
    cmake --build build && \
    cmake --install build && \
    rm -rf /tmp/cppcheck

# ----------------------------------------------------------------------------------------------------------------------
# Install Doxygen (precompiled binary)
# ----------------------------------------------------------------------------------------------------------------------
RUN wget -q https://www.doxygen.nl/files/${DOXYGEN_FILE} && \
    tar xf ${DOXYGEN_FILE} -C ${TOOLS_DIRECTORY} && \
    rm ${DOXYGEN_FILE}

ENV DOXYGEN_BIN_DIR=${TOOLS_DIRECTORY}/${DOXYGEN_DIRECTORY}/bin
ENV PATH="${PATH}:${DOXYGEN_BIN_DIR}"

# ----------------------------------------------------------------------------------------------------------------------
# Default shell
# ----------------------------------------------------------------------------------------------------------------------
CMD ["/bin/bash"]
