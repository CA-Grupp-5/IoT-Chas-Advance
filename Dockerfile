FROM alpine:latest
COPY data /tmp/data
WORKDIR /tmp
RUN apk add --no-cache netcat-openbsd
# use host.docker.internal by default; override with TARGET_HOST env if needed
CMD ["sh","-c","nc -q 0 ${TARGET_HOST:-host.docker.internal} 12345 < /tmp/data"]
