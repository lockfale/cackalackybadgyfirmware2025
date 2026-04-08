## Clean, Build, & Flash (pre-kiosk commits)
```sh
pio run -t clean;
pio run -v ;
pio run --target buildfs --jobs 30;
esptool.py -b 921600 --chip esp8266 write_flash 0x0 .pio/build/esp_wroom_02/firmware.bin 1048576 .pio/build/esp_wroom_02/littlefs.bin
```


## Build badge
```sh
pio run -t clean
pio run -e badge
pio run -e badge --target buildfs --jobs 30
esptool.py -b 921600 --chip esp8266 write_flash 0x0 .pio/build/badge/firmware.bin 1048576 .pio/build/badge/littlefs.bin
```

### All-in-1
```sh
pio run -t clean; pio run -e badge; pio run -e badge --target buildfs --jobs 30; esptool.py -b 921600 --chip esp8266 write_flash 0x0 .pio/build/badge/firmware.bin 1048576 .pio/build/badge/littlefs.bin
```

## Build Kiosk
```sh
pio run -t clean
pio run -e kiosk
pio run -e kiosk --target buildfs --jobs 30
esptool.py -b 921600 --chip esp8266 write_flash 0x0 .pio/build/kiosk/firmware.bin
```

### All-in-1
```sh
pio run -t clean; pio run -e kiosk; pio run -e kiosk --target buildfs --jobs 30; esptool.py -b 921600 --chip esp8266 write_flash 0x0 .pio/build/kiosk/firmware.bin
```

# Monitor
```sh
pio device monitor
```

# Configure on-device networking
```txt
SU Ecruiaiergo
printnetwork
your-mqtt-server.com
setmqttport 10432
```