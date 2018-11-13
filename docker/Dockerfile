from alpine:3.8

RUN apk update && \
    apk add --virtual build-dependencies \
            build-base gdb bison git

RUN echo "alias ll='ls -alF'" > /etc/profile.d/myalias.sh

CMD ["/bin/sh", "-l"]
