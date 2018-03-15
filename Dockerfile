################################################################################
# base system
################################################################################
FROM ubuntu:16.04 as system

ARG localbuild
RUN if [ "x$localbuild" != "x" ]; then sed -i 's#http://archive.ubuntu.com/#http://qnap.dorowu.com/#' /etc/apt/sources.list; fi

# built-in packages
RUN apt-get update \
    && apt-get install -y --no-install-recommends software-properties-common curl apache2-utils \
    && add-apt-repository ppa:fcwu-tw/apps \
    && apt-get update \
    && apt-get install -y --no-install-recommends --allow-unauthenticated \
        supervisor nginx sudo vim-tiny net-tools zenity xz-utils \
        dbus-x11 x11-utils alsa-utils \
        mesa-utils libgl1-mesa-dri \
        lxde x11vnc xvfb \
        gtk2-engines-murrine gnome-themes-standard gtk2-engines-pixbuf gtk2-engines-murrine arc-theme \
        firefox chromium-browser \
        ttf-ubuntu-font-family ttf-wqy-zenhei \
    && add-apt-repository -r ppa:fcwu-tw/apps \
    && apt-get autoclean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/*
# Additional packages require ~600MB
# libreoffice  pinta language-pack-zh-hant language-pack-gnome-zh-hant firefox-locale-zh-hant libreoffice-l10n-zh-tw

# tini for subreap                                   
ARG TINI_VERSION=v0.9.0
ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /bin/tini
RUN chmod +x /bin/tini

# ffmpeg
RUN mkdir -p /usr/local/ffmpeg \
    && curl -sSL https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-64bit-static.tar.xz | tar xJvf - -C /usr/local/ffmpeg/ --strip 1

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
FROM ubuntu:16.04 as builder

ARG localbuild
RUN if [ "x$localbuild" != "x" ]; then sed -i 's#http://archive.ubuntu.com/#http://qnap.dorowu.com/#' /etc/apt/sources.list; fi

RUN apt-get update \
    && apt-get install -y --no-install-recommends curl ca-certificates

# nodejs
RUN curl -sL https://deb.nodesource.com/setup_9.x | bash - \
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
FROM scratch
LABEL maintainer="fcwu.tw@gmail.com"

COPY --from=system / /
COPY --from=builder /src/web/dist/ /usr/local/lib/web/frontend/
COPY image /

EXPOSE 80
WORKDIR /root
ENV HOME=/home/ubuntu \
    SHELL=/bin/bash
ENTRYPOINT ["/startup.sh"]
