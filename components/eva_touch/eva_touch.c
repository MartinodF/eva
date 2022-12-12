#include "eva_touch.h"

static QueueHandle_t touches = NULL;
typedef struct touch {
  touch_pad_intr_mask_t intr_mask;
  uint32_t pad_num;
  uint32_t pad_status;
  uint32_t pad_val;
} touch_event_t;

static esp_timer_handle_t touch_timer;
static int touch_count = 0;
static bool touch_down = false;

static const char* TAG = "touch";

static void touch_intr_cb(void* arg) {
  int task_awoken = pdFALSE;

  if (touch_pad_get_current_meas_channel() == EVA_TOUCH_TOUCH_NUM) {
    touch_event_t evt;

    evt.intr_mask = touch_pad_read_intr_status_mask();
    evt.pad_status = touch_pad_get_status();
    evt.pad_num = touch_pad_get_current_meas_channel();

    xQueueSendFromISR(touches, &evt, &task_awoken);
  }

  if (task_awoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

static void touch_timer_cb(void* param) {
  ESP_LOGD(TAG, "touch timer elapsed");

  if (touch_down) {
    ESP_LOGI(TAG, "emitting hold");
    ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_TOUCH_HOLD, NULL, 0, portMAX_DELAY));
  } else {
    touch_count = touch_count > 3 ? 3 : touch_count;

    ESP_LOGI(TAG, "emitting tap x %d", touch_count);
    ESP_ERROR_CHECK(esp_event_post(EVA_EVENT, EVA_TOUCH_TAP + touch_count - 1, NULL, 0, portMAX_DELAY));
  }

  touch_count = 0;
}

void touch_loop(void* unused) {
  ESP_LOGI(TAG, "touch_loop starting");

  if (touches == NULL) {
    touches = xQueueCreate(4, sizeof(touch_event_t));
  }

  const esp_timer_create_args_t timer_args = {.callback = &touch_timer_cb, .name = "touch-timer"};
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &touch_timer));

  ESP_ERROR_CHECK(touch_pad_init());
  ESP_ERROR_CHECK(touch_pad_config(EVA_TOUCH_TOUCH_NUM));

  // Reduce power consumption since we have a very clean signal
  ESP_ERROR_CHECK(touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_LVOLT_0V8, TOUCH_PAD_ATTEN_VOLTAGE_THRESHOLD));
  ESP_ERROR_CHECK(touch_pad_set_cnt_mode(EVA_TOUCH_TOUCH_NUM, TOUCH_PAD_SLOPE_3, TOUCH_PAD_TIE_OPT_DEFAULT));

  ESP_ERROR_CHECK(touch_pad_isr_register(touch_intr_cb, NULL, TOUCH_PAD_INTR_MASK_ALL));
  ESP_ERROR_CHECK(touch_pad_intr_enable(TOUCH_PAD_INTR_MASK_ACTIVE | TOUCH_PAD_INTR_MASK_INACTIVE));

  ESP_ERROR_CHECK(touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER));
  ESP_ERROR_CHECK(touch_pad_fsm_start());

  // Wait for full initialization
  vTaskDelay(pdMS_TO_TICKS(50));

  uint32_t value;

  ESP_ERROR_CHECK(touch_pad_read_benchmark(EVA_TOUCH_TOUCH_NUM, &value));
  ESP_ERROR_CHECK(touch_pad_set_thresh(EVA_TOUCH_TOUCH_NUM, value * EVA_TOUCH_THRESHOLD));

  ESP_LOGI(TAG, "base: %lu, threshold: %lu", value, (uint32_t)(value * EVA_TOUCH_THRESHOLD));

  touch_event_t evt = {0};

  for (;;) {
    int ret = xQueueReceive(touches, &evt, portMAX_DELAY);
    if (!ret) {
      continue;
    }

    if (evt.intr_mask & TOUCH_PAD_INTR_MASK_ACTIVE) {
      ESP_LOGD(TAG, "activated %lu", evt.pad_num);

      touch_down = true;
      touch_count++;

      if (esp_timer_is_active(touch_timer)) {
        ESP_ERROR_CHECK(esp_timer_stop(touch_timer));
      }

      ESP_ERROR_CHECK(esp_timer_start_once(touch_timer, EVA_TOUCH_DELAY * 1000));
    }
    if (evt.intr_mask & TOUCH_PAD_INTR_MASK_INACTIVE) {
      ESP_LOGD(TAG, "deactivated %lu", evt.pad_num);

      touch_down = false;

      if (esp_timer_is_active(touch_timer)) {
        ESP_ERROR_CHECK(esp_timer_stop(touch_timer));
        ESP_ERROR_CHECK(esp_timer_start_once(touch_timer, EVA_TOUCH_DELAY * 1000));
      }
    }
  }

  ESP_ERROR_CHECK(touch_pad_fsm_stop());
  ESP_ERROR_CHECK(touch_pad_reset());
  ESP_ERROR_CHECK(touch_pad_deinit());

  ESP_LOGW(TAG, "touch_loop exited");
}
