#include "pebble.h"
//144 x 168

//#define NUM_MENU_SECTIONS 1
//#define NUM_FIRST_MENU_ITEMS 3
//#define NUM_SECOND_MENU_ITEMS 1
#define BUFSIZE 35
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
	ELEC_GRAPH=7,

        ELEC_CONS=8,
	ELEC_CONS_24=9,
	ELEC_CONS_VS_YESTERDAY=10,
	ELEC_CONS_WEEK=11,
	ELEC_CONS_VS_WEEK=12,
	ELEC_CONS_MONTH=13,
	ELEC_CONS_VS_MONTH=14,
	ELEC_CONS_GRAPH=15,
	
        ELEC_GEN=16,
	ELEC_GEN_24=17,
	ELEC_GEN_VS_YESTERDAY=18,
	ELEC_GEN_WEEK=19,
	ELEC_GEN_VS_WEEK=20,
	ELEC_GEN_MONTH=21,
	ELEC_GEN_VS_MONTH=22,
	ELEC_GEN_GRAPH=23,

	GAS=24,
	GAS_24=25,
	GAS_VS_YESTERDAY=26,
	GAS_WEEK=27,
	GAS_VS_WEEK=28,
	GAS_MONTH=29,
	GAS_VS_MONTH=30,
	GAS_GRAPH=31,

	WATER=32,
	WATER_24=33,
	WATER_VS_YESTERDAY=34,
	WATER_WEEK=35,
	WATER_VS_WEEK=36,
	WATER_MONTH=37,
	WATER_VS_MONTH=38
};
static char headers[5][32]={"Loading data...","","","",""};
static char header_details[5][32]={"\0","\0","\0","\0","\0"};
static char* sensor_data[39]={'\0','\0',"","","",","","","","",","","","","",","","","","",","","","","",","","","","",","","","",""};
static uint8_t* graph_data[5];
static uint8_t loaded=0;
static uint8_t debug=0;

static GPathInfo graph_points = {
  // This is the amount of points
  32,
  // A path can be concave, but it should not twist on itself
  // The points should be defined in clockwise order due to the rendering
  // implementation. Counter-clockwise will work in older firmwares, but
  // it is not officially supported
  (GPoint []) {
    {1, 71},
    {12, 69},
    {16, 69},
    {20, 69},
    {24, 69},
    {32, 69},
    {36, 69},
    {40, 69},
    {44, 69},
    {48, 69},
    {52, 69},
    {56, 69},
    {60, 69},
    {64, 69},
    {68, 69},
    {72, 69},
    {76, 69},
    {80, 69},
    {84, 69},
    {88, 69},
    {92, 69},
    {96, 69},
    {100, 69},
    {104, 69},
    {108, 69},
    {112, 69},
    {116, 69},
    {120, 69},
    {124, 69},
    {128, 69},
    {132, 69},
    {144, 71}
  }
};
static GPath *house_path;

#define NUM_GRAPHIC_PATHS 2

static GPath *graphic_paths[NUM_GRAPHIC_PATHS];

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
static TextLayer *text_layer_d1;
static TextLayer *text_layer_d2;
static TextLayer *text_layer_d3;
static TextLayer *text_layer_w1;
static TextLayer *text_layer_w2;
static TextLayer *text_layer_w3;
static TextLayer *text_layer_m1;
static TextLayer *text_layer_m2;
static TextLayer *text_layer_m3;
static TextLayer *text_layer_current;
static Layer *borderLayer;
static Layer *path_layer;
static Layer *bottomLayer;


// You can draw arbitrary things in a menu item such as a background
static GBitmap *menu_background;
static char header_title[32]="Loading data...";
static char buf[BUFSIZE]="";
static char buf_w[BUFSIZE]="";
static char buf_m[BUFSIZE]="";

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
  if(debug)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu selected ! %d",cell_index->row);
  selected_sensor=cell_index->row;
  if(loaded)
  {
    for(int i=1;i<31;i++)
    {
      graph_points.points[i].y=(int)graph_data[selected_sensor][i-1];
      if(debug)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Graph ! %d",graph_points.points[i].y);
      
    }
      
      window_stack_push(sensor_window, true);
  } 
//  switch (cell_index->row) {
//    // This is the menu item with the cycling icon
//    case 1:
//      layer_mark_dirty(menu_layer_get_layer(menu_layer));
//      break;
//  }

}
void menu_longclick_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if(debug)
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
static void draw_sensor_rect(Layer *me, GContext *ctx) {
    graphics_context_set_fill_color(ctx , GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    GRect bounds = layer_get_bounds(me);
    graphics_fill_rect(ctx,GRect(0, 0, bounds.size.w, bounds.size.h),4,GCornersAll );
  

}
static void draw_bottom_layer(Layer *me, GContext *ctx) {
    graphics_context_set_fill_color(ctx , GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    GRect bounds = layer_get_bounds(me);
    graphics_draw_line(ctx,GPoint(17, 5),GPoint(127,5));
    graphics_draw_line(ctx,GPoint(17, 6),GPoint(127,6));
  
    graphics_draw_line(ctx,GPoint(17, 28),GPoint(127,28));
    graphics_draw_line(ctx,GPoint(17, 29),GPoint(127,29));

}

// This initializes the menu upon window load
void sensor_window_load(Window *window) {
 // TODO !!! DO NOT FORGET TO COLLECT GARBAGE
    Layer *window_layer = window_get_root_layer(window);
    //GRect rect=GRect(0, 0, 50, 50);
    GRect rect = layer_get_frame(window_layer);

    borderLayer=layer_create(rect);
    layer_set_update_proc(borderLayer, draw_sensor_rect);
    layer_add_child(window_layer, borderLayer);


      // Cycle to the next path
    // Pass the corresponding GPathInfo to initialize a GPath
    current_path = gpath_create(&graph_points);
    //GRect bounds = layer_get_frame(window_layer);
    GRect bounds=GRect(0, 0, 142, 72);
    path_layer = layer_create(bounds);
    layer_set_update_proc(path_layer, path_layer_update_callback);
    layer_add_child(borderLayer, path_layer);

    text_layer_m1 = text_layer_create(GRect(0, 130, 16,18));
  	text_layer_set_text(text_layer_m1,"M:");
	  text_layer_set_font(text_layer_m1, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_m1, GTextAlignmentCenter);
  
    text_layer_m2 = text_layer_create(GRect(13, 130, 75,18));
  	text_layer_set_text(text_layer_m2,sensor_data[selected_sensor*8+5]);
	  text_layer_set_font(text_layer_m2, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_m2, GTextAlignmentRight);
    

    text_layer_m3 = text_layer_create(GRect(85, 130, 59,18));
  	text_layer_set_text(text_layer_m3,sensor_data[selected_sensor*8+6]);
	  text_layer_set_font(text_layer_m3, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_m3, GTextAlignmentRight);
    
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_m3));	
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_m2));	
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_m1));	

    text_layer_w1 = text_layer_create(GRect(0, 113, 16,18));
  	text_layer_set_text(text_layer_w1,"W:");
	  text_layer_set_font(text_layer_w1, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_w1, GTextAlignmentCenter);

    text_layer_w2 = text_layer_create(GRect(13, 113, 75,18));
  	text_layer_set_text(text_layer_w2,sensor_data[selected_sensor*8+3]);
	  text_layer_set_font(text_layer_w2, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_w2, GTextAlignmentRight);
    
    text_layer_w3 = text_layer_create(GRect(85, 113, 59,18));
  	text_layer_set_text(text_layer_w3,sensor_data[selected_sensor*8+4]);
	  text_layer_set_font(text_layer_w3, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_w3, GTextAlignmentRight);

    layer_add_child(borderLayer, text_layer_get_layer(text_layer_w3));	
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_w2));	
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_w1));	

    text_layer_d1 = text_layer_create(GRect(0, 96, 16,18));
  	text_layer_set_text(text_layer_d1,"D:");
	  text_layer_set_font(text_layer_d1, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_d1, GTextAlignmentCenter);

    text_layer_d2 = text_layer_create(GRect(13, 96, 75,18));
  	text_layer_set_text(text_layer_d2,sensor_data[selected_sensor*8+1]);
	  text_layer_set_font(text_layer_d2, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_d2, GTextAlignmentRight);

    text_layer_d3 = text_layer_create(GRect(85, 96, 59,18));
  	text_layer_set_text(text_layer_d3,sensor_data[selected_sensor*8+2]);
	  text_layer_set_font(text_layer_d3, fonts_get_system_font( FONT_KEY_GOTHIC_18_BOLD));
	  text_layer_set_text_alignment(text_layer_d3, GTextAlignmentRight);
    
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_d3));	
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_d2));	
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_d1));	

    rect = layer_get_frame(borderLayer);
    text_layer_current = text_layer_create(GRect(0, 70, rect.size.w,24));
  	text_layer_set_text(text_layer_current,sensor_data[selected_sensor*8]);
	  text_layer_set_font(text_layer_current, fonts_get_system_font( FONT_KEY_GOTHIC_24_BOLD));
	  text_layer_set_text_alignment(text_layer_current, GTextAlignmentCenter);
    layer_add_child(borderLayer, text_layer_get_layer(text_layer_current));	

  
  


    //layer_mark_dirty(menu_layer_get_layer(menu_layer));
    
    bottomLayer=layer_create(GRect(0,70,rect.size.w,rect.size.h-72));
    layer_set_update_proc(bottomLayer, draw_bottom_layer);
    layer_add_child(borderLayer, bottomLayer);

  
  

	  // App Logging!
    if(debug)
	    APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!  ");


}
void sensor_window_unload(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  layer_remove_child_layers(bottomLayer);
  layer_remove_child_layers(borderLayer);
  layer_remove_child_layers(window_layer);
  layer_destroy(borderLayer);
  layer_destroy(bottomLayer);
  layer_destroy(path_layer);
  text_layer_destroy(text_layer_current);
  text_layer_destroy(text_layer_d3);
  text_layer_destroy(text_layer_d2);
  text_layer_destroy(text_layer_d1);
  text_layer_destroy(text_layer_w1);
  text_layer_destroy(text_layer_w2);
  text_layer_destroy(text_layer_w3);
  text_layer_destroy(text_layer_m1);
  text_layer_destroy(text_layer_m2);
  text_layer_destroy(text_layer_m3);
  //destroy all layers
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
  loaded=0;
  strncpy(header_title, "Loading data....",32);
  layer_mark_dirty(menu_layer_get_layer(menu_layer));

}
// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	Tuple *tupleData;
 

  char time_txt[32];
  if(debug)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Just got message!");

  time_t t = time(NULL);
  struct tm *lt = localtime(&t);
  strftime(time_txt, 32, "Meters %H:%M:%S", lt);

  num_menu_sections=1;
  num_first_menu_items=0;
  loaded=1;
  for(int i=0;i<5;i++)
  {
    
    tuple = dict_find(received, i*8);
    if(tuple){
      strncpy(headers[num_first_menu_items],predefind_headers[i],30);
      sensor_data[i*8]=tuple->value->cstring;
      for(int j=i*8+1;j<i*8+7;j++)
      {
        tupleData=dict_find(received,j);
        if(tupleData)
        {
            sensor_data[j]=tupleData->value->cstring;
        }
        else
        {
            loaded=0;  
            strncpy(header_title, "Can't load data...",32);
        }
      }
      tupleData=dict_find(received,i*8+7);
      graph_data[i]=tupleData->value->data;
      if(loaded)
      {
        if(sensor_data[i*8+1])
        {
              char h_details[32]="";
              snprintf(h_details,30,"%s 24h:%s",sensor_data[i*8],sensor_data[i*8+1]);
              strncpy(header_details[num_first_menu_items],h_details,30);
         }
      }
//      for(int k=0;k<=i;k++)
//      {
//          APP_LOG(APP_LOG_LEVEL_DEBUG, header_details[k]);
//          APP_LOG(APP_LOG_LEVEL_DEBUG, "index %d",k);
//      }
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
    if(loaded)
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
  window_set_background_color(sensor_window, GColorBlack);
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
		

  window_destroy(sensor_window);
  window_destroy(window);
}
