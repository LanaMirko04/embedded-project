#import "@preview/cetz:0.5.1": canvas, draw

#let slide(body) = { body; pagebreak() }

#let INK  = rgb("#111827")
#let DIM  = rgb("#9ca3af")
#let SURF = rgb("#f3f4f6")
#let ACC  = rgb("#1e40af")

#set page(paper: "presentation-16-9", fill: white, margin: (x: 2cm, y: 1.4cm))
#set text(font: "Adwaita Sans", fill: INK, size: 13pt)
#set list(marker: text(fill: DIM)[–], indent: .35cm, spacing: .22cm)
#show heading.where(level: 1): it => {
  text(fill: ACC, size: 18pt, weight: "bold", it.body)
  v(.06cm)
  line(length: 100%, stroke: (paint: ACC, thickness: 1.2pt))
  v(.2cm)
}
#show raw.where(block: true): it => block(
  fill: SURF, radius: 3pt, inset: (x: 10pt, y: 7pt), width: 100%,
  text(size: 10.5pt, font: "JetBrainsMono NF", fill: INK, it),
)
#show raw.where(block: false): it => box(
  fill: SURF, radius: 2pt, inset: (x: 4pt, y: 1pt),
  text(size: 11.5pt, font: "JetBrainsMono NF", fill: ACC, it),
)

#let S = (stroke: 0.7pt + INK, fill: white, mark: (fill: INK, stroke: none, size: 0.3))

#slide[
  #set align(center + horizon)
  #text(size: 52pt, weight: "black", fill: ACC)[Sdrumo]
  #line(length: 38%, stroke: (paint: DIM, thickness: 1pt))
  #v(.5cm)
  #text(size: 16pt, fill: DIM)[Embedded Software for the Internet of Things]
  #v(.9cm)
  #text(size: 13pt, fill: DIM)[Giulia Cristofolini · Mattia Zagatti · Mirko Lana]
]

#slide[
  = Architecture Overview

  #v(2.3cm)
  #align(center)[
    #canvas(length: 2cm, {
      import draw: *
      set-style(..S)

      let bw = 2.8; let bh = 1.0; let hg = 1.5
      let x0 = 0.0; let x1 = x0 + bw + hg; let x2 = x1 + bw + hg
      let yf = 0.0 - bh - 1.4

      rect((x0, 0.0), (x0 + bw, bh), name: "esp",     radius: 0.1)
      rect((x1, 0.0), (x1 + bw, bh), name: "flask",   radius: 0.1)
      rect((x2, 0.0), (x2 + bw, bh), name: "tt",      radius: 0.1)
      rect((x1, yf),  (x1 + bw, yf + bh), name: "flutter", radius: 0.1)

      content("esp.center",     align(center, text(size: 10pt)[*ESP32 CYD*\ #text(8.5pt, font: "JetBrainsMono NF")[ESP-IDF + LVGL + C/C++]]))
      content("flask.center",   align(center, text(size: 10pt)[*Flask backend*\ #text(8.5pt, font: "JetBrainsMono NF")[Python + SQLite]]))
      content("tt.center",      align(center, text(size: 10pt)[*External APIs*\ #text(8.5pt)[Trentino Trasporti + OpenWeather]]))
      content("flutter.center", align(center, text(size: 10pt)[*Flutter App*\ #text(8.5pt, font: "JetBrainsMono NF")[Dart + Flutter]]))

      line("esp.east",    "flask.west",    mark: (start: ">", end: ">"))
      line("flask.east",  "tt.west",       mark: (end: ">"))
      line("flask.south", "flutter.north", mark: (start: ">", end: ">"))

      content(("esp.east", 50%, "flask.west"),       text(size: 9pt, fill: DIM)[HTTP/JSON], anchor: "south", padding: 0.14)
      content(("flask.east", 50%, "tt.west"),         text(size: 9pt, fill: DIM)[HTTPS],     anchor: "south", padding: 0.14)
      content(("flask.south", 50%, "flutter.north"),  text(size: 9pt, fill: DIM)[REST/JWT],  anchor: "west",  padding: 0.18)

      let es_cx = x0 + bw / 2
      set-style(stroke: (paint: DIM, thickness: 0.5pt, dash: "dashed"), mark: (fill: DIM, stroke: none, size: 0.22))
      line("flutter.west", "esp.south", mark: (end: ">"))
      content(((es_cx + x1 + bw / 2) / 2 - 0.2, yf + bh / 2 - 0.42),
        text(size: 8pt, fill: DIM)[EspTouch v2 (TCP)])
    })
  ]
  #v(.4cm)
]

#slide[
  = Software Architecture

  #grid(columns: (3fr, 2fr), gutter: 1.4cm)[
    #align(center)[
      #canvas(length: 1cm, {
        import draw: *
        set-style(..S)

        let cx  = 2.8
        let bw  = 3.4; let bh = 0.6
        let dw  = 1.1; let dh = 0.45
        let vx  = 5.1
        let ex  = 6.9; let ehw = 1.05

        let ys  = 9.0
        let yi  = 8.0
        let yd1 = 7.0
        let yw  = 5.95
        let yd2 = 4.9
        let yc  = 3.85
        let yd3 = 2.8
        let yv  = 1.75
        let ye  = 0.8

        let bx_l = cx - bw / 2; let bx_r = cx + bw / 2
        let d_r  = cx + dw; let d_l = cx - dw
        let sts_b = ys - 0.3; let sts_t = ys + 0.3
        let ste_b = ye - 0.3; let ste_t = ye + 0.3
        let ex_l = ex - ehw; let ex_r = ex + ehw
        let ey_b = yd2 - 0.32; let ey_t = yd2 + 0.32

        rect((cx - 1.3, sts_b), (cx + 1.3, sts_t), radius: 0.28, fill: SURF, stroke: 0.7pt + INK)
        content((cx, ys), text(size: 9pt)[*START*])
        rect((cx - 1.7, ste_b), (cx + 1.7, ste_t), radius: 0.28, fill: SURF, stroke: 0.7pt + INK)
        content((cx, ye), text(size: 9pt)[main loop])

        for (y, lbl) in ((yi, [INIT]), (yw, [WIFI\_CONNECTION]), (yc, [FETCH\_CONFIG]), (yv, [UPDATE\_VIEW])) {
          rect((bx_l, y - bh/2), (bx_r, y + bh/2), radius: 0.1)
          content((cx, y), text(size: 9pt, font: "JetBrainsMono NF", fill: ACC, lbl))
        }

        let diam(y) = {
          let dt = y + dh; let db = y - dh
          line((cx, dt), (d_r, y), (cx, db), (d_l, y),
               close: true, fill: white, stroke: 0.7pt + INK,
               mark: (start: none, end: none))
          content((cx, y), text(size: 8pt)[OK?])
        }
        diam(yd1); diam(yd2); diam(yd3)

        line((cx, sts_b), (cx, yi + bh/2), mark: (end: ">"))
        line((cx, yi - bh/2), (cx, yd1 + dh), mark: (end: ">"))
        line((cx, yd1 - dh), (cx, yw + bh/2), mark: (end: ">"))
        line((cx, yw - bh/2), (cx, yd2 + dh), mark: (end: ">"))
        line((cx, yd2 - dh), (cx, yc + bh/2), mark: (end: ">"))
        line((cx, yc - bh/2), (cx, yd3 + dh), mark: (end: ">"))
        line((cx, yd3 - dh), (cx, yv + bh/2), mark: (end: ">"))
        line((cx, yv - bh/2), (cx, ste_t), mark: (end: ">"))

        for (yd, yb) in ((yd1, yw), (yd2, yc), (yd3, yv)) {
          content((cx - 0.5, (yd - dh + yb + bh/2) / 2), text(size: 7.5pt, fill: DIM)[yes])
          content((d_r + 0.1, yd + 0.14), text(size: 7.5pt, fill: DIM)[no])
        }

        for y in (yd1, yd2, yd3) {
          line((d_r, y), (vx, y), stroke: 0.5pt + DIM, mark: (start: none, end: none))
        }
        line((vx, yd1), (vx, yd3), stroke: 0.5pt + DIM, mark: (start: none, end: none))
        line((vx, yd2), (ex_l, yd2), stroke: 0.5pt + DIM, mark: (end: ">"))

        rect((ex_l, ey_b), (ex_r, ey_t), radius: 0.1, fill: SURF, stroke: 0.7pt + INK)
        content((ex, yd2), text(size: 9pt, fill: ACC)[ERROR])
        let ea = yd2 - 0.9
        line((ex, ey_b), (ex, ea), mark: (end: ">"))
        content((ex, ea - 0.08), text(size: 8.5pt, font: "JetBrainsMono NF")[esp\_restart()], anchor: "north")
      })
    ]
  ][
    *C++ singletons*
    - `Fsm`: state machine
    - `Config`: NVS persistence
    - `ApiClient`: HTTP client
    - `NetHandler`: WiFi + SNTP

    #v(.4cm)
    *FreeRTOS tasks*
    - `LVGL` port: draw UI on the screen
    - `api_poll_task`: bus/weather/config
    - `smartconfig_task`: Wi-Fi setup

    #v(.4cm)
    *LVGL screens (C)*
    - `clock`: analog hands, alarm/timer btns
    - `bus`: stop picker + trip list (≤5)
    - `weather`: today + 3-day forecast
    - `alarm`, `timer`: scrollable pickers
    - `pairing`: token display + done btn
  ]
]

#slide[
  = HW/SW Interaction & Data Flow

  #grid(columns: (1fr, 1fr), gutter: 1.6cm)[
    *WiFi events*

    #text(size: 11pt, fill: DIM)[All hardware events dispatched by ESP-IDF event loop to a single callback. SPI DMA interrupts handled by drivers; app registers only callbacks.]

    #v(.3cm)
    #canvas(length: 1.5cm, {
      import draw: *
      set-style(..S)

      let bw = 6.8; let bh = 0.65; let gap = 0.42
      let events = (
        [`STA_START`: connect or SmartConfig],
        [`STA_DISCONNECTED`: retry or SmartConfig],
        [`STA_GOT_IP`: `connected = true`],
        [`SC_GOT_SSID_PSWD`: store credentials, reconnect],
      )
      let y = 0.0; let prev = none
      for ev in events {
        rect((0.0, y), (bw, y + bh), radius: 0.1, stroke: 0.5pt + DIM, fill: SURF)
        content((bw / 2, y + bh / 2), text(size: 9pt, font: "JetBrainsMono NF", fill: INK, ev))
        if prev != none { line((bw/2, prev), (bw/2, y + bh), mark: (end: ">"), stroke: 0.6pt + INK) }
        prev = y; y = y - bh - gap
      }
    })
  ][
    *Data processing chain*

    #v(.1cm)
    #canvas(length: 2.0cm, {
      import draw: *
      set-style(..S)

      let bw = 5.8; let bh = 0.7; let gap = 0.55
      let nodes = (
        [HTTP GET + cJSON parse],
        [`BusTrip[10]`, `Weather`, `cfg_rev`],
        [Copy parsed data],
        [LVGL screen render],
      )
      let y = 0.0; let prev = none
      for nd in nodes {
        rect((0.0, y), (bw, y + bh), radius: 0.12)
        content((bw/2, y + bh/2), text(size: 9.5pt, nd))
        if prev != none { line((bw/2, prev), (bw/2, y + bh), mark: (end: ">")) }
        prev = y; y = y - bh - gap
      }
    })

    #v(.3cm)
  ]
]

#slide[
  = Representative Code

  #show raw.where(block: true): it => block(
    fill: SURF, radius: 3pt, inset: (x: 8pt, y: 6pt), width: 100%,
    text(size: 8pt, font: "JetBrainsMono NF", fill: INK, it),
  )

  #grid(columns: (1fr, 1fr), gutter: 1.2cm)[
    *WiFi Event Handler* (`net.cpp`)

    ```cpp
    void NetHandler::event_handler(
        void *arg, esp_event_base_t event_base,
        std::int32_t event_id, void *event_data) {
        static NetHandler &net = NetHandler::get_instance();
        static Config &config  = Config::get_instance();

        if (event_base == WIFI_EVENT
            && event_id == WIFI_EVENT_STA_START) {
            if (net.smartconfig_running) return;
            if (config.get_ssid()[0U] != '\0'
                && config.get_password()[0U] != '\0')
                esp_wifi_connect();
            else
                net.start_smartconfig();
            return;
        }
        if (event_base == WIFI_EVENT
            && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            xEventGroupClearBits(net.event_group,
                                 NET_CONNECTED_BIT);
            if (net.retry_count < NET_MAX_RETRY)
                { ++net.retry_count; esp_wifi_connect(); }
            else net.start_smartconfig();
            return;
        }
        if (event_base == IP_EVENT
            && event_id == IP_EVENT_STA_GOT_IP) {
            net.connected = true; net.retry_count = 0;
            xEventGroupSetBits(net.event_group,
                               NET_CONNECTED_BIT);
            if (net.came_from_smartconfig)
                net.came_from_smartconfig = false;
            return;
        }
        if (event_base == SC_EVENT
            && event_id == SC_EVENT_SCAN_DONE) return;
        if (event_base == SC_EVENT
            && event_id == SC_EVENT_FOUND_CHANNEL) return;
        if (event_base == SC_EVENT
            && event_id == SC_EVENT_GOT_SSID_PSWD) {
            auto *evt = static_cast<
                smartconfig_event_got_ssid_pswd_t *>(
                    event_data);
            char ssid[33U] = { 0 };
            char password[65U] = { 0 };
            memcpy(ssid, evt->ssid, sizeof(evt->ssid));
            memcpy(password, evt->password,
                   sizeof(evt->password));
            std::size_t sl = strnlen(ssid, sizeof(ssid)-1);
            std::size_t pl = strnlen(password,
                                     sizeof(password)-1);
            Result cr = config.set_credentials(
                std::string_view(ssid, sl),
                std::string_view(password, pl));
            if (cr != Result::SUCCESS) {
            } else if (config.store() != Result::SUCCESS) {}
            if (evt->type == SC_TYPE_ESPTOUCH_V2) {
                uint8_t rvd[RVD_DATA_SIZE] = {};
                ESP_ERROR_CHECK(
                    esp_smartconfig_get_rvd_data(
                        rvd, sizeof(rvd)));
            }
            wifi_config_t cfg = {};
            memcpy(cfg.sta.ssid, evt->ssid,
                   sizeof(cfg.sta.ssid));
            memcpy(cfg.sta.password, evt->password,
                   sizeof(cfg.sta.password));
            net.retry_count = 0;
            ESP_ERROR_CHECK(
                esp_wifi_set_config(WIFI_IF_STA, &cfg));
            ESP_ERROR_CHECK(esp_wifi_connect());
            return;
        }
        if (event_base == SC_EVENT
            && event_id == SC_EVENT_SEND_ACK_DONE) {
            xEventGroupSetBits(net.event_group,
                               NET_ESPTOUCH_DONE_BIT);
            return;
        }
    }
    ```
  ][
    *LVGL Screen Manager* (`screen_manager.c`)

    ```c
    void ui_load_screen(lv_obj_t *screen) {
        screen_clock_destroy();
        screen_bus_destroy_timer();
        screen_pairing_destroy_timer();
        lv_obj_clean(screen);

        switch (present_screen_type) {
            case SCREEN_BOOT:
                ui_load_screen_boot(screen); break;
            case SCREEN_WIFI:
                ui_load_screen_wifi(screen); break;
            case SCREEN_CLOCK:
                ui_load_screen_clock(screen);
                ui_load_arrows_btn(screen); break;
            case SCREEN_BUS:
                ui_load_screen_bus(screen);
                ui_load_arrows_btn(screen); break;
            case SCREEN_ALARM:
                ui_load_screen_alarm(screen); break;
            case SCREEN_TIMER:
                ui_load_screen_timer(screen); break;
            case SCREEN_PAIRING:
                ui_load_screen_pairing(screen); break;
            case SCREEN_WEATHER:
                ui_load_screen_weather(screen);
                ui_load_arrows_btn(screen); break;
            default:
                ui_load_screen_wifi(screen);
                ui_load_arrows_btn(screen); break;
        }
    }
    ```
  ]
]

= Testing & Conclusions

#grid(columns: (1fr, 1fr), gutter: 1.6cm)[
  *Testing*
  - `idf.py monitor`: FSM states, HTTP payloads, task logs
  - Bruno collection: testing all APIs before their implementation in the firmware
  - Physical device: checking if actual result = desired result
  - `flutter analyze` + Android/Linux run
  - `idf.py erase-flash`: full re-registration and re-pairing + Wi-Fi configuration
][
  *Result*

  Working embedded product: live busses + weather on a 2.8" touch display; zero-config WiFi pairing.

  #v(.4cm)
  *Future improvements*
  - HTTPS support
  - MQTT / WebSocket to replace config polling
  - Light-sleep + screen off during night
  - Alarm & timer backend (screens already done)
  - Buzzer for timer and alarm
]
