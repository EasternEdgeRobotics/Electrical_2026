/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "errno.h"
#include "driver/gpio.h"

#include "usb/hid_host.h"
#include "usb/hid_usage_keyboard.h"
#include "usb/hid_usage_mouse.h"

/* GPIO Pin number for quit from example logic */
#define APP_QUIT_PIN                GPIO_NUM_0

static const char *TAG = "example";

QueueHandle_t app_event_queue = NULL;

/**
 * @brief APP event group
 *
 * Application logic can be different. There is a one among other ways to distinguish the
 * event by application event group.
 * In this example we have two event groups:
 * APP_EVENT            - General event, which is APP_QUIT_PIN press event (Generally, it is IO0).
 * APP_EVENT_HID_HOST   - HID Host Driver event, such as device connection/disconnection or input report.
 */
typedef enum {
    APP_EVENT = 0,
    APP_EVENT_HID_HOST
} app_event_group_t;

/**
 * @brief APP event queue
 *
 * This event is used for delivering the HID Host event from callback to a task.
 */
typedef struct {
    app_event_group_t event_group;
    /* HID Host - Device related info */
    struct {
        hid_host_device_handle_t handle;
        hid_host_driver_event_t event;
        void *arg;
    } hid_host_device;
} app_event_queue_t;

/**
 * @brief HID Protocol string names
 */
static const char *hid_proto_name_str[] = {
    "NONE",
    "KEYBOARD",
    "MOUSE"
};

/**
 * @brief Key event
 */
typedef struct {
    enum key_state {
        KEY_STATE_PRESSED = 0x00,
        KEY_STATE_RELEASED = 0x01
    } state;
    uint8_t modifier;
    uint8_t key_code;
} key_event_t;

/* Main char symbol for ENTER key */
#define KEYBOARD_ENTER_MAIN_CHAR    '\r'
/* When set to 1 pressing ENTER will be extending with LineFeed during serial debug output */
#define KEYBOARD_ENTER_LF_EXTEND    1

/**
 * @brief Makes new line depending on report output protocol type
 *
 * @param[in] proto Current protocol to output
 */
static void hid_print_new_device_report_header(hid_protocol_t proto)
{
    static hid_protocol_t prev_proto_output = -1;

    if (prev_proto_output != proto) {
        prev_proto_output = proto;
        printf("\r\n");
        if (proto == HID_PROTOCOL_MOUSE) {
            printf("Mouse\r\n");
        } else if (proto == HID_PROTOCOL_KEYBOARD) {
            printf("Keyboard\r\n");
        } else {
            printf("Generic\r\n");
        }
        fflush(stdout);
    }
}

typedef struct {
  bool a;
  bool b;
  bool x;
  bool y;
  bool l1;
  bool l2;
  bool l3;
  bool r1;
  bool r2;
  bool r3;
  bool ga; // Google Assistant 
  bool st; // Stadia
  bool mn; // Menu
  bool sl; // Select
  bool cp; // Capture
} b_state_t;

typedef struct {
    float lx; // Left Joystick X
    float ly; // Left Joystick Y
    float rx; // Right Joystick X
    float ry; // Right Joystick Y
    float l2; // Left Trigger Analog
    float r2; // Right Triggr Analog
} axis_state_t;

static const char *const dpad_names[0x09] = {
    [0x00] = "Up",
    [0x01] = "Up Right",
    [0x02] = "Right",
    [0x03] = "Down Right",
    [0x04] = "Down",
    [0x05] = "Down Left",
    [0x06] = "Left",
    [0x07] = "Up Left",
    [0x08] = "None",
};

#define FB_BTN_A 0x40
#define FB_BTN_B 0x20
#define FB_BTN_X 0x10
#define FB_BTN_Y 0x08
#define FB_BTN_L1 0x04
#define FB_BTN_L3 0x01
#define FB_BTN_R1 0x02
#define FB_BTN_R2 0x80

#define MB_BTN_GA 0x02
#define MB_BTN_ST 0x10
#define MB_BTN_MN 0x20
#define MB_BTN_SL 0x40
#define MB_BTN_CP 0x01
#define MB_BTN_L2 0x04
#define MB_BTN_L3 0x08
#define MB_BTN_R3 0x80

static inline const char *parse_dpad(uint8_t raw) {
    if (raw <= 0x08 && dpad_names[raw])
        return dpad_names[raw];
    return "Invalid";
}

static b_state_t parse_buttons (uint8_t face_byte, uint8_t menu_byte) {
    b_state_t s = {0};
    // Face Buttons
    s.a = (face_byte & FB_BTN_A) != 0;
    s.b = (face_byte & FB_BTN_B) != 0;
    s.x = (face_byte & FB_BTN_X) != 0;
    s.y = (face_byte & FB_BTN_Y) != 0;

    // Left Trigger, bumper and joystick button
    s.l1 = (face_byte & FB_BTN_L1) != 0;
    s.l2 = (menu_byte & MB_BTN_L2) != 0;
    s.l3 = (face_byte & FB_BTN_L3) != 0;

    // Right trigger, bumper and joystick button
    s.r1 = (face_byte & FB_BTN_R1) != 0;
    s.r2 = (face_byte & FB_BTN_R2) != 0;
    s.r3 = (menu_byte & MB_BTN_R3) != 0;

    // Menu buttons
    s.ga = (menu_byte & MB_BTN_GA) != 0; // Google Assistant
    s.st = (menu_byte & MB_BTN_ST) != 0; // Stadia Button
    s.mn = (menu_byte & MB_BTN_MN) != 0; // Menu Button
    s.sl = (menu_byte & MB_BTN_SL) != 0; // Select button (...)
    s.cp = (menu_byte & MB_BTN_CP) != 0; // Capture BUtton 
    return s;
}

static void print_buttons(const b_state_t *s) {
  printf("A:%s B:%s X:%s Y:%s\n",
         s->a ? "down" : "up",
         s->b ? "down" : "up",
         s->x ? "down" : "up",
         s->y ? "down" : "up");

  printf("L1:%s L2:%s L3:%s R1:%s R2:%s R3:%s\n",
         s->l1 ? "down" : "up",
         s->l2 ? "down" : "up",
         s->l3 ? "down" : "up",
         s->r1 ? "down" : "up",
         s->r2 ? "down" : "up",
         s->r3 ? "down" : "up");

  printf("Google:%s Stadia:%s Menu:%s Select:%s Capture:%s\n",
         s->ga ? "down" : "up",
         s->st ? "down" : "up",
         s->mn ? "down" : "up",
         s->sl ? "down" : "up",
         s->cp ? "down" : "up");
}

static axis_state_t parse_axis(uint8_t lx_raw, uint8_t ly_raw, uint8_t rx_raw, uint8_t ry_raw, uint8_t l2_raw, uint8_t r2_raw) {
  // Normalized to -1..+1 (handle asymmetry 127 vs 128)
  axis_state_t s = {0};
  s.lx = (lx_raw >= 128)
            ? ((float)lx_raw - 128.0f) / 127.0f   // 128..255 -> 0..+1
            : ((float)lx_raw - 128.0f) / 128.0f;  // 0..127   -> -1..0
  s.ly = (ly_raw >= 128)
            ? ((float)ly_raw - 128.0f) / 127.0f
            : ((float)ly_raw - 128.0f) / 128.0f;
  s.rx = (rx_raw >= 128)
            ? ((float)rx_raw - 128.0f) / 127.0f
            : ((float)rx_raw - 128.0f) / 128.0f;
  s.ry = (ry_raw >= 128)
            ? ((float)ry_raw - 128.0f) / 127.0f
            : ((float)ry_raw - 128.0f) / 128.0f;

  s.l2 = (float)l2_raw / 255.0f;
  s.r2 = (float)r2_raw / 255.0f;

  return s;
}

static void print_joysticks(const axis_state_t *s) {
  printf("Left X = %.3f Left Y = %.3f Right X = %.3f Right Y = %.3f\n", s->lx, s->ly, s->rx, s->ry);
}

static void print_triggers(const axis_state_t *s) {
  printf("L2 = %.3f R2 = %.3f\n", s->l2, s->r2);
}

/**
 * @brief USB HID Host Generic Interface report callback handler
 *
 * 'generic' means anything else than mouse or keyboard
 *
 * @param[in] data    Pointer to input report data buffer
 * @param[in] length  Length of input report data buffer
 */

static void hid_host_generic_report_callback_raw(const uint8_t *const data, const int length)
{
    hid_print_new_device_report_header(HID_PROTOCOL_NONE);
    for (int i = 0; i < length; i++) {
        printf("%02X", data[i]);
    }
    putchar('\r');
}
static void hid_host_generic_report_callback(const uint8_t *const data, const int length)
{
    hid_print_new_device_report_header(HID_PROTOCOL_NONE);
    const size_t DPAD_OFFSET = 1;
    const size_t MB_OFFSET = 2;
    const size_t FB_OFFSET = 3;
    const size_t LX_OFFSET = 4;
    const size_t LY_OFFSET = 5;
    const size_t RX_OFFSET = 6;
    const size_t RY_OFFSET = 7;
    const size_t L2_OFFSET = 8;
    const size_t R2_OFFSET = 9;
    printf("\n");

    // D-Pad
    printf("D-Pad: %s ", parse_dpad(data[DPAD_OFFSET]));
    
    // Menu and Face Buttons
    b_state_t buttons = parse_buttons(data[FB_OFFSET], data[MB_OFFSET]);
    print_buttons(&buttons);

    // Axises - idk what the plural verion of axis is lmao
    axis_state_t axis = parse_axis(data[LX_OFFSET], data[LY_OFFSET], data[RX_OFFSET], data[RY_OFFSET], data[L2_OFFSET], data[R2_OFFSET]);
    print_joysticks(&axis);
    print_triggers(&axis);
    
    printf("\n");
    //putchar('\r');
}

/**
 * @brief USB HID Host interface callback
 *
 * @param[in] hid_device_handle  HID Device handle
 * @param[in] event              HID Host interface event
 * @param[in] arg                Pointer to arguments, does not used
 */
void hid_host_interface_callback(hid_host_device_handle_t hid_device_handle,
                                 const hid_host_interface_event_t event,
                                 void *arg)
{
    uint8_t data[64] = { 0 };
    size_t data_length = 0;
    hid_host_dev_params_t dev_params;
    ESP_ERROR_CHECK(hid_host_device_get_params(hid_device_handle, &dev_params));

    switch (event) {
    case HID_HOST_INTERFACE_EVENT_INPUT_REPORT:
        ESP_ERROR_CHECK(hid_host_device_get_raw_input_report_data(hid_device_handle,
                                                                  data,
                                                                  64,
                                                                  &data_length));

        
        hid_host_generic_report_callback(data, data_length);

        break;
    case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HID Device, protocol '%s' DISCONNECTED",
                 hid_proto_name_str[dev_params.proto]);
        ESP_ERROR_CHECK(hid_host_device_close(hid_device_handle));
        break;
    case HID_HOST_INTERFACE_EVENT_TRANSFER_ERROR:
        ESP_LOGI(TAG, "HID Device, protocol '%s' TRANSFER_ERROR",
                 hid_proto_name_str[dev_params.proto]);
        break;
    default:
        ESP_LOGE(TAG, "HID Device, protocol '%s' Unhandled event",
                 hid_proto_name_str[dev_params.proto]);
        break;
    }
}

/**
 * @brief USB HID Host Device event
 *
 * @param[in] hid_device_handle  HID Device handle
 * @param[in] event              HID Host Device event
 * @param[in] arg                Pointer to arguments, does not used
 */
void hid_host_device_event(hid_host_device_handle_t hid_device_handle,
                           const hid_host_driver_event_t event,
                           void *arg)
{
    hid_host_dev_params_t dev_params;
    ESP_ERROR_CHECK(hid_host_device_get_params(hid_device_handle, &dev_params));

    switch (event) {
    case HID_HOST_DRIVER_EVENT_CONNECTED:
        ESP_LOGI(TAG, "HID Device, protocol '%s' CONNECTED",
                 hid_proto_name_str[dev_params.proto]);

        const hid_host_device_config_t dev_config = {
            .callback = hid_host_interface_callback,
            .callback_arg = NULL
        };

        ESP_ERROR_CHECK(hid_host_device_open(hid_device_handle, &dev_config));
        if (HID_SUBCLASS_BOOT_INTERFACE == dev_params.sub_class) {
            ESP_ERROR_CHECK(hid_class_request_set_protocol(hid_device_handle, HID_REPORT_PROTOCOL_BOOT));
            if (HID_PROTOCOL_KEYBOARD == dev_params.proto) {
                ESP_ERROR_CHECK(hid_class_request_set_idle(hid_device_handle, 0, 0));
            }
        }
        ESP_ERROR_CHECK(hid_host_device_start(hid_device_handle));
        break;
    default:
        break;
    }
}

/**
 * @brief Start USB Host install and handle common USB host library events while app pin not low
 *
 * @param[in] arg  Not used
 */
static void usb_lib_task(void *arg)
{
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };

    ESP_ERROR_CHECK(usb_host_install(&host_config));
    xTaskNotifyGive(arg);

    while (true) {
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        // In this example, there is only one client registered
        // So, once we deregister the client, this call must succeed with ESP_OK
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_ERROR_CHECK(usb_host_device_free_all());
            break;
        }
    }

    ESP_LOGI(TAG, "USB shutdown");
    // Clean up USB Host
    vTaskDelay(10); // Short delay to allow clients clean-up
    ESP_ERROR_CHECK(usb_host_uninstall());
    vTaskDelete(NULL);
}

/**
 * @brief BOOT button pressed callback
 *
 * Signal application to exit the HID Host task
 *
 * @param[in] arg Unused
 */
static void gpio_isr_cb(void *arg)
{
    BaseType_t xTaskWoken = pdFALSE;
    const app_event_queue_t evt_queue = {
        .event_group = APP_EVENT,
    };

    if (app_event_queue) {
        xQueueSendFromISR(app_event_queue, &evt_queue, &xTaskWoken);
    }

    if (xTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

/**
 * @brief HID Host Device callback
 *
 * Puts new HID Device event to the queue
 *
 * @param[in] hid_device_handle HID Device handle
 * @param[in] event             HID Device event
 * @param[in] arg               Not used
 */
void hid_host_device_callback(hid_host_device_handle_t hid_device_handle,
                              const hid_host_driver_event_t event,
                              void *arg)
{
    const app_event_queue_t evt_queue = {
        .event_group = APP_EVENT_HID_HOST,
        // HID Host Device related info
        .hid_host_device.handle = hid_device_handle,
        .hid_host_device.event = event,
        .hid_host_device.arg = arg
    };

    if (app_event_queue) {
        xQueueSend(app_event_queue, &evt_queue, 0);
    }
}

void app_main(void)
{
    BaseType_t task_created;
    app_event_queue_t evt_queue;
    ESP_LOGI(TAG, "HID Host example");

    // Init BOOT button: Pressing the button simulates app request to exit
    // It will disconnect the USB device and uninstall the HID driver and USB Host Lib
    const gpio_config_t input_pin = {
        .pin_bit_mask = BIT64(APP_QUIT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&input_pin));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1));
    ESP_ERROR_CHECK(gpio_isr_handler_add(APP_QUIT_PIN, gpio_isr_cb, NULL));

    /*
    * Create usb_lib_task to:
    * - initialize USB Host library
    * - Handle USB Host events while APP pin in in HIGH state
    */
    task_created = xTaskCreatePinnedToCore(usb_lib_task,
                                           "usb_events",
                                           4096,
                                           xTaskGetCurrentTaskHandle(),
                                           2, NULL, 0);
    assert(task_created == pdTRUE);

    // Wait for notification from usb_lib_task to proceed
    ulTaskNotifyTake(false, 1000);

    /*
    * HID host driver configuration
    * - create background task for handling low level event inside the HID driver
    * - provide the device callback to get new HID Device connection event
    */
    const hid_host_driver_config_t hid_host_driver_config = {
        .create_background_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .core_id = 0,
        .callback = hid_host_device_callback,
        .callback_arg = NULL
    };

    ESP_ERROR_CHECK(hid_host_install(&hid_host_driver_config));

    // Create queue
    app_event_queue = xQueueCreate(10, sizeof(app_event_queue_t));

    ESP_LOGI(TAG, "Waiting for HID Device to be connected");

    while (1) {
        // Wait queue
        if (xQueueReceive(app_event_queue, &evt_queue, portMAX_DELAY)) {
            if (APP_EVENT == evt_queue.event_group) {
                // User pressed button
                usb_host_lib_info_t lib_info;
                ESP_ERROR_CHECK(usb_host_lib_info(&lib_info));
                if (lib_info.num_devices == 0) {
                    // End while cycle
                    break;
                } else {
                    ESP_LOGW(TAG, "To shutdown example, remove all USB devices and press button again.");
                    // Keep polling
                }
            }

            if (APP_EVENT_HID_HOST ==  evt_queue.event_group) {
                hid_host_device_event(evt_queue.hid_host_device.handle,
                                      evt_queue.hid_host_device.event,
                                      evt_queue.hid_host_device.arg);
            }
        }
    }

    ESP_LOGI(TAG, "HID Driver uninstall");
    ESP_ERROR_CHECK(hid_host_uninstall());
    gpio_isr_handler_remove(APP_QUIT_PIN);
    xQueueReset(app_event_queue);
    vQueueDelete(app_event_queue);
}