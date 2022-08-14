OLD_FILE=""
DIR=$(pwd)

export DISPLAY=:1.0

echo "Wait for dbus to start."
until PID=$(pgrep xfce4-session); do
    echo "Rechecking dbus"
    sleep 1
done    

echo "DBUS PID: $PID"
sleep 5

# Loop and check if its time to update the background
while true; do
    PID=$(pgrep xfce4-session)
    DBUS_SESSION_BUS_ADDRESS=$(grep -z DBUS_SESSION_BUS_ADDRESS /proc/$PID/environ | cut -d= -f2- | tr -d '\0')
    export DBUS_SESSION_BUS_ADDRESS
    FILE_COUNT=$(ls *.jpg | wc -l)
    IMAGE_NUMBER=$(bc -l <<< "x=( $(date +%H) / ( 24 / $FILE_COUNT )); scale=0; x/1+1")
    FILE=$DIR/$(ls -1 *.jpg | sed "${IMAGE_NUMBER}q;d")
    OLD_FILE=$(xfconf-query -c xfce4-desktop -p /backdrop/screen0/monitorscreen/workspace0/last-image)
    if [[ $FILE != $OLD_FILE ]]; then 
        echo "Changing background from $OLD_FILE to $FILE"
        xfconf-query -c xfce4-desktop -p /backdrop/screen0/monitorscreen/workspace0/last-image -n -t string -s $FILE
        OLD_FILEx=$FILE
    fi
    sleep 60
done