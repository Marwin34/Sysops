./catcher "$1" &
PID=$!
sleep 1
./sender "$PID" 500 "$1"