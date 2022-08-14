# Check when DBUS is active and start configuring.

echo "Wait for dbus to start."
until PID=$(pgrep xfce4-session); do
    echo "Rechecking dbus"
    sleep 1
done    

echo "DBUS PID: $PID"
sleep 5

DBUS_ADDRESS=$(grep -z DBUS_SESSION_BUS_ADDRESS /proc/$PID/environ|cut -d= -f2-)
echo "DBUS Session Address: $DBUS_ADDRESS"
export DBUS_SESSION_BUS_ADDRESS
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
else
    su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/show-menu-icons -t bool -s 'false'"
fi

# Set tooltips
if [ -n "$SHOW_TOOLTIPS" ]; then
    su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/show-tooltips -t bool -s '$SHOW_TOOLTIPS'"
else
    su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/show-tooltips -t bool -s 'false'"
fi

# Set menu names
if [ -n "$SHOW_GENERIC_NAMES" ]; then
    su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/show-generic-names -t bool -s '$SHOW_GENERIC_NAMES'"
else
    su $USER -c "xfconf-query -c xfce4-panel -np /plugins/plugin-1/show-generic-names -t bool -s 'true'"
fi

# Set desktop right click menu
if [ -n "$RIGHT_CLICK_APP_MENU" ]; then
    su $USER -c "xfconf-query -c xfce4-desktop -np /desktop-menu/show -t bool -s '$RIGHT_CLICK_APP_MENU'"
else
    su $USER -c "xfconf-query -c xfce4-desktop -np /desktop-menu/show -t bool -s 'false'"
fi

# Remove desktop icons
su $USER -c "xfconf-query -c xfce4-desktop -np /desktop-icons/file-icons/show-removable -t bool -s 'false'"
su $USER -c "xfconf-query -c xfce4-desktop -np /desktop-icons/file-icons/show-home -t bool -s 'true'"
su $USER -c "xfconf-query -c xfce4-desktop -np /desktop-icons/file-icons/show-filesystem -t bool -s 'false'"
su $USER -c "xfconf-query -c xfce4-desktop -np /desktop-icons/file-icons/show-trash -t bool -s 'true'"

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
