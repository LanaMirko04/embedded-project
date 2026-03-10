# How to use ESP-IDF

## Installation

To compile the firware, you need to install **ESP-IDF** following the [official guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation).

> [!TIP]
> If you're using **Arch Linux**, you can simply install it from the [AUR](https://aur.archlinux.org/packages/esp-idf).

## Commands

here are the essential commands to manage the project.

```bash
# Build project
$ idf.py build

# Upload firmware
$ idf.py flash

# Clean build files
$ idf.py clean

# Clean build files + managed components
$ idf.py fullclean

# Read serial messages
$ idf.py monitor
```
