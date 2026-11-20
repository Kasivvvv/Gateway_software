ARG REGISTRY

FROM ${REGISTRY}/build:bookworm

RUN apt-get update -q \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qy --no-install-recommends \
        doxygen \
        graphviz \
    && rm -rf /var/lib/apt/lists/*
