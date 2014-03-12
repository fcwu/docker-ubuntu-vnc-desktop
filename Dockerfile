FROM ubuntu:12.04
MAINTAINER Doro Wu <fcwu.tw@gmail.com>

ENV DEBIAN_FRONTEND noninteractive

# setup our Ubuntu sources (ADD breaks caching)
RUN echo "deb http://us.archive.ubuntu.com/ubuntu/ precise main\n\
deb http://us.archive.ubuntu.com/ubuntu/ precise multiverse\n\
deb http://us.archive.ubuntu.com/ubuntu/ precise universe\n\
deb http://us.archive.ubuntu.com/ubuntu/ precise restricted\n\
deb http://ppa.launchpad.net/chris-lea/node.js/ubuntu precise main\n\
"> /etc/apt/sources.list

#RUN echo "Acquire::http { Proxy \"http://172.17.42.1:3142\"; };\n\
#Acquire::http::Proxy {\n\
#    private-ppa.launchpad.net DIRECT;\n\
#    download.virtualbox.org DIRECT;\n\
#}\n\
#" > /etc/apt/apt.conf.d/90apt-cacher-ng

# no Upstart or DBus
# https://github.com/dotcloud/docker/issues/1724#issuecomment-26294856
RUN apt-mark hold initscripts udev plymouth mountall
RUN dpkg-divert --local --rename --add /sbin/initctl && ln -sf /bin/true /sbin/initctl

RUN apt-get update

# install our "base" environment
RUN apt-get install -y --no-install-recommends openssh-server pwgen sudo vim-tiny

# install tty.js
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C7917B12
RUN apt-get install -y --force-yes nodejs
ADD tty.js /tty.js/

RUN apt-get install -y --no-install-recommends lxde
RUN apt-get install -y --no-install-recommends x11vnc xvfb
RUN apt-get install -y supervisor
RUN apt-get install -y libreoffice firefox

# noVNC
RUN apt-get install -y net-tools
ADD noVNC /noVNC/

# clean up after ourselves
RUN apt-get clean
#RUN rm /etc/apt/apt.conf.d/90apt-cacher-ng

ADD startup.sh /
ADD supervisord.conf /
EXPOSE 6080
EXPOSE 5900
EXPOSE 3000
EXPOSE 22
WORKDIR /
ENTRYPOINT ["/startup.sh"]
