# Hardware Info
@melvin Can you update this?

* gpio16 - haptic
* gpio15 - UNUSED
* gpio14 - NEOPIXEL
* gpio13 - accel interrupt 1
* gpio12 - 1 wire
* gpio5 - SCL 
* gpio4 - SDA
* gpio2 - gpio expander interrupt
* gpio0 - BOOT Button

## Display

ssd1306 

## iButton
On 1wire....

* iButton needs to be touching badge in order for code to interact with it.
* need to scan in order to get the serial number of the iButton that is being used before reading/writing from/to it.
  * so normally it basically goes scan the interface to find all present devices
  * the devices will return their serial numbers
  * the problem is that every button is different, so you scan first, then talk to it
  * there is a generic query that you can use where you dont have to know the serial number first
* obviously there will probably be some menu item or something or some game prompt that will tell people to insert their ibutton
* so you just turn on/off the task as needed

About the search algo:
* the search algorithm is really convoluted
* apparently the official library that i linked above does a really good job implementing it compared to a lot of other libraries
* https://github.com/PaulStoffregen/OneWire/blob/master/examples/DS18x20_Temperature/DS18x20_Temperature.ino

1wire is a needy little bitch
* it might also be worth trying to turn off all the wifi and everything else when implementing the onewire stuff to make sure its not a timing issue
* i know a lot of people were saying that the timing of the onewire shenanigans was pretty tight, and sometimes the wifi interrupts will flake out the timing
* they had solutions to the problem, but at least to get it working it might be better to just have everything off
* https://bbs.espressif.com/viewtopic.php?t=9553

References
* https://www.analog.com/media/en/technical-documentation/data-sheets/DS1971.pdf
* https://www.analog.com/media/en/technical-documentation/data-sheets/DS2430A.pdf
* https://owfs.org/index_php_page_family-code-list.html
* https://www.analog.com/en/products/ds1971.html
* https://github.com/flipperdevices/flipperzero-firmware/blob/dev/lib/ibutton/protocols/dallas/protocol_ds1971.c
* https://github.com/PaulStoffregen/OneWire - Arduino library works.

## Accelerometer

**lis2dw12** https://www.st.com/en/mems-and-sensors/lis2dw12.html
