FROM debian:bullseye

RUN apt-get update && \
	export DEBIAN_FRONTEND=noninteractive && \
	apt-get install -y \
	build-essential \
	libcap-dev \
	libtclap-dev \
	libseccomp-dev \
	libseccomp2 \
	cmake \
	g++-multilib \
	gcc-multilib \
	wget \
	devscripts \
	lintian \
	debhelper \
	ccache \
	fakeroot

ENV LIBRARY_PATH=/usr/lib/x86_64-linux-gnu
ENV DEB_BUILD_OPTIONS "lang=en-US ccache nocheck"
WORKDIR /app
CMD ["/bin/bash"]