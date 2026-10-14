FROM python:3.11-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends git make \
    && rm -rf /var/lib/apt/lists/*

ENV PLATFORMIO_CORE_DIR=/opt/platformio

RUN pip install --no-cache-dir platformio

# Pre-fetch the ESP8266 toolchain and Arduino core so builds work from a warm cache.
# A throwaway project is used so this layer doesn't depend on the source tree.
WORKDIR /tmp/warmup
RUN mkdir src \
    && printf '[env:esp12e]\nplatform = espressif8266\nboard = esp12e\nframework = arduino\n' > platformio.ini \
    && printf '#include <Arduino.h>\nvoid setup(){}\nvoid loop(){}\n' > src/main.cpp \
    && pio run \
    && rm -rf /tmp/warmup \
    && chmod -R a+rwX /opt/platformio

WORKDIR /project
CMD ["bash"]
