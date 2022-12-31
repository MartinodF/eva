#include "eva_debug.h"

static const char *TAG = "debug";

static esp_err_t print_real_time_stats() {
  TaskStatus_t *start_array = NULL, *end_array = NULL;
  UBaseType_t start_array_size, end_array_size;
  uint32_t start_run_time, end_run_time;
  esp_err_t ret;

  // Allocate array to store current task states
  start_array_size = uxTaskGetNumberOfTasks() + EVA_DEBUG_ARRAY_SIZE_OFFSET;
  start_array = malloc(sizeof(TaskStatus_t) * start_array_size);
  if (start_array == NULL) {
    ret = ESP_ERR_NO_MEM;
    goto exit;
  }
  // Get current task states
  start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
  if (start_array_size == 0) {
    ret = ESP_ERR_INVALID_SIZE;
    goto exit;
  }

  vTaskDelay(pdMS_TO_TICKS(EVA_DEBUG_TICKS));

  // Allocate array to store tasks states post delay
  end_array_size = uxTaskGetNumberOfTasks() + EVA_DEBUG_ARRAY_SIZE_OFFSET;
  end_array = malloc(sizeof(TaskStatus_t) * end_array_size);
  if (end_array == NULL) {
    ret = ESP_ERR_NO_MEM;
    goto exit;
  }
  // Get post delay task states
  end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
  if (end_array_size == 0) {
    ret = ESP_ERR_INVALID_SIZE;
    goto exit;
  }

  // Calculate total_elapsed_time in units of run time stats clock period.
  uint32_t total_elapsed_time = (end_run_time - start_run_time);
  if (total_elapsed_time == 0) {
    ret = ESP_ERR_INVALID_STATE;
    goto exit;
  }

  // Match each task in start_array to those in the end_array
  for (size_t i = 0; i < start_array_size; i++) {
    int k = -1;
    for (size_t j = 0; j < end_array_size; j++) {
      if (start_array[i].xHandle == end_array[j].xHandle) {
        k = j;
        // Mark that task have been matched by overwriting their handles
        start_array[i].xHandle = NULL;
        end_array[j].xHandle = NULL;
        break;
      }
    }
    // Check if matching task found
    if (k >= 0) {
      uint32_t task_elapsed_time = end_array[k].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
      uint32_t percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time * portNUM_PROCESSORS);
      ESP_LOGD(TAG, "task: %-15s -> %7lu (%3lu%%)", start_array[i].pcTaskName, task_elapsed_time, percentage_time);
    }
  }

  // Print unmatched tasks
  for (size_t i = 0; i < start_array_size; i++) {
    if (start_array[i].xHandle != NULL) {
      ESP_LOGD(TAG, "task: %-15s -> deleted", start_array[i].pcTaskName);
    }
  }
  for (size_t i = 0; i < end_array_size; i++) {
    if (end_array[i].xHandle != NULL) {
      ESP_LOGD(TAG, "task: %-15s -> created", end_array[i].pcTaskName);
    }
  }
  ret = ESP_OK;

exit:  // Common return path
  free(start_array);
  free(end_array);
  return ret;
}

void debug_loop(void *unused) {
  ESP_LOGI(TAG, "debug_loop starting");

  for (;;) {
    ESP_ERROR_CHECK(print_real_time_stats());
    vTaskDelay(pdMS_TO_TICKS(60000));
  }

  ESP_LOGW(TAG, "debug_loop exited");
}
