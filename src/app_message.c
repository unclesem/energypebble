#include "pebble.h"

//#define NUM_MENU_SECTIONS 1
//#define NUM_FIRST_MENU_ITEMS 3
//#define NUM_SECOND_MENU_ITEMS 1
#define BUFSIZE 25
#define ELECTRICITY_TOTAL_TITLE "Electricity total"
#define ELECTRICITY_CONS_TITLE "Elect. Consuption"
#define ELECTRICITY_GEN_TITLE "Elect. Generation"
#define GAS_TITLE "Gas"
#define WATER_TITLE "Water"
  
// Key values for AppMessage Dictionary
enum {
	ELEC=0,
	ELEC_24=1,
	ELEC_VS_YESTERDAY=2,
	ELEC_WEEK=3,
	ELEC_VS_WEEK=4,
	ELEC_MONTH=5,
	ELEC_VS_MONTH=6,

  ELEC_CONS=7,
	ELEC_CONS_24=8,
	ELEC_CONS_VS_YESTERDAY=9,
	ELEC_CONS_WEEK=10,
	ELEC_CONS_VS_WEEK=11,
	ELEC_CONS_MONTH=12,
	ELEC_CONS_VS_MONTH=13,
	
  ELEC_GEN=14,
	ELEC_GEN_24=15,
	ELEC_GEN_VS_YESTERDAY=16,
	ELEC_GEN_WEEK=17,
	ELEC_GEN_VS_WEEK=18,
	ELEC_GEN_MONTH=19,
	ELEC_GEN_VS_MONTH=20,

	GAS=21,
	GAS_24=22,
	GAS_VS_YESTERDAY=23,
	GAS_WEEK=24,
	GAS_VS_WEEK=25,
	GAS_MONTH=26,
	GAS_VS_MONTH=27,

	WATER=28,
	WATER_24=29,
	WATER_VS_YESTERDAY=30,
	WATER_WEEK=31,
	WATER_VS_WEEK=32,
	WATER_MONTH=33,
	WATER_VS_MONTH=34
};
static char* headers[5]={"Loading data...","","","",""};

static char* item1_header="Loading data...";
static char* item1_current;
static char* item1_24="";
static char* item2_header;
static char* item2_current;
static char* item2_24="";
static char* item3_header;
static char* item3_current;
static char* item3_24="";
static char* item4_header;
static char* item4_current;
static char* item4_24="";
static char* item5_header;
static char* item5_current;
static char* item5_24="";

//Window elements
static Window *window;
static Window *sensor_window;
static int selected_sensor = 0;
//menu descrition

static uint num_menu_sections=1;
static uint num_first_menu_items=1;
static uint num_second_menu_items=0;
static uint num_third_menu_items=0;
  
// This is a menu layer
// You have more control than with a simple menu layer
static MenuLayer *menu_layer;
static TextLayer *text_layer;


// You can draw arbitrary things in a menu item such as a background
static GBitmap *menu_background;
static char buf[BUFSIZE]="";
static char header_title[32]="Loading data...";

void init_buf(char *buf, size_t size);
void print_buf(char *buf);


// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return num_menu_sections;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return num_first_menu_items;

    case 1:
      return num_second_menu_items;
    case 2:
      return num_third_menu_items;

    default:
      return 0;
  }
}

// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, header_title);
      break;

 //   case 1:
 //     menu_cell_basic_header_draw(ctx, cell_layer, "One more");
 //     break;
  }
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  char details[20]="";
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu details %s",details);
  if(item1_current&&item1_24){
   APP_LOG(APP_LOG_LEVEL_DEBUG, "item1_current %s",item1_current);
   APP_LOG(APP_LOG_LEVEL_DEBUG, "item1_24 %s",item1_24);
  }
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          // This is a basic menu item with a title and subtitle
       if(item1_current&&item1_24)
            snprintf(details,20,"%s 24h:%s",item1_current,item1_24);
          menu_cell_basic_draw(ctx, cell_layer, item1_header, details, NULL);
        break;

        case 1:
         if(item2_current&&item2_24)
            snprintf(details,20,"%s 24h:%s",item2_current,item2_24);
       
          // This is a basic menu icon with a cycling icon
          menu_cell_basic_draw(ctx, cell_layer, item2_header, details, NULL);
          break;
        case 2:
         if(item3_current&&item3_24)
            snprintf(details,20,"%s 24h:%s",item3_current,item3_24);
          // Here we use the graphics context to draw something different
          // In this case, we show a strip of a watchface's background
          menu_cell_basic_draw(ctx, cell_layer, item3_header, details, NULL);
          break;
        case 3:
         if(item4_current&&item4_24)
            snprintf(details,20,"%s 24h:%s",item4_current,item4_24);
          // Here we use the graphics context to draw something different
          // In this case, we show a strip of a watchface's background
          menu_cell_basic_draw(ctx, cell_layer, item4_header, details, NULL);
          break;
        case 4:
         if(item4_current&&item4_24)
            snprintf(details,20,"%s 24h:%s",item4_current,item4_24);
          // Here we use the graphics context to draw something different
          // In this case, we show a strip of a watchface's background
          menu_cell_basic_draw(ctx, cell_layer, item5_header, details, NULL);
          break;
       
      }
      break;
    
//    Title item
//    case 1:
//      switch (cell_index->row) {
//        case 0:
//          // There is title draw for something more simple than a basic menu item
//          menu_cell_basic_draw(ctx, cell_layer, "Final Item","1231",NULL);
//          break;
//      }
  }
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu selected ! %d",cell_index->row);
  selected_sensor=cell_index->row;
  window_stack_push(sensor_window, true);
  
//  switch (cell_index->row) {
//    // This is the menu item with the cycling icon
//    case 1:
//      layer_mark_dirty(menu_layer_get_layer(menu_layer));
//      break;
//  }

}

// This initializes the menu upon window load
void window_load(Window *window) {
  // Here we load the bitmap assets
  // resource_init_current_app must be called before all asset loading


  // Now we prepare to initialize the menu layer
  // We need the bounds to specify the menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  menu_layer = menu_layer_create(bounds);

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}
void window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(menu_layer);
}

void sensor_window_unload(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  layer_remove_child_layers(window_layer);
  text_layer_destroy(text_layer);
}

// This initializes the menu upon window load
void sensor_window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    snprintf(buf, BUFSIZE, "bla bla %d" ,selected_sensor);
	  APP_LOG(APP_LOG_LEVEL_DEBUG, buf);
    text_layer = text_layer_create(GRect(0, 0, 144, 154));
  	text_layer_set_text(text_layer,buf);
	  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));	
    layer_mark_dirty(menu_layer_get_layer(menu_layer));

	  // App Logging!
	  APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!  ");
}
static WindowHandlers sensor_window_handlers = {
  .load = sensor_window_load,
  .unload = sensor_window_unload
};

// Write message to buffer & send
void send_message(void){
  
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_cstring(iter, ELEC, "getData");
	
	dict_write_end(iter);
 	app_message_outbox_send();
}
// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	Tuple *tupleData;

  char time_txt[32];
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Just got message!");

  time_t t = time(NULL);
  struct tm *lt = localtime(&t);
  strftime(time_txt, 32, "Meters %H:%M:%S", lt);

  num_menu_sections=1;
  num_first_menu_items=0;
  tuple = dict_find(received, ELEC);
  if(tuple){
    num_first_menu_items++;
    
    item1_current=tuple->value->cstring;
    tupleData=dict_find(received,ELEC_24);
    if(tupleData)
    {
      item1_24=tupleData->value->cstring;
    }
    item1_header=ELECTRICITY_TOTAL_TITLE;
  }

  tuple = dict_find(received, ELEC_CONS);
  if(tuple){
    num_first_menu_items++;
    if(num_first_menu_items==1){
      item1_header=ELECTRICITY_CONS_TITLE;
      item1_current=tuple->value->cstring;

    }
    else
      {
      item2_header=ELECTRICITY_CONS_TITLE;
      item2_current=tuple->value->cstring;

    }

  }
  tuple = dict_find(received, ELEC_GEN);
  if(tuple){
    num_first_menu_items++;
    if(num_first_menu_items==1){
      item1_header=ELECTRICITY_GEN_TITLE;
      item1_current=tuple->value->cstring;
    }
    else if(num_first_menu_items==2)
    {
      item2_header=ELECTRICITY_GEN_TITLE;
      item2_current=tuple->value->cstring;

    }
    else{
          item3_header=ELECTRICITY_GEN_TITLE;
          item3_current=tuple->value->cstring;
    } 
  }
  tuple = dict_find(received, GAS);
  if(tuple){
    num_first_menu_items++;
    if(num_first_menu_items==1){
      item1_header=GAS_TITLE;
          item1_current=tuple->value->cstring;
}
    else if(num_first_menu_items==2){
      item2_header=GAS_TITLE;
            item2_current=tuple->value->cstring;

    }
    else if(num_first_menu_items==3){
      item3_header=GAS_TITLE;
            item3_current=tuple->value->cstring;

    }
    else{
      item4_header=GAS_TITLE;
            item4_current=tuple->value->cstring;

    }
  }
  tuple = dict_find(received, WATER);
  if(tuple){
    num_first_menu_items++;
    if(num_first_menu_items==1)
      {
      item1_header=WATER_TITLE;
      item1_current=tuple->value->cstring;
    }
      else if(num_first_menu_items==2){
      item2_header=WATER_TITLE;
      item2_current=tuple->value->cstring;
    }
      else if(num_first_menu_items==3){
      item3_header=WATER_TITLE;
      item3_current=tuple->value->cstring;
    }
      else if(num_first_menu_items==4){
      item4_header=WATER_TITLE;
      item4_current=tuple->value->cstring;
    }
      else {
      item5_header=WATER_TITLE;
      item5_current=tuple->value->cstring;
    }
  }

	
	/*
  tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %s", tuple->value->cstring); 
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status MSG: %d", (int)tuple->value->uint32); 
    // Set the text, font, and text alignment
  	//text_layer_set_text(text_layer, tuple->value->cstring);
	  //text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	  //text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	  //layer_remove_child_layers(window_get_root_layer(window));
	  // Add the text layer to the window
	  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));

	  // Push the window
	  //window_stack_push(window, true);
	
    
	  // App Logging!
    strncpy(buf,tuple->value->cstring,sizeof(buf));
    //snprintf(buf, BUFSIZE,  "Received: %s", tuple->value->cstring);
	  APP_LOG(APP_LOG_LEVEL_DEBUG, buf);
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	}
*/
    strncpy(header_title, time_txt,32);
 
    menu_layer_reload_data(menu_layer);
    layer_mark_dirty(menu_layer_get_layer(menu_layer));

}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

int main(void) {
  window = window_create();

  sensor_window = window_create();
  window_set_window_handlers(sensor_window, sensor_window_handlers);

  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
	window_stack_push(window, true /* Animated */);

  app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  send_message();	

  app_event_loop();
  	 //Register AppMessage handlers
		

  window_destroy(window);
}
