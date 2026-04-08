FROM python:3.12-slim-bookworm

# Install dependencies
RUN apt-get update && apt-get install -y \
    gcc git build-essential curl make libncurses-dev flex bison gperf python3-serial usbutils cmake \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN echo "deb http://deb.debian.org/debian sid main" | tee -a /etc/apt/sources.list  \
    && apt update  \
    && apt install clang-tidy -y \
    && apt-get clean && rm -rf /var/lib/apt/lists/*


# Set working directory
WORKDIR /code

RUN curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
RUN python3 get-platformio.py
RUN python3 -m pip install -U platformio
RUN pip install esptool

# may have to install this:
# https://docs.platformio.org/en/latest/core/installation/udev-rules.html

RUN echo "alias pio_build='pio run -v'" >> ~/.bashrc \
    && echo "alias pio_build_littlefs='pio run --target buildfs --jobs 30'" >> ~/.bashrc \
    && echo "alias pio_build_all='pio run --jobs 30 && pio run --target buildfs --jobs 30'" >> ~/.bashrc \
    && echo "alias clean_build='pio run -t clean && pio run --jobs 30 && pio run --target buildfs --jobs 30'" >> ~/.bashrc \
    && echo "alias pio_monitor='pio device monitor'" >> ~/.bashrc \
    && echo "alias pio_flash='esptool -b 921600 --chip esp8266 write-flash 0x0 .pio/build/badge/firmware.bin 1048576 .pio/build/badge/littlefs.bin'" >> ~/.bashrc \
    && echo "alias repo='cd /code/cacka'" >> ~/.bashrc

# Default command (optional, can be overridden)
CMD ["/bin/bash"]
