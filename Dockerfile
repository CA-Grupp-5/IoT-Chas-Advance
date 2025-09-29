FROM python:3.11

RUN apt-get update && apt-get install -y \
    git \
    curl \
    gcc \
    g++ \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --upgrade pip \
    && pip install platformio

WORKDIR /workspace

COPY . /workspace

CMD ["pio", "--help"]
