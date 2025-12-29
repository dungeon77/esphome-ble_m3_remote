# ESPHome BLE M3 Remote Component

Компонент для подключения BLE пультов типа M3 (с 5-6 кнопками) к ESPHome через ESP32.

## Особенности
- Поддержка 5 основных кнопок (Up, Down, Left, Right, Center)
- Опциональная поддержка кнопки Camera
- Автоматическое подключение к пульту по BLE
- Фильтры для стабильной работы кнопок

## Требования
- ESP32 с поддержкой BLE
- ESPHome версии 2023.12 или выше
- BLE пульт M3 с MAC-адресом

## Конфигурация

### Пример конфигурации ESPHome:

```yaml
esphome:
  name: ble_m3_gateway
  friendly_name: "BLE M3 Remote Gateway"

esp32:
  board: esp32-c3-devkitm-1

# Обязательный BLE трекер
esp32_ble_tracker:
  scan_parameters:
    interval: 320ms
    window: 30ms

# BLE клиент для подключения к пульту
ble_client:
  - mac_address: "XX:XX:XX:XX:XX:XX"  # MAC вашего пульта
    id: m3_ble_client
    auto_connect: true

# Наш компонент
external_components:
  - source:
      type: git
      url: https://github.com/dungeon77/esphome-ble_m3_remote
      ref: main
    components: [ ble_m3_remote ]

ble_m3_remote:
  ble_client_id: m3_ble_client
  
  button_up:
    name: "M3 Button Up"
    filters:
      - delayed_off: 200ms
      
  button_down:
    name: "M3 Button Down"
    filters:
      - delayed_off: 200ms
      
  button_left:
    name: "M3 Button Left"
    filters:
      - delayed_off: 200ms
      
  button_right:
    name: "M3 Button Right"
    filters:
      - delayed_off: 200ms
      
  button_center:
    name: "M3 Button Center"
    filters:
      - delayed_off: 200ms
      
  # Опционально
  # button_camera:
  #   name: "M3 Button Camera"
  #   filters:
  #     - delayed_off: 200ms
