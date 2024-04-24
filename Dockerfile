# Base image decision passed from the shell script
ARG BASE_IMAGE
FROM $BASE_IMAGE

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libopencv-dev \
    openmpi-bin \
    openmpi-common \
    libopenmpi-dev \
    g++ \
    make \
    && if [ "${USE_CUDA:-0}" = "1" ]; then \
    apt-get install -y cuda-command-line-tools-12-4; \
    fi \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ENV DEBIAN_FRONTEND=

ENV OMPI_MCA_btl_vader_single_copy_mechanism=none

WORKDIR /app

RUN groupadd -r myuser && useradd --no-log-init -r -g myuser myuser

RUN chown -R myuser:myuser /app

USER myuser

COPY --chown=myuser:myuser . /app

ARG USE_CUDA
ENV USE_GPU=${USE_CUDA}
RUN make all