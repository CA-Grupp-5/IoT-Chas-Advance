FROM python:3.11-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    curl \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --no-cache-dir platformio

WORKDIR /workspace

COPY . /workspace

# copy custom shell script that creates the secrets if it's not there
# run script

# entry point to the copied shell script

CMD ["pio", "--help"]
