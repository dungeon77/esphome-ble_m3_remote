#include "ble_m3_remote.h"
#include "esphome/core/log.h"
#include "esphome/components/ble_client/ble_client.h"

namespace esphome {
namespace ble_m3_remote {

static const char *const TAG = "ble_m3_remote";

// // 3-байтные ключи (последние 3 байта из 9-байтных кодов)
// const std::vector<uint8_t> BLEM3Remote::BUTTON_UP_KEY = {0x02, 0x30, 0x00};
// const std::vector<uint8_t> BLEM3Remote::BUTTON_DOWN_KEY = {0x02, 0xD0, 0xFF};
// const std::vector<uint8_t> BLEM3Remote::BUTTON_LEFT_KEY = {0x96, 0x00, 0x00};
// const std::vector<uint8_t> BLEM3Remote::BUTTON_RIGHT_KEY = {0x6A, 0x0F, 0x00};
// const std::vector<uint8_t> BLEM3Remote::BUTTON_CENTER_KEY = {0x00, 0x00, 0x00};
// const std::vector<uint8_t> BLEM3Remote::BUTTON_CAMERA_KEY = {0x00, 0x00, 0x00};

// ТОЧНЫЕ 9-БАЙТНЫЕ КЛЮЧИ из библиотеки:
const std::vector<uint8_t> BLEM3Remote::BUTTON_UP_9BYTE = 
    {0x3C, 0x80, 0x0C, 0x01, 0x00, 0x00, 0x02, 0x30, 0x00};        // ИЗМЕНИТЬ! [3C 80 0C 01 00 00 02 30 00]
const std::vector<uint8_t> BLEM3Remote::BUTTON_DOWN_9BYTE = 
    {0x3C, 0x40, 0xEC, 0x01, 0x00, 0x00, 0x02, 0xD0, 0xFF};       // ✓ Правильный
const std::vector<uint8_t> BLEM3Remote::BUTTON_LEFT_9BYTE = 
    {0x28, 0x80, 0x11, 0x01, 0x00, 0x00, 0x96, 0x00, 0x00};       // ✓ Правильный
const std::vector<uint8_t> BLEM3Remote::BUTTON_RIGHT_9BYTE = 
    {0xAE, 0x8F, 0x11, 0x01, 0x00, 0x00, 0x6A, 0x0F, 0x00};       // ✓ Правильный
const std::vector<uint8_t> BLEM3Remote::BUTTON_CAMERA_9BYTE = 
    {0xA0, 0x30, 0xE8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};       // ✓ Правильный
const std::vector<uint8_t> BLEM3Remote::BUTTON_CENTER_9BYTE = 
    {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};       // ✓ Правильный
    
// Таймаут между пакетами (миллисекунды)
const uint32_t PACKET_TIMEOUT_MS = 100;

void BLEM3Remote::setup() {
  ESP_LOGCONFIG(TAG, "Setting up BLE M3 Remote...");
  
  if (this->ble_client_ == nullptr) {
    ESP_LOGE(TAG, "No BLE client configured!");
    return;
  }
  
  this->ble_client_->register_ble_node(this);
}

void BLEM3Remote::dump_config() {
  ESP_LOGCONFIG(TAG, "BLE M3 Remote:");
  ESP_LOGCONFIG(TAG, "  BLE Client: %s", this->ble_client_ != nullptr ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  Subscribed: %s", this->subscribed_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  HID Report Handle: %d", this->hid_report_handle_);
  
  // Выводим 9-байтные ключи для отладки
  ESP_LOGCONFIG(TAG, "  9-Byte Button Keys:");
  ESP_LOGCONFIG(TAG, "    UP:     %s", 
    format_hex_pretty(BUTTON_UP_9BYTE).c_str());
  ESP_LOGCONFIG(TAG, "    DOWN:   %s", 
    format_hex_pretty(BUTTON_DOWN_9BYTE).c_str());
  ESP_LOGCONFIG(TAG, "    LEFT:   %s", 
    format_hex_pretty(BUTTON_LEFT_9BYTE).c_str());
  ESP_LOGCONFIG(TAG, "    RIGHT:  %s", 
    format_hex_pretty(BUTTON_RIGHT_9BYTE).c_str());
  ESP_LOGCONFIG(TAG, "    CENTER: %s", 
    format_hex_pretty(BUTTON_CENTER_9BYTE).c_str());
  ESP_LOGCONFIG(TAG, "    CAMERA: %s", 
    format_hex_pretty(BUTTON_CAMERA_9BYTE).c_str());
}

void BLEM3Remote::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                     esp_ble_gattc_cb_param_t *param) {
  ble_client::BLEClientNode::gattc_event_handler(event, gattc_if, param);
  
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "Connected to BLE device");
      } else {
        ESP_LOGE(TAG, "Failed to connect, status: %d", param->open.status);
      }
      break;
    }
    
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGI(TAG, "Service discovery complete");
      this->subscribe_to_hid_notifications_();
      break;
    }
    
    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.is_notify) {
        std::vector<uint8_t> data(param->notify.value, param->notify.value + param->notify.value_len);
        this->process_hid_data_(data);
      }
      break;
    }
    
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      ESP_LOGI(TAG, "REG_FOR_NOTIFY_EVT: handle %d, status %d",
               param->reg_for_notify.handle, param->reg_for_notify.status);
      if (param->reg_for_notify.status == ESP_GATT_OK) {
        this->subscribed_ = true;
        ESP_LOGI(TAG, "Successfully subscribed to handle %d", param->reg_for_notify.handle);
      }
      break;
    }
    
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGI(TAG, "Disconnected from BLE device");
      this->subscribed_ = false;
      this->hid_report_handle_ = 0;
    //   this->packet_buffer_.clear(); // Очищаем буфер при отключении
      break;
    }
    
    default:
      break;
  }
}

void BLEM3Remote::subscribe_to_hid_notifications_() {
  if (this->ble_client_ == nullptr) {
    return;
  }

  ESP_LOGI(TAG, "Looking for HID service...");

  auto service = this->ble_client_->get_service(esp32_ble_tracker::ESPBTUUID::from_uint16(0x1812));
  if (service == nullptr) {
    ESP_LOGE(TAG, "HID service (0x1812) not found!");
    return;
  }

  ESP_LOGI(TAG, "Found HID service (start: %d, end: %d)", service->start_handle, service->end_handle);

  // Подписываемся на все handles в диапазоне сервиса
  uint16_t target_handles[] = {19, 23, 31, 35};
  
  for (uint16_t target_handle : target_handles) {
    if (target_handle >= service->start_handle && target_handle <= service->end_handle) {
      ESP_LOGI(TAG, "Subscribing to handle %d", target_handle);
      
      esp_err_t err = esp_ble_gattc_register_for_notify(
          this->ble_client_->get_gattc_if(),
          this->ble_client_->get_remote_bda(),
          target_handle);
      
      if (err == ESP_OK) {
        ESP_LOGI(TAG, "Subscription request sent for handle %d", target_handle);
        this->hid_report_handle_ = target_handle;
      }
    }
  }
}

void BLEM3Remote::process_hid_data_(const std::vector<uint8_t> &data) {
  if (data.size() != 3) {
    ESP_LOGW(TAG, "Неожиданный размер HID-данных: %d байт", data.size());
    return;
  }

  ESP_LOGD(TAG, "RAW 3-BYTE: [%02X %02X %02X]", data[0], data[1], data[2]);

  // 1. ОБНОВЛЯЕМ КОЛЬЦЕВОЙ БУФЕР (точно как в оригинале)
  for (int i = 0; i < 6; i++) {
    button_history_[i] = button_history_[i + 3];
  }
  button_history_[6] = data[0];
  button_history_[7] = data[1];
  button_history_[8] = data[2];

  // 2. Логируем для отладки
  ESP_LOGI(TAG, "9-BYTE BUFFER: [%02X %02X %02X %02X %02X %02X %02X %02X %02X]",
           button_history_[0], button_history_[1], button_history_[2],
           button_history_[3], button_history_[4], button_history_[5],
           button_history_[6], button_history_[7], button_history_[8]);

  // 3. ПРОВЕРЯЕМ КНОПКИ В ТОЧНОМ ПОРЯДКЕ КАК В ОРИГИНАЛЕ!
  int detected_button = -1;
  
  // Порядок ВАЖЕН! CENTER должен быть перед CAMERA!
  if (this->key_match_9byte_(button_history_, BUTTON_UP_9BYTE)) {
    detected_button = 0; // UP
  } else if (this->key_match_9byte_(button_history_, BUTTON_DOWN_9BYTE)) {
    detected_button = 1; // DOWN
  } else if (this->key_match_9byte_(button_history_, BUTTON_LEFT_9BYTE)) {
    detected_button = 2; // LEFT
  } else if (this->key_match_9byte_(button_history_, BUTTON_RIGHT_9BYTE)) {
    detected_button = 3; // RIGHT
  } else if (this->key_match_9byte_(button_history_, BUTTON_CENTER_9BYTE)) {
    detected_button = 4; // CENTER - ВАЖНО: перед CAMERA!
  } else if (this->key_match_9byte_(button_history_, BUTTON_CAMERA_9BYTE)) {
    detected_button = 5; // CAMERA - ВАЖНО: после CENTER!
  }

  // 4. ОБРАБОТКА (БЕЗ ЗАЩИТЫ ОТ ДРЕБЕЗГА, как в оригинале)
  if (detected_button != -1) {
    this->handle_button_press_(detected_button);
    ESP_LOGI(TAG, "Обработана кнопка %d", detected_button);
  }
}

// НОВЫЙ МЕТОД для сравнения 9-байтных массивов
bool BLEM3Remote::key_match_9byte_(const uint8_t* buf1, const std::vector<uint8_t>& vec2) {
  for (int i = 0; i < 9; i++) {
    if (buf1[i] != vec2[i]) {
      return false;
    }
  }
  return true;
}

void BLEM3Remote::handle_button_press_(int button_code) {
  const char* button_names[] = {"UP", "DOWN", "LEFT", "RIGHT", "CENTER", "CAMERA"};
  ESP_LOGI(TAG, "=== BUTTON PRESSED: %s ===", button_names[button_code]);
  
  binary_sensor::BinarySensor *sensor = nullptr;
  
  switch (button_code) {
    case 0: sensor = this->button_up_sensor_; break;
    case 1: sensor = this->button_down_sensor_; break;
    case 2: sensor = this->button_left_sensor_; break;
    case 3: sensor = this->button_right_sensor_; break;
    case 4: sensor = this->button_center_sensor_; break;
    case 5: sensor = this->button_camera_sensor_; break;
  }
  
  if (sensor != nullptr) {
    // Включаем сенсор
    sensor->publish_state(true);
    ESP_LOGI(TAG, "Button %s: ON", button_names[button_code]);
    
    // Выключаем через 50ms
    this->defer([this, sensor, button_names, button_code]() {
      delay(50);
      sensor->publish_state(false);
      ESP_LOGI(TAG, "Button %s: OFF", button_names[button_code]);
    });
  }
}

}  // namespace ble_m3_remote
}  // namespace esphome