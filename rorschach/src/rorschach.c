#include <pebble.h>

Window *window;

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

BitmapLayer *image_containers[TOTAL_IMAGE_SLOTS];
GBitmap     *bitmaps[TOTAL_IMAGE_SLOTS];

void load_digit_image_into_slot(int slot_number, int digit_value, bool reversed) {

  if ((slot_number < 0) || (slot_number >= TOTAL_IMAGE_SLOTS)) {
    return;
  }

  if ((digit_value < 0) || (digit_value > 9)) {
    return;
  }

  if(bitmaps[slot_number]){
    return;
  }

  if(reversed){
    bitmaps[slot_number] = gbitmap_create_with_resource(IMAGE_RESOURCE_REVERSE_IDS[digit_value]);
  }else{
    bitmaps[slot_number] = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[digit_value]);
  }
  bitmap_layer_set_bitmap(image_containers[slot_number],bitmaps[slot_number]);
}


void unload_digit_image_from_slot(int slot_number) {
  if (bitmaps[slot_number]) {
    gbitmap_destroy(bitmaps[slot_number]);
    bitmaps[slot_number] = NULL;
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

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  display_value(get_display_hour(tick_time->tm_hour), 0, true, true, 1, 0);
  display_value(get_display_hour(tick_time->tm_hour), 0, true, false, 3, 2);
  display_value(tick_time->tm_min, 1, true, false, 1, 0);
  display_value(tick_time->tm_min, 1, true, true, 3, 2);

  layer_mark_dirty(window_get_root_layer(window));
}


void handle_init() {

  window = window_create();
  
  window_set_background_color(window, GColorWhite);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  for (int i = 0; i < TOTAL_IMAGE_SLOTS; i++) {
    bitmaps[i] = NULL;
    image_containers[i] = bitmap_layer_create(GRect(
      ((i % 4) * 36),
      ((i / 4) * 70)+15,
      36,70));
    layer_add_child(window_layer, bitmap_layer_get_layer(image_containers[i]));
  }

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  window_stack_push(window, true);
}


void handle_deinit() {
  for (int i = 0; i < TOTAL_IMAGE_SLOTS; i++) {
    unload_digit_image_from_slot(i);
    bitmap_layer_destroy(image_containers[i]);
  }
  window_destroy(window);
}


int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}