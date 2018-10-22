.PHONY: build run

REPO  ?= dorowu/ubuntu-desktop-lxde-vnc
TAG   ?= latest
IMAGE ?= ubuntu:18.04
LOCALBUILD ?= 1

build:
	docker build -t $(REPO):$(TAG) --build-arg localbuild=$(LOCALBUILD) --build-arg image=$(IMAGE) .

run:
	docker run --rm \
		-p 6080:80 -p 6081:443 \
		-v ${PWD}:/src:ro \
		-e HTTP_PASSWORD=123456 \
		--name ubuntu-desktop-lxde-test \
		$(REPO):$(TAG)

shell:
	docker exec -it ubuntu-desktop-lxde-test bash

gen-ssl:
	mkdir -p ssl
	openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
		-keyout ssl/nginx.key -out ssl/nginx.crt
