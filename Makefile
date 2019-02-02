.PHONY: build run

REPO  ?= lxqt
TAG   ?= latest
IMAGE ?= ubuntu:18.04
LOCALBUILD ?= tw
HTTP_PASSWORD ?= 123456
CUSTOM_USER ?= ubuntu
PASSWORD ?= ubuntu

build:
	docker build -t $(REPO):$(TAG) --build-arg localbuild=$(LOCALBUILD) --build-arg image=$(IMAGE) .

run:
	docker run --rm \
		-p 6080:80 -p 6081:443 \
		-v ${PWD}:/src:ro \
		-e HTTP_PASSWORD=$(HTTP_PASSWORD) \
		-e USER=$(CUSTOM_USER) \
		-e PASSWORD=$(PASSWORD) \
		--name ubuntu-desktop-lxde-test \
		$(REPO):$(TAG)

shell:
	docker exec -it ubuntu-desktop-lxde-test bash

gen-ssl:
	mkdir -p ssl
	openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
		-keyout ssl/nginx.key -out ssl/nginx.crt
