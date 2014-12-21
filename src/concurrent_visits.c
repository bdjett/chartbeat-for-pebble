#include "common.h"
  
static Window *s_window;
static TextLayer *s_data_text_layer;
static TextLayer *s_label_text_layer;
static AppTimer *s_timer;
static Layer *s_graph_layer;
static int32_t max_visitors;
static int32_t visitors;

#define REFRESH_TIME 5000

// Request data from JavaScript
static void get_data(void) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  if (iter == NULL) {
    return;
  }
  
  dict_write_int16(iter, GET_DATA, 1);
  app_message_outbox_send();
}

// Received AppMessage
void concurrent_visits_in_received_handler(DictionaryIterator *iter) {
  Tuple *people_tuple = dict_find(iter, PEOPLE);
  Tuple *people_string_tuple = dict_find(iter, PEOPLE_STRING);
  Tuple *max_tuple = dict_find(iter, MAX_PEOPLE);
  if (people_tuple) {
    visitors = people_tuple->value->int32;
    text_layer_set_text(s_data_text_layer, people_string_tuple->value->cstring);
    max_visitors = max_tuple->value->int32;
    layer_mark_dirty(s_graph_layer);
  }
}

// Timer handler
static void timer_callback(void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Getting data");
  get_data();
  s_timer = app_timer_register(REFRESH_TIME, timer_callback, NULL);
}

// Circular graph layer update procedure
void graph_layer_update_proc(Layer *my_layer, GContext* ctx) {
  // Fill the path:
  graphics_context_set_fill_color(ctx, GColorBlack);
  if (visitors > 0) {
    graphics_draw_arc(ctx, GPoint(62, 60), 60, 5, 0, (((double)visitors / max_visitors) * 360));
  }
  graphics_draw_arc(ctx, GPoint(62, 60), 60, 1, 0, 360);
}

// Initialize all UI components
static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, true);
  
  visitors = 0;
  max_visitors = 0;
  
  s_data_text_layer = text_layer_create(GRect(0, 54, 144, 35));
  text_layer_set_text_alignment(s_data_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_data_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_data_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_data_text_layer, "Loading...");
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_data_text_layer));
  
  s_label_text_layer = text_layer_create(GRect(0, 136, 144, 35));
  text_layer_set_text_alignment(s_label_text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_label_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(s_label_text_layer, GColorBlack);
  text_layer_set_text_color(s_label_text_layer, GColorWhite);
  text_layer_set_font(s_label_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_label_text_layer, "Concurrent Visits");
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_label_text_layer));
  
  s_graph_layer = layer_create(GRect(10, 10, 124, 121));
  layer_set_update_proc(s_graph_layer, graph_layer_update_proc);
  layer_add_child(window_get_root_layer(s_window), s_graph_layer);
  
  get_data();
  s_timer = app_timer_register(REFRESH_TIME, timer_callback, NULL);
}

// Free all memory form UI components
static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_data_text_layer);
  text_layer_destroy(s_label_text_layer);
  app_timer_cancel(s_timer);
}

// Window unload callback
static void handle_window_unload(Window* window) {
  destroy_ui();
}

// Show window
void show_concurrent_visits(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

// Hide window
void hide_concurrent_visits(void) {
  window_stack_remove(s_window, true);
}