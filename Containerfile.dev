FROM ubuntu:latest

WORKDIR /build

COPY . /build/

RUN chmod +x /build/scripts/apt.sh && /build/scripts/apt.sh

RUN cd / && chmod +x /build/scripts/vcpkg.sh && /build/scripts/vcpkg.sh
ENV VCPKG_ROOT=/vcpkg
ENV PATH="$PATH:$VCPKG_ROOT" 

RUN cd /build && git submodule update --init --recursive

CMD [ "bash" ]