OLD_FILE=""
DBUS_FILE="/var/log/dbus.txt"
DIR=$(pwd)

# Wait until dbus is listening and delay 5 seconds for it to fully start.
until [[ $SUCCESS == "TRUE" ]]; do
    SUCCESS="FALSE"
    if test -f "$DBUS_FILE"; then SUCCESS="TRUE"; DBUS_ADDRESS=$(cat $DBUS_FILE); fi
    #sleep 1
    echo "DBUS Address: $DBUS_ADDRESS"
    export DBUS_SESSION_BUS_ADDRESS="unix:abstract=$DBUS_ADDRESS"
    export DISPLAY=:1.0
done

# Loop and check if its time to update the background
while true; do
    echo $(date)
    FILE_COUNT=$(ls *.jpg | wc -l)
    IMAGE_NUMBER=$(bc -l <<< "x=( $(date +%H) / ( 24 / $FILE_COUNT )); scale=0; x/1")
    FILE=$(ls -1 *.jpg | sed "${IMAGE_NUMBER}q;d")
    if [[ $FILE != $OLD_FILE ]]; then 
        echo "Setting new background to $FILE"
        xfconf-query -c xfce4-desktop -p /backdrop/screen0/monitorscreen/workspace0/last-image -n -t string -s $DIR/$FILE
        OLD_FILE=$FILE
    fi
    sleep 60
done