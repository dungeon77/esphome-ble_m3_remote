#pragma once

#include "esphome.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace ble_m3_remote {

class BLEM3Remote : public Component, public ble_client::BLEClientNode {
 public:
  void setup() override;
  void dump_config() override;
  
  void set_ble_client(ble_client::BLEClient *client) { this->ble_client_ = client; }
  void set_button_up_sensor(binary_sensor::BinarySensor *sensor) { button_up_sensor_ = sensor; }
  void set_button_down_sensor(binary_sensor::BinarySensor *sensor) { button_down_sensor_ = sensor; }
  void set_button_left_sensor(binary_sensor::BinarySensor *sensor) { button_left_sensor_ = sensor; }
  void set_button_right_sensor(binary_sensor::BinarySensor *sensor) { button_right_sensor_ = sensor; }
  void set_button_center_sensor(binary_sensor::BinarySensor *sensor) { button_center_sensor_ = sensor; }
  void set_button_camera_sensor(binary_sensor::BinarySensor *sensor) { button_camera_sensor_ = sensor; }
  
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                          esp_ble_gattc_cb_param_t *param) override;

 protected:
  void process_hid_data_(const std::vector<uint8_t> &data);
  // Метод для сравнения 9-байтных массивов
  bool key_match_9byte_(const uint8_t* buf1, const std::vector<uint8_t>& vec2);
  void handle_button_press_(int button_code);
  
  // 3-байтные ключи (для обратной совместимости)
  static const std::vector<uint8_t> BUTTON_UP_KEY;
  static const std::vector<uint8_t> BUTTON_DOWN_KEY;
  static const std::vector<uint8_t> BUTTON_LEFT_KEY;
  static const std::vector<uint8_t> BUTTON_RIGHT_KEY;
  static const std::vector<uint8_t> BUTTON_CENTER_KEY;
  static const std::vector<uint8_t> BUTTON_CAMERA_KEY;
  
  // 9-байтные ключи (правильные из библиотеки)
  static const std::vector<uint8_t> BUTTON_UP_9BYTE;
  static const std::vector<uint8_t> BUTTON_DOWN_9BYTE;
  static const std::vector<uint8_t> BUTTON_LEFT_9BYTE;
  static const std::vector<uint8_t> BUTTON_RIGHT_9BYTE;
  static const std::vector<uint8_t> BUTTON_CENTER_9BYTE;
  static const std::vector<uint8_t> BUTTON_CAMERA_9BYTE;
  
 private:
  ble_client::BLEClient *ble_client_{nullptr};
  bool subscribed_{false};
  uint16_t hid_report_handle_{0};

  // Кольцевой буфер 9 байт (как в исходной библиотеке)
  uint8_t button_history_[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  
  // Для защиты от дребезга
  uint32_t last_button_press_time_{0};
  int last_pressed_button_{-1};
  
  binary_sensor::BinarySensor *button_up_sensor_{nullptr};
  binary_sensor::BinarySensor *button_down_sensor_{nullptr};
  binary_sensor::BinarySensor *button_left_sensor_{nullptr};
  binary_sensor::BinarySensor *button_right_sensor_{nullptr};
  binary_sensor::BinarySensor *button_center_sensor_{nullptr};
  binary_sensor::BinarySensor *button_camera_sensor_{nullptr};
  
  void subscribe_to_hid_notifications_();
};

}  // namespace ble_m3_remote
}  // namespace esphome