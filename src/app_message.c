#include "pebble.h"
//144 x 168

//#define NUM_MENU_SECTIONS 1
//#define NUM_FIRST_MENU_ITEMS 3
//#define NUM_SECOND_MENU_ITEMS 1
#define BUFSIZE 25
#define ELECTRICITY_TOTAL_TITLE "Electricity total"
#define ELECTRICITY_CONS_TITLE "Elect. Consuption"
#define ELECTRICITY_GEN_TITLE "Elect. Generation"
#define GAS_TITLE "Gas"
#define WATER_TITLE "Water"
  
static char*  predefind_headers[5]={ELECTRICITY_TOTAL_TITLE,ELECTRICITY_CONS_TITLE,ELECTRICITY_GEN_TITLE,GAS_TITLE,WATER_TITLE};
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
static char headers[5][32]={"Loading data...","","","",""};
static char header_details[5][32]={"","","","",""};
static char* sensor_data[35]={'\0','\0',"","","",","","","","",","","","","",","","","","",","","","","",","","","","",","","","",""};
static const GPathInfo HOUSE_PATH_POINTS = {
  // This is the amount of points
  24,
  // A path can be concave, but it should not twist on itself
  // The points should be defined in clockwise order due to the rendering
  // implementation. Counter-clockwise will work in older firmwares, but
  // it is not officially supported
  (GPoint []) {
    {0, 110},
    {2, 20},
    {4, 75},
    {6, 5},
    {8, 80},
    {10, 50},
    {12, 56},
    {14, 56},
    {16, 20},
    {18, 80},
    {20, 100},
    {66, 110},
    {72, 20},
    {78, 75},
    {84, 5},
    {90, 80},
    {96, 50},
    {102, 56},
    {108, 56},
    {114, 20},
    {120, 100},
    {126, 10},
    {132, 90},
    {144, 110}
  }
};
static GPath *house_path;

#define NUM_GRAPHIC_PATHS 2

static GPath *graphic_paths[NUM_GRAPHIC_PATHS];

//Window elements
static Window *window;
static Window *sensor_window;
static Layer *path_layer;
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
static char header_title[32]="Loading data...";
static char buf[BUFSIZE]="";

static GPath *current_path = NULL;
static int current_path_index = 0;
static int path_angle = 0;
static bool outline_mode = true;

void send_message(void);


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
  if(cell_index->section==0&&cell_index->row<5)
  {
   // APP_LOG(APP_LOG_LEVEL_DEBUG, "index %d",cell_index->row);
   // APP_LOG(APP_LOG_LEVEL_DEBUG, header_details[cell_index->row]);
    menu_cell_basic_draw(ctx, cell_layer, headers[cell_index->row], header_details[cell_index->row], NULL);
  }    
//    Title item
//    case 1:
//      switch (cell_index->row) {
//        case 0:
//          // There is title draw for something more simple than a basic menu item
//          menu_cell_basic_draw(ctx, cell_layer, "Final Item","1231",NULL);
//          break;
//      }
  
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
void menu_longclick_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu longclick ! %d",cell_index->row);
  send_message();
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
    .select_long_click = menu_longclick_callback
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
// This is the layer update callback which is called on render updates
static void path_layer_update_callback(Layer *me, GContext *ctx) {
  (void)me;

  // You can rotate the path before rendering
  //gpath_rotate_to(current_path, (TRIG_MAX_ANGLE / 360) * path_angle);

  // There are two ways you can draw a GPath: outline or filled
  // In this example, only one or the other is drawn, but you can draw
  // multiple instances of the same path filled or outline.
  if (outline_mode) {
    // draw outline uses the stroke color
    graphics_context_set_stroke_color(ctx, GColorBlack);
    gpath_draw_outline(ctx, current_path);
  } else {
    // draw filled uses the fill color
    graphics_context_set_fill_color(ctx, GColorBlack);
    gpath_draw_filled(ctx, current_path);
  }
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
    
  
  
    // Cycle to the next path
    // Pass the corresponding GPathInfo to initialize a GPath
    current_path = gpath_create(&HOUSE_PATH_POINTS);
    GRect bounds = layer_get_frame(window_layer);
    path_layer = layer_create(bounds);
    layer_set_update_proc(path_layer, path_layer_update_callback);
    layer_add_child(window_layer, path_layer);

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
  strncpy(header_title, "Loading data....",32);
  layer_mark_dirty(menu_layer_get_layer(menu_layer));

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
  for(int i=0;i<5;i++)
  {
    
    tuple = dict_find(received, i*7);
    if(tuple){
      strncpy(headers[num_first_menu_items],predefind_headers[i],30);
      sensor_data[i*7]=tuple->value->cstring;
      for(int j=i*7+1;j<i*7+7;j++)
      {
        tupleData=dict_find(received,j);
        if(tupleData)
        {
            sensor_data[j]=tupleData->value->cstring;
        }
      }
      if(sensor_data[i*7+1])
      {
         char h_details[32]="";
          snprintf(h_details,30,"%s 24h:%s",sensor_data[i*7],sensor_data[i*7+1]);
          strncpy(header_details[num_first_menu_items],h_details,30);
      }
     
      for(int k=0;k<=i;k++)
      {
          APP_LOG(APP_LOG_LEVEL_DEBUG, header_details[k]);
          APP_LOG(APP_LOG_LEVEL_DEBUG, "index %d",k);
      }
      num_first_menu_items++;
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
