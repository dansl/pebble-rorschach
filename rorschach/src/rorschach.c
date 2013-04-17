#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0xC9, 0x26, 0x15, 0x47, 0x71, 0x41, 0x44, 0xFF, 0x83, 0x7A, 0x86, 0xEC, 0xDF, 0xFD, 0x26, 0x63 }
PBL_APP_INFO(MY_UUID,
             "Rorschach", "Dansl",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

#define TOTAL_IMAGE_SLOTS 8

#define NUMBER_OF_IMAGES 10

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_NUM_0, RESOURCE_ID_IMAGE_NUM_1, RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3, RESOURCE_ID_IMAGE_NUM_4, RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6, RESOURCE_ID_IMAGE_NUM_7, RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

const int IMAGE_RESOURCE_REVERSE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_NUM_0_REVERSE, RESOURCE_ID_IMAGE_NUM_1_REVERSE, RESOURCE_ID_IMAGE_NUM_2_REVERSE,
  RESOURCE_ID_IMAGE_NUM_3_REVERSE, RESOURCE_ID_IMAGE_NUM_4_REVERSE, RESOURCE_ID_IMAGE_NUM_5_REVERSE,
  RESOURCE_ID_IMAGE_NUM_6_REVERSE, RESOURCE_ID_IMAGE_NUM_7_REVERSE, RESOURCE_ID_IMAGE_NUM_8_REVERSE,
  RESOURCE_ID_IMAGE_NUM_9_REVERSE
};

BmpContainer image_containers[TOTAL_IMAGE_SLOTS];

#define EMPTY_SLOT -1

int image_slot_state[TOTAL_IMAGE_SLOTS] = {EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT};


void load_digit_image_into_slot(int slot_number, int digit_value, bool reversed) {


  if ((slot_number < 0) || (slot_number >= TOTAL_IMAGE_SLOTS)) {
    return;
  }

  if ((digit_value < 0) || (digit_value > 9)) {
    return;
  }

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    return;
  }

  image_slot_state[slot_number] = digit_value;
  if(reversed){
  	bmp_init_container(IMAGE_RESOURCE_REVERSE_IDS[digit_value], &image_containers[slot_number]);
  }else{
  	bmp_init_container(IMAGE_RESOURCE_IDS[digit_value], &image_containers[slot_number]);
  }
  image_containers[slot_number].layer.layer.frame.origin.x = ((slot_number % 4) * 36); 
  image_containers[slot_number].layer.layer.frame.origin.y = ((slot_number / 4) * 70)+15;//+15 for top padding
  layer_add_child(&window.layer, &image_containers[slot_number].layer.layer);

}


void unload_digit_image_from_slot(int slot_number) {

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    layer_remove_from_parent(&image_containers[slot_number].layer.layer);
    bmp_deinit_container(&image_containers[slot_number]);
    image_slot_state[slot_number] = EMPTY_SLOT;
  }

}


void display_value(unsigned short value, unsigned short row_number, bool show_first_leading_zero, bool isReversed, int startNum, int endNum) {

  value = value % 100;
  
  for (int column_number = startNum; column_number >= endNum; column_number--) {
    int slot_number = (row_number * 4) + column_number;
    if(isReversed){
    	if(slot_number == 0){
    		slot_number = 1;
    	}else if(slot_number == 1){
    		slot_number = 0;
    	}
    	if(slot_number == 6){
    		slot_number = 7;
    	}else if(slot_number == 7){
    		slot_number = 6;
    	}
    }
    unload_digit_image_from_slot(slot_number);
    if (!((value == 0) && (column_number == 0) && !show_first_leading_zero)) {
      load_digit_image_into_slot(slot_number, value % 10, isReversed);
    }
    value = value / 10;
  }
}


unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}


void display_time(PblTm *tick_time) {

  display_value(get_display_hour(tick_time->tm_hour), 0, true, true, 1, 0);
  display_value(get_display_hour(tick_time->tm_hour), 0, true, false, 3, 2);
  display_value(tick_time->tm_min, 1, true, false, 1, 0);
  display_value(tick_time->tm_min, 1, true, true, 3, 2);
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;

  display_time(t->tick_time);
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Rorschach");
  window_stack_push(&window, true);
  window_set_background_color(&window, GColorWhite);

  resource_init_current_app(&APP_RESOURCES);

  // Avoids a blank screen on watch start.
  PblTm tick_time;

  get_time(&tick_time);
  display_time(&tick_time);
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  for (int i = 0; i < TOTAL_IMAGE_SLOTS; i++) {
    unload_digit_image_from_slot(i);
  }

}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }

  };
  app_event_loop(params, &handlers);
}