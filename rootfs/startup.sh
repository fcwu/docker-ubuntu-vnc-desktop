#!/bin/bash

if [ -n "$VNC_PASSWORD" ]; then
    echo -n "$VNC_PASSWORD" > /.password1
    x11vnc -storepasswd $(cat /.password1) /.password2
    chmod 400 /.password*
    sed -i 's/^command=x11vnc.*/& -rfbauth \/.password2/' /etc/supervisor/conf.d/supervisord.conf
    export VNC_PASSWORD=
fi

if [ -n "$X11VNC_ARGS" ]; then
    sed -i "s/^command=x11vnc.*/& ${X11VNC_ARGS}/" /etc/supervisor/conf.d/supervisord.conf
fi

if [ -n "$OPENBOX_ARGS" ]; then
    sed -i "s#^command=/usr/bin/openbox\$#& ${OPENBOX_ARGS}#" /etc/supervisor/conf.d/supervisord.conf
fi

if [ -n "$RESOLUTION" ]; then
    sed -i "s/1024x768/$RESOLUTION/" /usr/local/bin/xvfb.sh
fi

if [ -n "$TZ" ]; then
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
    dpkg-reconfigure --frontend noninteractive tzdata
fi

if [ -n "$FILE_SHARE" ]; then
    FILE_SHARE=_Shared_Files_
    sed -i "s/%FILE_SHARE%/_Shared_Files_/" /etc/xdg/user-dirs.defaults
else
    sed -i "s/%FILE_SHARE%/$FILE_SHARE/" /etc/xdg/user-dirs.defaults
fi

#sed -i "s#/usr/share/backgrounds.*#/usr/share/backgrounds/default.png\"/>#g" /etc/skel/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-desktop.xml

USER=${USER:-root}
HOME=/root
if [ "$USER" != "root" ]; then
    echo "* enable custom user: $USER"
    #useradd --create-home --shell /bin/bash --user-group --groups adm,sudo,docker -d /workspace/.home/$USER $USER
    useradd --create-home --shell /bin/bash --user-group --groups adm,sudo,docker $(if [ ! $PUID == "" ]; then echo "--uid $PUID"; fi) $(if [ ! $PGID == "" ]; then echo "--gid $PGID"; fi) $USER

    if [ -z "$PASSWORD" ]; then
        echo "  set default password to \"ubuntu\""
        PASSWORD=ubuntu
    elif [[ "$PASSWORD" == "randomize" ]]; then
        echo "  set random default password"

        PASSWORD=$(cat /proc/sys/kernel/random/uuid | sed 's/[-]//g' | head -c 20)
    fi
    
    #HOME=/workspace/.home/$USER
    HOME=/home/$USER
    echo "$USER:$PASSWORD" | chpasswd
    cp -r /root/{.config,.gtkrc-2.0,.asoundrc} ${HOME}
    [ -d "/dev/snd" ] && chgrp -R adm /dev/snd
fi
sed -i -e "s|%USER%|$USER|" -e "s|%HOME%|$HOME|" /etc/supervisor/conf.d/supervisord.conf

#mkdir -r /home/$USER/.config/
# Set the default file manager
#grep "FileManager" /home/$USER/.config/xfce4-helpers.rc && sed -i "/FileManager/c\FileManager=nautilus" || echo "FileManager=nautilus" >> /home/$USER/.config/xfce4-helpers.rc
# Set the default web browser if it does not exist
#grep "WebBrowser" /home/$USER/.config/xfce4-helpers.rc || echo "WebBrowser=google-chrome" >> /home/$USER/.config/xfce4-helpers.rc
#chown -R $USER:$USER /home/$USER/.config

# nginx workers
sed -i 's|worker_processes .*|worker_processes 1;|' /etc/nginx/nginx.conf

# nginx ssl
if [ -n "$SSL_PORT" ] && [ -e "/etc/nginx/ssl/nginx.key" ]; then
    echo "* enable SSL"
        sed -i 's|#_SSL_PORT_#\(.*\)443\(.*\)|\1'$SSL_PORT'\2|' /etc/nginx/sites-enabled/default
        sed -i 's|#_SSL_PORT_#||' /etc/nginx/sites-enabled/default
fi

# nginx http base authentication
if [ -n "$HTTP_PASSWORD" ]; then
    echo "* enable HTTP base authentication"
    htpasswd -bc /etc/nginx/.htpasswd $USER $HTTP_PASSWORD
        sed -i 's|#_HTTP_PASSWORD_#||' /etc/nginx/sites-enabled/default
fi

# dynamic prefix path renaming
if [ -n "$RELATIVE_URL_ROOT" ]; then
    echo "* enable RELATIVE_URL_ROOT: $RELATIVE_URL_ROOT"
        sed -i 's|#_RELATIVE_URL_ROOT_||' /etc/nginx/sites-enabled/default
        sed -i 's|_RELATIVE_URL_ROOT_|'$RELATIVE_URL_ROOT'|' /etc/nginx/sites-enabled/default
fi

# clearup
PASSWORD=
HTTP_PASSWORD=

# BashRC
cp /cloud9/bashrc.default $HOME/.bashrc
chown -R $USER:$USER $HOME/.bashrc

if [ -n "$DOMAIN" ]; then
    DOMAIN="cloud9.example.com"
fi

# Reduce this so its not everything
#chown -R $USER:$USER $HOME /cloud9 /workspace

# Add required packages for ubuntu user (Run as user)

#rm -rf $HOME/.c9
#mkdir -p /workspace/.c9
#chown $USER:$USER /workspace/.c9

 
chown -R $USER:$USER /home/$USER


sudo -H -u $USER bash -c 'bash /cloud9/user-install.sh' 2>&1> /home/$USER/.cloud9-install.log &

# Only for testing while editing the menu
#chown $USER /usr/share/applications/

# Setup Backgrounds
mkdir -p /home/$USER/.dynamic-background/active/
for BACKGROUND in $(ls -1 /usr/share/backgrounds/dynamic-background/ | grep -v -e "active" -e "default" ); do
    cp -r /usr/share/backgrounds/dynamic-background/$BACKGROUND /home/$USER/.dynamic-background/active/
done
if [ ! "$(ls -A /home/$USER/.dynamic-background/active/ )" ]; then
    cp -r /usr/share/backgrounds/dynamic-background/default/* /home/$USER/.dynamic-background/active/
fi

bash /cloud9/configure_desktop.sh &

mkdir -p /home/$USER/Workspace/$FILE_SHARE
grep -qxF "/home/$USER/Workspace /workspace none defaults,bind 0 0" /etc/fstab || echo "/home/$USER/Workspace /workspace none defaults,bind 0 0" >> /etc/fstab
grep -qxF "/home/$USER/Workspace/$FILE_SHARE /workspace/$FILE_SHARE none defaults,bind 0 0" /etc/fstab || echo "/home/$USER/Workspace/$FILE_SHARE /workspace/$FILE_SHARE none defaults,bind 0 0" >> /etc/fstab
mount -a

# Make directory for bookmarks
mkdir -p /home/$USER/.config/gtk-3.0

# Keep these bookmarks
#grep "Documents" /home/$USER/.config/gtk-3.0/bookmarks || echo "file:///home/$USER/Documents" >> /home/$USER/.config/gtk-3.0/bookmarks
grep "Workspace" /home/$USER/.config/gtk-3.0/bookmarks || echo "file:///home/$USER/Workspace" >> /home/$USER/.config/gtk-3.0/bookmarks
grep "$FILE_SHARE" /home/$USER/.config/gtk-3.0/bookmarks || echo "file:///home/$USER/Workspace/$FILE_SHARE" >> /home/$USER/.config/gtk-3.0/bookmarks
grep "Downloads" /home/$USER/.config/gtk-3.0/bookmarks || echo "file:///home/$USER/Downloads" >> /home/$USER/.config/gtk-3.0/bookmarks

# Duke.edu repo is down 8/18/2020
grep "127.0.0.1 archive.linux.duke.edu" /etc/hosts || echo "127.0.0.1 archive.linux.duke.edu" >> /etc/hosts

if [ -n "$HOSTNAME" ]; then
    echo "$HOSTNAME" >> /etc/hosts
fi

if [ -n "$NAMESERVER" ]; then
    echo "nameserver $NAMESERVER" > /etc/resolv.conf
else
    echo "nameserver 127.0.0.11" > /etc/resolv.conf
fi

if [ -n "$SEARCHDOMAIN" ]; then
    echo "search $SEARCHDOMAIN" >> /etc/resolv.conf
fi

chown -R $USER:$USER /home/$USER
exec /bin/tini -- supervisord -n -c /etc/supervisor/supervisord.conf
