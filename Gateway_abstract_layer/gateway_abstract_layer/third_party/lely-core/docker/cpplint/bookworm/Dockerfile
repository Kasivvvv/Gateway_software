FROM debian:bookworm

RUN apt-get update -q \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qy --no-install-recommends \
        ca-certificates \
        git \
        python3 \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/cpplint/cpplint.git \
    && cp cpplint/cpplint.py /usr/local/bin \
    && rm -rf cpplint
