#!/bin/sh
#

if [ ! -f Dockerfile ]; then
	echo "Dockerfile not found!"
	echo "Are you in the right directory?"
	echo "Run this script from the repo's root directory like this:"
	echo "    scripts/rebuild_esp_docker.sh"
	exit
fi

CONTAINER_ID=`docker ps -a|grep esp32-dev|awk '{print $1}'`
if [ -n "$CONTAINER_ID" ]; then
	echo "\n[+] Stopping container"
	docker stop $CONTAINER_ID
	echo "\n[+] Removing Container"
	docker rm $CONTAINER_ID
	echo "\n[+] Removing container image"
	docker rmi esp32-dev
fi

echo "\n[+] Building new container image"
docker build -t esp32-dev .

echo "\n[+] Running new container"
if ls -l /dev/ttyACM0 2> /dev/null | grep -q "ttyACM0"; then
	echo "Found /dev/ttyACM0, using it for the container"
	DEVICE="/dev/ttyACM0"
elif ls -l /dev/ttyUSB0 2> /dev/null | grep -q "ttyUSB0"; then
	echo "Found /dev/ttyUSB0, using it for the container"
	DEVICE="/dev/ttyUSB0"
else
	echo "No serial device found at /dev/ttyACM0 or /dev/ttyUSB0. Exiting."
	exit 1
fi

docker run -dit --name esp32-dev --device="$DEVICE" -v "$(pwd)":/code/cacka esp32-dev

echo "\nCreate a shell in the new container with the following command:"
echo "docker exec -it esp32-dev bash"