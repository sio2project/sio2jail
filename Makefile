CC := g++
INSTALL_PREFIX = $(HOME)/.local
CONTAINER_NAME = sio2jail-dev-container
CCACHE := $(shell ccache --version 2>/dev/null)

ifdef CCACHE
	CC := ccache
endif

define run_in_docker
	docker build -t sio2jail-dev .
	- docker run -v $(shell pwd)/ccache:/ccache -v $(shell pwd):/app -e CCACHE_DIR=/ccache --rm --name $(CONTAINER_NAME) -d -it sio2jail-dev bash
	docker exec $(CONTAINER_NAME) $(1)
	docker exec $(CONTAINER_NAME) rm -rf build
	docker stop $(CONTAINER_NAME)
endef

install: clean
	mkdir -p ccache
	mkdir -p ccache/tmp
	cmake -DCMAKE_CXX_COMPILER_LAUNCHER=$(CC) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) . -B ./build
	make -C build 
	make -C build install

deb:
	mkdir -p out
	fakeroot debuild -us -uc -b
	mv ../*.deb out/

release: install deb
	mkdir -p out
	mv $(INSTALL_PREFIX)/bin/sio2jail out/

release-docker: clean-docker
	$(call run_in_docker,make release)

install-docker: clean-docker
	$(call run_in_docker,make install)

deb-docker: clean-docker
	$(call run_in_docker,make deb)

test:
	make -C build check

clean:
	- rm -rf build
	- rm -rf out
	- rm -rf obj-*
	- rm -rf install
	- rm -rf bin
	- rm -rf debian/.debhelper
	- rm -rf debian/sio2jail
	- rm debian/files
	- rm debian/sio2jail.substvars

clean-docker:
	docker build -t sio2jail-dev .
	- docker run -v $(shell pwd):/app --rm --name $(CONTAINER_NAME) -d -it sio2jail-dev bash
	docker exec $(CONTAINER_NAME) make clean