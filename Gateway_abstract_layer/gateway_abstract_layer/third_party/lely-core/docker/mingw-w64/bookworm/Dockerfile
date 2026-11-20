FROM debian:bookworm

ARG ARCH

RUN apt-get update -q \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qy --no-install-recommends \
        apt-transport-https \
        build-essential \
        ca-certificates \
        curl \
        git \
        gnupg \
        software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# For some reason, the first invocation of apt-add-repository does not actually
# add the repository.
RUN curl --proto "=https" -L https://dl.winehq.org/wine-builds/winehq.key | tee /etc/apt/trusted.gpg.d/winehq.asc \
    && apt-add-repository -y -S 'deb https://dl.winehq.org/wine-builds/debian/ bookworm main' \
    && apt-add-repository -y -S 'deb https://dl.winehq.org/wine-builds/debian/ bookworm main' \
    && dpkg --add-architecture i386 \
    && apt-get update -q \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qy --no-install-recommends \
        libc6:i386 \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qy --no-install-recommends \
        mingw-w64 \
        winehq-stable \
    && rm -rf /var/lib/apt/lists/*

ENV WINEPATH Z:\\usr\\lib\\gcc\\${ARCH}-w64-mingw32\\12-posix\;Z:\\usr\\${ARCH}-w64-mingw32\\lib

RUN wineboot -i

RUN apt-get update -q \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qy --no-install-recommends \
        automake \
        libtool \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --branch v4.0 --depth 1 https://github.com/cpputest/cpputest.git \
    && cd cpputest/cpputest_build \
    && autoreconf .. -i \
    && ../configure --host="$ARCH-w64-mingw32" --disable-memory-leak-detection \
    && make \
    && make install \
    && cd - \
    && rm -rf cpputest
