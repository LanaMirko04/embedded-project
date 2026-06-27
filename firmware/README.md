# Firmware

ESP32 CYD firmware built with ESP-IDF. The device displays clock, real-time bus arrivals (with stop selection, color-coded indicators, and time-to-arrival), and weather data fetched from the Sdrumo backend. Screens are touch-navigable via LVGL.

## How to use ESP-IDF

### Installation

To compile the firmware, you need to install **ESP-IDF** following the [official guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation).

> [!TIP]
> If you're using **Arch Linux**, you can simply install it from the [AUR](https://aur.archlinux.org/packages/esp-idf).

### WiFi Configuration

WiFi credentials are provisioned via **EspTouch V2** using the companion app. On first boot (or after flash erase), the device waits for SmartConfig.

1. Open the **companion app** on your phone
2. Connect phone to the 2.4 GHz WiFi network you want the device to join
3. Use the WiFi setup screen in the companion app; enter password and start provisioning
4. Device connects, stores credentials in NVS, and boots into normal operation

> [!NOTE]
> EspTouch V2 uses TCP which is more reliable than V1 (UDP broadcast).

### First Boot Pairing

On first boot, the device registers with the Sdrumo backend and displays a token on screen. Open the companion app, navigate to the pairing screen, and enter the token to link the device to your account. Once paired, the device transitions automatically to the main clock screen.

> [!NOTE]
> If you erase NVS, the device will register again and display a new token.

### Commands

Essential commands to manage the project.

```bash
# Erase flash (run before first flash or to reset device state)
$ idf.py erase-flash

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
