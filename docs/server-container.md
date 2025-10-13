# Server container #
Uses a Docker container to simulate a request/data transfer to an socket/endpoint. Build the image first, then edit the file *data* to contain the raw text you want to send, and finally run the container with the IP/DNS and port as arguments! Success or failure is printed into the terminal.

## Building: ##
Build the image using:
```bash
docker build -t <image_name> .
```

## Running: ##
Run a container using:
```bash
docker run --rm <image_name> <IP/DNS> <port>
```