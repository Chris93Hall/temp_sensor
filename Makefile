IMAGE  ?= temp-sensor-build
PORT   ?= /dev/ttyUSB0

# 'cd /' avoids snap-docker failing when the cwd is on a path it cannot stat.
DOCKER = cd / && docker
DOCKER_RUN = $(DOCKER) run --rm -v $(CURDIR):/project -w /project

.PHONY: help docker-build docker-run build upload clean

help:
	@echo "make docker-build  Build the Docker image ($(IMAGE))"
	@echo "make docker-run    Open an interactive shell in the image with the project mounted"
	@echo "make build         Compile the firmware inside Docker (output: build/esp12e/firmware.bin)"
	@echo "make upload        Flash the board via PORT=$(PORT) (needs the device passed through)"
	@echo "make clean         Remove build output (build/ and .pio/)"

docker-build:
	$(DOCKER) build -t $(IMAGE) $(CURDIR)

docker-run:
	$(DOCKER_RUN) -it $(IMAGE) bash

build:
	$(DOCKER_RUN) --user $(shell id -u):$(shell id -g) -e HOME=/tmp $(IMAGE) pio run

upload:
	$(DOCKER_RUN) --device=$(PORT) $(IMAGE) pio run -t upload --upload-port $(PORT)

clean:
	rm -rf build .pio
