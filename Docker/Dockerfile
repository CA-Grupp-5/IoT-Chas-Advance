FROM alpine:latest

# Copy the data file into the container
COPY data /tmp/data

# Set working directory
WORKDIR /tmp

# Install netcat for sending data
RUN apk add --no-cache netcat-openbsd

# Default command: uses arguments passed to docker run
ENTRYPOINT ["sh", "-c", "nc -vq 0 \"$1\" \"$2\" < /tmp/data", "--"]
