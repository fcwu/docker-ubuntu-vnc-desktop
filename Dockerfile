FROM ubuntu:12.04
MAINTAINER Doro Wu <fcwu.tw@gmail.com>

ENV DEBIAN_FRONTEND noninteractive
ENV HTTP_PROXY http://172.17.42.1:3134

# setup our Ubuntu sources (ADD breaks caching)
RUN echo "deb http://us.archive.ubuntu.com/ubuntu/ precise main\n\
deb http://us.archive.ubuntu.com/ubuntu/ precise multiverse\n\
deb http://us.archive.ubuntu.com/ubuntu/ precise universe\n\
deb http://us.archive.ubuntu.com/ubuntu/ precise restricted\n\
"> /etc/apt/sources.list

# no Upstart or DBus
# https://github.com/dotcloud/docker/issues/1724#issuecomment-26294856
RUN apt-mark hold initscripts udev plymouth mountall
RUN dpkg-divert --local --rename --add /sbin/initctl && ln -s /bin/true /sbin/initctl

RUN apt-get update

# install our "base" environment
RUN apt-get install -y --no-install-recommends openssh-server pwgen sudo vim-tiny
RUN apt-get install -y --no-install-recommends lxde
RUN apt-get install -y --no-install-recommends x11vnc xvfb
RUN apt-get install -y supervisor
RUN apt-get install -y libreoffice
RUN apt-get install -y firefox

ADD startup.sh /
ADD supervisord.conf /

# clean up after ourselves
RUN apt-get clean

EXPOSE 5900
EXPOSE 22
WORKDIR /
ENTRYPOINT ["/startup.sh"]
