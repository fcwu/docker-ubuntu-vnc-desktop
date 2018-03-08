.PHONY: build run

REPO  ?= dorowu/ubuntu-desktop-lxde-vnc
TAG   ?= latest

build:
	docker build -t $(REPO):$(TAG) .

run:
	docker run -it --rm -p 6080:80 \
		--name ubuntu-desktop-lxde-test \
		$(REPO):$(TAG)

shell:
	docker exec -it ubuntu-desktop-lxde-test bash
