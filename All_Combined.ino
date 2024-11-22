#include "U8glib.h"
#include "clock_menu.h"
#include "about_menu.h"

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);

// Button definitions
#define BUTTON_UP 5
#define BUTTON_SELECT 6
#define BUTTON_DOWN 7

#define RELAY1_PIN 8
#define RELAY2_PIN 9
#define RELAY3_PIN 10
#define RELAY4_PIN 11

// Menu state flags
bool inSubmenu = false; // Flag for submenu state

// Menu navigation
int item_selected = 0;
int item_sel_prev, item_sel_next;

// Submenu navigation
int sitem_selected = 0;
bool sitem_states[4] = { false, false, false, false }; // All items OFF by default

// Timing for button debounce
unsigned long last_press_time = 0;
const unsigned long debounce_delay = 200; // 200ms debounce
const unsigned long idle_timeout = 8000;  // 5 seconds idle timeout

// 'icons', 128x32px
const unsigned char epd_bitmap_sel_opt [] PROGMEM = {
  0x07, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x20, 0x10, 0x00, 0x00, 0x00, 
  0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 
  0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 
  0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 
  0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 
  0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 
  0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 
  0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 
  0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 
  0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 
  0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x08, 0x00, 
  0x00, 0x00, 0x00, 0x38, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xff, 0xff, 0xf0
};

// 'icon_about', 32x24px
const unsigned char epd_bitmap_icon_about [] PROGMEM = {
  0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x07, 0xe0, 0x00, 
  0x00, 0x0e, 0x70, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x1c, 0x38, 0x00, 0x00, 0x38, 0x1c, 0x00, 
  0x00, 0x31, 0x8c, 0x00, 0x00, 0x71, 0x8e, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xc1, 0x83, 0x80, 
  0x01, 0xc1, 0x81, 0x80, 0x03, 0x81, 0x81, 0xc0, 0x07, 0x01, 0x80, 0xe0, 0x06, 0x01, 0x80, 0x60, 
  0x0e, 0x01, 0x80, 0x70, 0x1c, 0x01, 0x80, 0x38, 0x18, 0x01, 0x80, 0x18, 0x38, 0x01, 0x80, 0x1c, 
  0x70, 0x00, 0x00, 0x0e, 0x60, 0x00, 0x00, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

// 'icon_about', 32x24px
const unsigned char epd_bitmap_icon_power_off [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x14, 0x28, 0x00, 
  0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 
  0x00, 0x14, 0x28, 0x00, 0x01, 0xff, 0xff, 0x80, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 
  0x01, 0xff, 0xff, 0x80, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 0x84, 0x00, 0x00, 0x21, 0x04, 0x00, 
  0x00, 0x23, 0xc4, 0x00, 0x00, 0x20, 0x84, 0x00, 0x00, 0x21, 0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 
  0x00, 0x10, 0x08, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 'icon_about', 32x24px
const unsigned char epd_bitmap_icon_power_on [] PROGMEM = {
  0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x07, 0xe0, 0x00, 
  0x00, 0x0e, 0x70, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x1c, 0x38, 0x00, 0x00, 0x38, 0x1c, 0x00, 
  0x00, 0x31, 0x8c, 0x00, 0x00, 0x71, 0x8e, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xc1, 0x83, 0x80, 
  0x01, 0xc1, 0x81, 0x80, 0x03, 0x81, 0x81, 0xc0, 0x07, 0x01, 0x80, 0xe0, 0x06, 0x01, 0x80, 0x60, 
  0x0e, 0x01, 0x80, 0x70, 0x1c, 0x01, 0x80, 0x38, 0x18, 0x01, 0x80, 0x18, 0x38, 0x01, 0x80, 0x1c, 
  0x70, 0x00, 0x00, 0x0e, 0x60, 0x00, 0x00, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

// 'icon_Clock', 32x24px
const unsigned char epd_bitmap_icon_Clock [] PROGMEM = {
  0x00, 0x07, 0xf0, 0x00, 0x00, 0x38, 0x8c, 0x00, 0x00, 0x60, 0x82, 0x00, 0x00, 0x80, 0x01, 0x00, 
  0x01, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x40, 0x06, 0x00, 0x00, 0x20, 0x04, 0x00, 0x02, 0x20, 
  0x04, 0x00, 0x04, 0x30, 0x08, 0x00, 0x08, 0x10, 0x08, 0x08, 0x10, 0x10, 0x0e, 0x04, 0x20, 0x70, 
  0x08, 0x02, 0x40, 0x10, 0x08, 0x01, 0x80, 0x10, 0x08, 0x00, 0x80, 0x10, 0x04, 0x00, 0x00, 0x20, 
  0x04, 0x00, 0x00, 0x20, 0x06, 0x00, 0x00, 0x60, 0x03, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc0, 
  0x00, 0xc0, 0x01, 0x80, 0x00, 0x60, 0x83, 0x00, 0x00, 0x38, 0x9c, 0x00, 0x00, 0x07, 0xe0, 0x00
};
// 'icon_Monitor', 32x24px
const unsigned char epd_bitmap_icon_Switch [] PROGMEM = {
  0x3f, 0xff, 0xff, 0xfc, 0x20, 0x00, 0x00, 0x04, 0x2f, 0xff, 0xff, 0xf4, 0x28, 0x00, 0x00, 0x14, 
  0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 
  0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 
  0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x2f, 0xff, 0xff, 0xf4, 0x20, 0x00, 0x00, 0x04, 
  0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x3f, 0xff, 0xff, 0xfc, 0x20, 0x00, 0x00, 0x04, 0x20, 0x00, 0x00, 0x04, 0x3f, 0xff, 0xff, 0xfc
};
// 'sub_menu_selector_icon', 32x24px
const unsigned char sub_menu_selector_icon [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x1f, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff
};

// 'sub_charger_off_icon', 32x24px
const unsigned char sub_charger_off_icon [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x14, 0x28, 0x00, 
  0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 0x00, 0x14, 0x28, 0x00, 
  0x00, 0x14, 0x28, 0x00, 0x01, 0xff, 0xff, 0x80, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 
  0x01, 0xff, 0xff, 0x80, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 0x84, 0x00, 0x00, 0x21, 0x04, 0x00, 
  0x00, 0x23, 0xc4, 0x00, 0x00, 0x20, 0x84, 0x00, 0x00, 0x21, 0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 
  0x00, 0x10, 0x08, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'sub_charger_on_icon copy', 32x24px
const unsigned char sub_charger_on_icon [] PROGMEM = {
 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x1f, 0xf3, 0xcf, 0xf8, 0x3f, 0xeb, 0xd7, 0xfc, 
  0x7f, 0xeb, 0xd7, 0xfe, 0x7f, 0xeb, 0xd7, 0xfe, 0x7f, 0xeb, 0xd7, 0xfe, 0x7f, 0xeb, 0xd7, 0xfe, 
  0x7f, 0xeb, 0xd7, 0xfe, 0x7e, 0x00, 0x00, 0x7e, 0x7d, 0xff, 0xff, 0xbe, 0x7d, 0xff, 0xff, 0xbe, 
  0x7e, 0x00, 0x00, 0x7e, 0x7f, 0xdf, 0xfb, 0xfe, 0x7f, 0xdf, 0x7b, 0xfe, 0x7f, 0xde, 0xfb, 0xfe, 
  0x7f, 0xdc, 0x3b, 0xfe, 0x7f, 0xdf, 0x7b, 0xfe, 0x7f, 0xde, 0xfb, 0xfe, 0x7f, 0xdf, 0xfb, 0xfe, 
  0x3f, 0xef, 0xf7, 0xfc, 0x1f, 0xe0, 0x07, 0xf8, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0
};
// 'sub_light_off_icon', 32x24px
const unsigned char sub_light_off_icon [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xc0, 
  0x10, 0x00, 0x00, 0x20, 0x16, 0xdb, 0x6d, 0xa0, 0x16, 0xdb, 0x6d, 0xa0, 0x10, 0x00, 0x00, 0x20, 
  0x0f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0x9c, 0x00, 0x00, 0x84, 0x12, 0x00, 
  0x00, 0x87, 0x92, 0x00, 0x00, 0x94, 0x12, 0x00, 0x00, 0xf7, 0x9c, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x07, 0xff, 0xff, 0xe0, 0x08, 0x00, 0x00, 0x10, 0x0b, 0x6d, 0xb6, 0xd0, 0x0b, 0x6d, 0xb6, 0xd0, 
  0x08, 0x00, 0x00, 0x10, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'sub_light_on_icon copy', 32x24px
const unsigned char sub_light_on_icon [] PROGMEM = {
 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xf8, 0x30, 0x00, 0x00, 0x3c, 
  0x6f, 0xff, 0xff, 0xde, 0x69, 0x24, 0x92, 0x5e, 0x69, 0x24, 0x92, 0x5e, 0x6f, 0xff, 0xff, 0xde, 
  0x70, 0x00, 0x00, 0x1e, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0x78, 0x63, 0xfe, 0x7f, 0x7b, 0xed, 0xfe, 
  0x7f, 0x78, 0x6d, 0xfe, 0x7f, 0x6b, 0xed, 0xfe, 0x7f, 0x08, 0x63, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 
  0x78, 0x00, 0x00, 0x1e, 0x77, 0xff, 0xff, 0xee, 0x74, 0x92, 0x49, 0x2e, 0x74, 0x92, 0x49, 0x2e, 
  0x37, 0xff, 0xff, 0xec, 0x18, 0x00, 0x00, 0x18, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0
};

// 'sub_monitor_off_icon', 32x24px
const unsigned char sub_monitor_off_icon [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 
  0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 
  0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x20, 
  0x07, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 
  0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'sub_monitor_on_icon copy', 32x24px
const unsigned char sub_monitor_on_icon [] PROGMEM = {
 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xfc, 
  0x7f, 0xff, 0xff, 0xfe, 0x78, 0x00, 0x00, 0x1e, 0x7b, 0xff, 0xff, 0xde, 0x7b, 0xff, 0xff, 0xde, 
  0x7b, 0xff, 0xff, 0xde, 0x7b, 0xff, 0xff, 0xde, 0x7b, 0xff, 0xff, 0xde, 0x7b, 0xff, 0xff, 0xde, 
  0x7b, 0xff, 0xff, 0xde, 0x7b, 0xff, 0xff, 0xde, 0x7b, 0xff, 0xff, 0xde, 0x7b, 0xff, 0xff, 0xde, 
  0x78, 0x00, 0x00, 0x1e, 0x78, 0x00, 0x00, 0x1e, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 
  0x3f, 0xf0, 0x0f, 0xfc, 0x1f, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0
};
// 'sub_printer_off_icon', 32x24px
const unsigned char sub_printer_off_icon [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x60, 0x06, 0x00, 
  0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 
  0x0f, 0xff, 0xff, 0xf0, 0x08, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x50, 0x08, 0x00, 0x00, 0x10, 
  0x09, 0xff, 0xff, 0x90, 0x09, 0xff, 0xff, 0x90, 0x09, 0x80, 0x01, 0x90, 0x0f, 0xbf, 0xfd, 0xf0, 
  0x00, 0x80, 0x01, 0x00, 0x00, 0xbf, 0xfd, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0xbf, 0xfd, 0x00, 
  0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'sub_printer_on_icon copy', 32x24px
const unsigned char sub_printer_on_icon [] PROGMEM = {
 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x1f, 0xc0, 0x03, 0xf8, 0x3f, 0x9f, 0xf9, 0xfc, 
  0x7f, 0xbf, 0xfd, 0xfe, 0x7f, 0xbf, 0xfd, 0xfe, 0x7f, 0xbf, 0xfd, 0xfe, 0x7f, 0xbf, 0xfd, 0xfe, 
  0x70, 0x00, 0x00, 0x0e, 0x77, 0xff, 0xff, 0xee, 0x77, 0xff, 0xff, 0xae, 0x77, 0xff, 0xff, 0xee, 
  0x76, 0x00, 0x00, 0x6e, 0x76, 0x00, 0x00, 0x6e, 0x76, 0x7f, 0xfe, 0x6e, 0x70, 0x40, 0x02, 0x0e, 
  0x7f, 0x7f, 0xfe, 0xfe, 0x7f, 0x40, 0x02, 0xfe, 0x7f, 0x7f, 0xfe, 0xfe, 0x7f, 0x40, 0x02, 0xfe, 
  0x3f, 0x00, 0x00, 0xfc, 0x1f, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0
};


// Icons for main menu
const unsigned char* menu_items[] = { epd_bitmap_icon_Clock, epd_bitmap_icon_Switch, epd_bitmap_icon_about, epd_bitmap_icon_power_off};
const int menu_item_len = 4;

// Icons for submenu (on and off states)
const int smenu_item_len = 4;
const unsigned char* smenu_on[] = { sub_charger_on_icon, sub_light_on_icon, sub_monitor_on_icon, sub_printer_on_icon };
const unsigned char* smenu_off[] = { sub_charger_off_icon, sub_light_off_icon, sub_monitor_off_icon, sub_printer_off_icon };


// Relay pins array
const int relay_pins[] = { RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN };

void setup() {
  u8g.setColorIndex(1); // Set the color to white

  // Initialize buttons
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  // Initialize relays
  for (int i = 0; i < smenu_item_len; i++) {
    pinMode(relay_pins[i], OUTPUT);
    digitalWrite(relay_pins[i], LOW); // Start with all relays off
   
  }
  // Start tracking time
  last_press_time = millis();
}

void loop() {
  item_sel_prev = item_selected -1;
  if (item_sel_prev < 0) (item_sel_prev = menu_item_len -1);

  item_sel_next = item_selected + 1;
  if (item_sel_next >= menu_item_len)(item_sel_next = 0);
 
  handleButtonPress(); // Process button presses
  updateRelays();      // Update relay states
  
  if (millis() - last_press_time > idle_timeout) {
    defaultLive(); // Call default function if idle
    return;        // Avoid further processing in this loop iteration
  }
  
  if (inSubmenu) {
    renderSMenu(); // Stay in the submenu
  } else {
    renderMainMenu(); // Render main menu
    handleMainMenuSelection(); // Handle main menu selection
  }
}

void handleButtonPress() {
  unsigned long current_time = millis();
  if (current_time - last_press_time < debounce_delay) return;

  if (digitalRead(BUTTON_UP) == LOW) {
    last_press_time = current_time; // Reset idle timer
    if (inSubmenu) {
      sitem_selected = (sitem_selected - 1 + smenu_item_len) % smenu_item_len; // Navigate submenu
    } else {
      item_selected = (item_selected - 1 + menu_item_len) % menu_item_len; // Navigate main menu
    }
  }

  if (digitalRead(BUTTON_DOWN) == LOW) {
    last_press_time = current_time; // Reset idle timer
    if (inSubmenu) {
      sitem_selected = (sitem_selected + 1) % smenu_item_len; // Navigate submenu
    } else {
      item_selected = (item_selected + 1) % menu_item_len; // Navigate main menu
    }
  }

  if (digitalRead(BUTTON_SELECT) == LOW) {
    last_press_time = current_time; // Reset idle timer
    delay(20); // Stabilize the reading
    while (digitalRead(BUTTON_SELECT) == LOW); // Wait for button release

    if (inSubmenu) {
      sitem_states[sitem_selected] = !sitem_states[sitem_selected]; // Toggle submenu state
    }
  }
}

void updateRelays() {
  // Update relay states based on submenu item states
  for (int i = 0; i < smenu_item_len; i++) {
    digitalWrite(relay_pins[i], sitem_states[i] ? HIGH : LOW);
  }
}

 

void renderMainMenu() {
  u8g.firstPage();
  do {
    // Draw menu icons
    u8g.drawBitmapP(39, 0, 48 / 8, 32, epd_bitmap_sel_opt); // Draw the bitmap
    u8g.drawBitmapP(4, 4, 32 / 8, 24, menu_items[item_sel_prev]);
    u8g.drawBitmapP(47, 4, 32 / 8, 24, menu_items[item_selected]);
    u8g.drawBitmapP(90, 4, 32 / 8, 24, menu_items[item_sel_next]);
  } while (u8g.nextPage());
}

void handleMainMenuSelection() {
  if (digitalRead(BUTTON_SELECT) == LOW) {
    last_press_time = millis(); // Reset idle timer
    delay(20); // Short delay to stabilize reading
    while (digitalRead(BUTTON_SELECT) == LOW); // Wait for button release

    switch (item_selected) {
      case 0: 
        displayClock(); // Implement this function to display clock
        break;
      case 1: 
        inSubmenu = true; // Enter the submenu
        break;
      case 2: 
        displayAbout(); // Implement this function to display About info
        break;
      case 3: 
        displayAbout(); // Implement this function to display About info
        break;
    }
  }
}


void renderSMenu() {
  u8g.firstPage();
  do {
    for (int i = 0; i < smenu_item_len; i++) {
      int x = i * 32; // X position for each submenu item
      const unsigned char* icon = sitem_states[i] ? smenu_on[i] : smenu_off[i];
      u8g.drawBitmapP(x, 0, 32 / 8, 24, icon); // Draw submenu icon
    }
    // Draw selector icon below the selected submenu item
    u8g.drawBitmapP(sitem_selected * 32, 24, 32 / 8, 8, sub_menu_selector_icon);
  } while (u8g.nextPage());

  // Exit submenu on long press
  if (digitalRead(BUTTON_SELECT) == LOW) {
    unsigned long press_start = millis();
    while (digitalRead(BUTTON_SELECT) == LOW); // Wait for button release
    if (millis() - press_start > 1000) { // Long press to exit
      inSubmenu = false;
    }
  }
}

// Idle timeout function
void defaultLive() {
  displayAbout();
}
