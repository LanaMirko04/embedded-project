# Sdrumo companion app

The companion app is a **Flutter** application used to create an account, provision
WiFi onto an ESP32 CYD device over **EspTouch v2**, pair it to the account, and
configure what it shows (location, bus stops, bus lines). It talks only to the
[Sdrumo backend](../webserver) over its JWT REST API.

## 1. Install Flutter

Follow the [Flutter install guide](https://docs.flutter.dev/get-started/install)
(stable channel), then verify:

```bash
flutter doctor
```

## 2. Configure the backend URL

Set `baseUrl` in `lib/constants/api_config.dart` to your running backend:

```dart
const baseUrl = 'http://sdrumo.zagatti.me';
```

> [!NOTE]
> Use `http://10.0.2.2:5000` for the Android emulator hitting a backend on the host.
> All endpoint paths are constants in the same file.

## 3. Run

```bash
cd flutter-client
flutter pub get
flutter run          # or: flutter run -d android | -d linux | -d chrome
```

## 4. Build

```bash
flutter analyze              # static analysis
flutter build apk            # Android
flutter build linux          # Linux desktop
dart run flutter_launcher_icons   # regenerate launcher icons from assets/icon/icon.png
```

## Architecture

Two singleton services own all state; pages and dialogs are thin.

| Module | Role |
|--------|------|
| `services/auth_service.dart` | JWT access/refresh tokens in `flutter_secure_storage`; login/register/refresh; keyring-locked handling. |
| `services/api_service.dart`  | Single `Dio` client; attaches `Bearer` token; refreshes once on **401 / JWT-422** and retries. |
| `app/app.dart` (`AuthGate`)  | Gates root: unauthenticated → `LoginDialog`, authenticated → `MyHomePage`. |
| `dialogs/smartconfig_dialog.dart` | EspTouch v2 WiFi provisioning (`esp_smartconfig`); auto-fills SSID/BSSID via `iwgetid` on desktop. |

## Pairing flow

1. Provision WiFi via the SmartConfig dialog (device in setup mode).
2. Device registers with the backend and shows a **token** on screen.
3. Enter the token in the app → `POST /api/config/pair` links it to your account.
4. Configure the device; it picks up changes on its next config poll — no reboot.
