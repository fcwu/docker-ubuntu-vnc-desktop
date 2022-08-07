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


#sed -i "s#/usr/share/backgrounds.*#/usr/share/backgrounds/default.png\"/>#g" /etc/skel/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-desktop.xml

USER=${USER:-root}
HOME=/root
if [ "$USER" != "root" ]; then
    echo "* enable custom user: $USER"
    useradd --create-home --shell /bin/bash --user-group --groups adm,sudo $USER
    if [ -z "$PASSWORD" ]; then
        echo "  set default password to \"ubuntu\""
        PASSWORD=ubuntu
    elif [[ "$PASSWORD" == "randomize" ]]; then
        echo "  set random default password"
        
        PASSWORD=$(cat /proc/sys/kernel/random/uuid | sed 's/[-]//g' | head -c 20)
    fi
    HOME=/home/$USER
    echo "$USER:$PASSWORD" | chpasswd
    cp -r /root/{.config,.gtkrc-2.0,.asoundrc} ${HOME}
    chown -R $USER:$USER ${HOME}
    [ -d "/dev/snd" ] && chgrp -R adm /dev/snd
fi
sed -i -e "s|%USER%|$USER|" -e "s|%HOME%|$HOME|" /etc/supervisor/conf.d/supervisord.conf

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
cp /cloud9/bashrc.default /home/$USER/.bashrc
chown -R $USER:$USER /home/$USER/.bashrc

# Gitconfig
touch /workspace/.ubuntu/gitconfig
ln -sf /workspace/.ubuntu/gitconfig /home/$USER/.gitconfig

# cloud9
cp /cloud9/bashrc.default /home/$USER/.bashrc
mkdir -p /workspace/.$USER/.standalone
mkdir -p /workspace/.$USER/.c9
USER_SETTINGS="/workspace/.$USER/user.settings"
if [ ! -f $USER_SETTINGS ]; then touch $USER_SETTINGS; fi
ln -sf $USER_SETTINGS /home/$USER/.c9/user.settings

# Symlink SSH keys
mkdir -p /workspace/.$USER/.ssh 
chmod 700 /workspace/.$USER/.ssh
ln -sf /workspace/.$USER/.ssh /home/$USER/.ssh

if [ -n "$DOMAIN" ]; then
    DOMAIN="cloud9.example.com"
fi
chown -R $USER:$USER /home/$USER /cloud9 /workspace

# Add required packages for ubuntu user (Run as user)

mkdir -p /workspace/.c9
chown $USER:$USER /workspace/.c9 
sudo -H -u $USER bash -c 'bash /cloud9/user-install.sh' 2>&1> /workspace/.c9/install.log &

chown $USER /usr/share/applications/

# Setup Backgrounds
mkdir -p /workspace/.ubuntu/dynamic-background/active/
for BACKGROUND in $(ls -1 /usr/share/backgrounds/dynamic-background/ | grep -v -e "active" -e "default" ); do
    cp -r /usr/share/backgrounds/dynamic-background/$BACKGROUND /workspace/.ubuntu/dynamic-background/
done
if [ ! "$(ls -A /workspace/.ubuntu/dynamic-background/active/ )" ]; then
    cp -r /usr/share/backgrounds/dynamic-background/default/* /workspace/.ubuntu/dynamic-background/active
fi

# Check when DBUS is active and start configuring. 
until [[ $SUCCESS == "TRUE" ]]; do
    SUCCESS="FALSE"
    if TEST=$(netstat -xeW | grep dbus 2>&1); then
        sleep 5
        SUCCESS="TRUE"
        DBUS_ADDRESS=$(netstat -xeW | grep dbus | head -n 1 | awk '{print $(NF)}' | sed "s/@//g")
        echo "DBUS Session Address: $DBUS_ADDRESS"
        echo "$DBUS_ADDRESS" > /var/log/dbus.txt
        export DBUS_SESSION_BUS_ADDRESS="unix:abstract=$DBUS_ADDRESS"
        export DISPLAY=:1.0

        # Hide Lower Panel, Temp Removal
        su $USER -c "xfconf-query -c xfce4-panel -p /panels -t int -s 1 -a"

        # Set branding
        if [ -n "$MENU_NAME" ]; then
            su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/button-title -t string -s '$MENU_NAME'"
        fi
        if [ -n "$MENU_ICON" ]; then
            su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/button-icon -t string -s '$MENU_ICON'"
        fi
        
        # Set menu icons
        if [ -n "$SHOW_MENU_ICONS" ]; then
            su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/show-menu-icons -t bool -s '$SHOW_MENU_ICONS'"
        fi

        # Set tooltips
        if [ -n "$SHOW_TOOLTIPS" ]; then
            su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/show-tooltips -t bool -s '$SHOW_TOOLTIPS'"
        fi

        # Set menu icons
        if [ -n "$SHOW_GENERIC_NAMES" ]; then
            su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/show-generic-names -t bool -s '$SHOW_GENERIC_NAMES'"
        fi

        # Remove plugins
        su $USER -c "
            # Copy Existing Array
            ARRAY=();
            for ID in \$(xfconf-query -c xfce4-panel -p /panels/panel-1/plugin-ids | tail -n +3); do ARRAY+=( \$ID ); done;

            # For loops to remove item from array
            for PLUGIN_NAME in actions pager; do
                PLUGIN=\$(xfconf-query -c xfce4-panel -p /plugins -lv | grep \$PLUGIN_NAME | awk '{print \$1}')
                DELETE=\$(echo \$PLUGIN | sed 's/.*plugin-//g')
                ARRAY=( \${ARRAY[@]/\$DELETE} )
            done;

            # Update Panel Plugin Array
            UPDATE_PANEL_PLUGINS='xfconf-query -c xfce4-panel -p /panels/panel-1/plugin-ids'
            for INT in \${ARRAY[@]}; do
                UPDATE_PANEL_PLUGINS+=\" -t int -s \$INT\"
            done
            \$UPDATE_PANEL_PLUGINS
        "
        # Reload Panel
        su $USER -c "xfce4-panel -r"
    fi
    sleep 1
done &

DOCKER_CREDS=/workspace/.ubuntu/docker_creds
if test -f "$DOCKER_CREDS"; then
    echo "DOCKER_CREDS exists."
    su $USER -c "DOCKER_USER=$(cat $DOCKER_CREDS | head -n1); DOCKER_PASS=$(cat $DOCKER_CREDS | tail -n1); echo \$DOCKER_PASS | docker login --username \$DOCKER_USER --password-stdin)"
fi

exec /bin/tini -- supervisord -n -c /etc/supervisor/supervisord.conf
