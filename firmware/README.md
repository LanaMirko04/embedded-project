# How to use PlatformIO with ESP-IDF framework

```shell
# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Build specific environment
$ pio run -e esp32-2432S028Rv2

# Upload firmware for the specific environment
$ pio run -e esp32-2432S028Rv2 --target upload

# Clean build files
$ pio run --target clean
```
