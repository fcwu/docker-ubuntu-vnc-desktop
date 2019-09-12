# Built with arch: amd64 flavor: lxde image: ubuntu:18.04 localbuild: 1
#
################################################################################
# base system
################################################################################

FROM ubuntu:bionic as system

# built-in packages
ENV DEBIAN_FRONTEND noninteractive
RUN apt update \
    && apt install -y --no-install-recommends software-properties-common curl apache2-utils \
    && apt update \
    && apt install -y --no-install-recommends --allow-unauthenticated \
    supervisor nginx sudo net-tools zenity xz-utils \
    dbus-x11 x11-utils alsa-utils \
    mesa-utils libgl1-mesa-dri \
    && apt autoclean -y \
    && apt autoremove -y \
    && rm -rf /var/lib/apt/lists/*
# install debs error if combine together
RUN add-apt-repository -y ppa:fcwu-tw/apps \
    && apt update \
    && apt install -y --no-install-recommends --allow-unauthenticated \
    xvfb x11vnc=0.9.16-1 \
    vim-tiny firefox chromium-browser ttf-ubuntu-font-family ttf-wqy-zenhei  \
    && add-apt-repository -r ppa:fcwu-tw/apps \
    && apt autoclean -y \
    && apt autoremove -y \
    && rm -rf /var/lib/apt/lists/*

RUN apt update \
    && apt install -y --no-install-recommends --allow-unauthenticated \
    lxde gtk2-engines-murrine gnome-themes-standard gtk2-engines-pixbuf gtk2-engines-murrine arc-theme \
    && apt autoclean -y \
    && apt autoremove -y \
    && rm -rf /var/lib/apt/lists/*


# Additional packages require ~600MB
# libreoffice  pinta language-pack-zh-hant language-pack-gnome-zh-hant firefox-locale-zh-hant libreoffice-l10n-zh-tw

# tini for subreap
ARG TINI_VERSION=v0.18.0
ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /bin/tini
RUN chmod +x /bin/tini

# ffmpeg
# RUN apt update \
#     && apt install -y --no-install-recommends --allow-unauthenticated \
#         ffmpeg \
#     && rm -rf /var/lib/apt/lists/* \
#     && mkdir /usr/local/ffmpeg \
#     && ln -s /usr/bin/ffmpeg /usr/local/ffmpeg/ffmpeg

# python library
COPY image/usr/local/lib/web/backend/requirements.txt /tmp/
RUN apt-get update \
    && dpkg-query -W -f='${Package}\n' > /tmp/a.txt \
    && apt-get install -y python-pip python-dev build-essential \
    && pip install setuptools wheel && pip install -r /tmp/requirements.txt \
    && dpkg-query -W -f='${Package}\n' > /tmp/b.txt \
    && apt-get remove -y `diff --changed-group-format='%>' --unchanged-group-format='' /tmp/a.txt /tmp/b.txt | xargs` \
    && apt-get autoclean -y \
    && apt-get autoremove -y \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /var/cache/apt/* /tmp/a.txt /tmp/b.txt


################################################################################
# builder
################################################################################
FROM ubuntu:18.04 as builder


RUN sed -i 's#http://archive.ubuntu.com/#http://tw.archive.ubuntu.com/#' /etc/apt/sources.list;


RUN apt-get update \
    && apt-get install -y --no-install-recommends curl ca-certificates gnupg patch

# nodejs
RUN curl -sL https://deb.nodesource.com/setup_8.x | bash - \
    && apt-get install -y nodejs

# yarn
RUN curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add - \
    && echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list \
    && apt-get update \
    && apt-get install -y yarn

# build frontend
COPY web /src/web
RUN cd /src/web \
    && yarn \
    && npm run build



################################################################################
# merge
################################################################################
FROM system
# LABEL maintainer="fcwu.tw@gmail.com"

COPY --from=builder /src/web/dist/ /usr/local/lib/web/frontend/

# Install Paraview


RUN apt update \ 
    && apt upgrade -y \
    && apt install build-essential -y

#old dependencies for STKO:
#RUN apt update \
#    && apt install -y \
#    python3-dev \
#    tcl8.6-dev \
#    tk8.6-dev \ 
#    libtogl-dev \
#    libglu1-mesa-dev \
#    freeglut3-dev \
#    mesa-common-dev \
#    mesa-utils \
#    libxi-dev \
#    libxmu-dev \
#    xterm 

RUN apt-get update && apt-get install -y paraview xterm
      #### Old Paraview installation packages:
      #  wget bzip2 nodejs nodejs-legacy curl gnupg gnupg2 gnupg1 && \
      #  wget -q http://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh && \
      #  bash Miniconda2-latest-Linux-x86_64.sh -p /miniconda -b && \
      #  rm Miniconda2-latest-Linux-x86_64.sh && \
      #  rm -rf /var/lib/apt/lists/* && \
      #  apt-get purge -y wget && \
      #  conda install paraview -c bioconda -c conda-forge -y
RUN curl -sL https://deb.nodesource.com/setup_8.x | bash
RUN apt-get install -y nodejs protobuf-compiler
RUN npm install -g pvw-visualizer
# RUN echo $CONDA/lib/paraview-5.2/ > /etc/ld.so.conf.d/paraview.conf && \
RUN ldconfig && \
    mkdir /usr/local/opt/ && \
    mkdir /Applications
# RUN ldconfig /usr/local/lib /miniconda/lib/paraview-5.2

RUN apt update \
    && apt install -y \
    libxkbcommon-x11-0

# Add paraview executable
# ADD Paraview.sh /Paraview.sh
ADD docker-paraviewweb/startup.sh startup.sh
# Use/overwrite script so that it uses qt libs that will be installed by online installer
# COPY Paraview.sh /Paraview-Install/Paraview.sh

## Install qt 5.12.4 using online installer
#COPY qt_install_utils/ /qt_temp
#ADD http://download.qt.io/official_releases/qt/5.12/5.12.4/qt-opensource-linux-x64-5.12.4.run /qt_temp/qt-opensource-linux-x64-5.12.4.run
#RUN chmod +x /qt_temp/qt-opensource-linux-x64-5.12.4.run
#RUN /qt_temp/qt-opensource-linux-x64-5.12.4.run --script /qt_temp/qt-installer.qs -platform minimal
#RUN rm -rf /qt_temp

COPY image /
EXPOSE 6080
WORKDIR /root
ENV HOME=/home/ubuntu \
    SHELL=/bin/bash

RUN groupadd --gid 816877 G-816877
RUN useradd --uid 458981 --create-home --shell /bin/bash --user-group --groups G-816877,adm,sudo ubuntu && chown ubuntu:G-816877 /home/ubuntu
RUN usermod -g G-816877 ubuntu
RUN mkdir -p /home/ubuntu/.config/pcmanfm/LXDE/ && cp /usr/local/share/doro-lxde-wallpapers/desktop-items-0.conf /home/ubuntu/.config/pcmanfm/LXDE/ && chown -R ubuntu:G-816877 /home/ubuntu/.config
COPY kill.py /
# COPY /etc/pki/tls/certs/designsafe-exec-01.tacc.utexas.edu.cer /etc/nginx/ssl/
# COPY /etc/pki/tls/private/designsafe-exec-01.tacc.utexas.edu.key /etc/nginx/ssl/
# HEALTHCHECK --interval=30s --timeout=5s CMD curl --fail http://127.0.0.1:6079/api/health
ENTRYPOINT ["/startup.sh"]
