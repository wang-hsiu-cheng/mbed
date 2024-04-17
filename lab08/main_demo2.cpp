#include "mbed.h"
#include "UTouch.h"
#include "lcd.h"
#include "lvgl/lvgl.h"
#include <string.h>

using namespace std::chrono;
//LCD           VCC GND  CS  RESET DC/RS  SDI/MOSI  SCK  SDO/MISO  LED
//mbed                   D10,  D9,  D8,    D11,     D13, D12,      3V3

//CS  RESET DC/RS  SDI/MOSI  SCK  SDO/MISO
LCD lcd(D10, D9, D8, D11, D13, D12);

//Touch             tclk,  tcs,          tdin,  dout,  irq
//mbed (L4S5I)PMOD. PD_1,  PD_2 (Buttom) PD_3,  PD_4,  PD_5
UTouch  myTouch(PD_1, PD_2, PD_3, PD_4, PD_5);

#define LVGL_TICK 5                             //Time tick value for lvgl in 5ms (1-10ms)
const microseconds TICKER_TIME=LVGL_TICK*1ms; //modified to miliseconds 5000us=5ms

void lv_ticker_func();
void display_init(void);
void touchpad_init(void);
static void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
static bool touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
void lv_demo_widgets();

// main() runs in its own thread in the OS
int main()
{
    printf("LittlevGL DEMO\n");
    display_init();
    touchpad_init();

    lv_demo_widgets();

    while (true) {
        lv_ticker_func();
        ThisThread::sleep_for(5ms);
    }
}

void lv_ticker_func(){
    lv_tick_inc(LVGL_TICK);
    //Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be between 1 and 10).
    //It is required for the internal timing of LittlevGL.
    lv_task_handler();
    //Call lv_task_handler() periodically every few milliseconds.
    //It will redraw the screen if required, handle input devices etc.
}

void display_init(void){
    //Init the touch screen display
    lcd.InitLCD();
    lcd.LCD_Clear(0x0);

    lv_init();                                  //Initialize the LittlevGL
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10]; //Declare a buffer for 10 lines
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10); //Initialize the display buffer

    //Implement and register a function which can copy a pixel array to an area of your display
    lv_disp_drv_t disp_drv;                     //Descriptor of a display driver
    lv_disp_drv_init(&disp_drv);                //Basic initialization
    disp_drv.flush_cb = my_disp_flush_cb;       //Set your driver function
    disp_drv.buffer = &disp_buf;                //Assign the buffer to the display
    lv_disp_drv_register(&disp_drv);            //Finally register the driver
}

void my_disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p)
{
    //cast lv_color_t to uint16_t pointer
    uint16_t* colors=reinterpret_cast<uint16_t*> (color_p);
    //Put all pixels to the screen one-by-one by FillArea for a specified area
    lcd.FillArea(area->x1, area->y1, area->x2, area->y2, colors);
    //IMPORTANT!!!* Inform the graphics library that you are ready with the flushing
    lv_disp_flush_ready(disp_drv);
}

void touchpad_init(void){
    myTouch.InitTouch();
    myTouch.SetPrecision(PREC_HI);
    lv_indev_drv_t indev_drv;                       //Descriptor of an input device driver
    lv_indev_drv_init(&indev_drv);                  //Basic initialization
    indev_drv.type = LV_INDEV_TYPE_POINTER;         //The touchpad is pointer type device
    indev_drv.read_cb = touchpad_read;              //Set the touchpad_read function
    lv_indev_drv_register(&indev_drv);              //Register touch driver in LvGL
}

static bool touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data){
    // Read your touchpad
    if (myTouch.DataAvailable()) {
       if(myTouch.Read()) {
         data->point.x = myTouch.GetX();
         data->point.y = myTouch.GetY();
         data->state = LV_INDEV_STATE_PR;
       }
     }else {
        data->point.x = 0;
        data->point.y = 0;
        data->state = LV_INDEV_STATE_REL;
    }
    return false;   //false: no more data to read because we are no buffering
}

lv_obj_t *screenMain;
lv_obj_t * mbox1;
int counter = 0;

static void event_handler_btn1(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
        char str[80];
        sprintf(str, "%d", counter);
        lv_msgbox_set_text(mbox1, str);
        counter++;
    }
}

static void event_handler_btn2(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
       lv_msgbox_set_text(mbox1, "Goodbye");
    }
}

void lv_demo_widgets(){

    lv_obj_t *btn1;
    lv_obj_t *btn2;

    lv_obj_t *label;
    screenMain = lv_obj_create(NULL, NULL);

    label = lv_label_create(screenMain, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_label_set_text(label, "Press a button");
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_size(label, 240, 40);
    lv_obj_set_pos(label, 0, 15);

    btn1 = lv_btn_create(screenMain, NULL);
    lv_obj_set_event_cb(btn1, event_handler_btn1);
    lv_obj_set_width(btn1, 70);
    lv_obj_set_height(btn1, 32);
    lv_obj_set_pos(btn1, 32, 100);

    lv_obj_t * label1 = lv_label_create(btn1, NULL);
    lv_label_set_text(label1, "Click");

//   btn2 = lv_btn_create(screenMain, NULL);
//   lv_obj_set_event_cb(btn2, event_handler_btn2);
//   lv_obj_set_width(btn2, 70);
//   lv_obj_set_height(btn2, 32);
//   lv_obj_set_pos(btn2, 142, 100);

//   lv_obj_t * label2 = lv_label_create(btn2, NULL);
//   lv_label_set_text(label2, "Goodbye");

    mbox1 = lv_msgbox_create(screenMain, NULL);
    lv_msgbox_set_text(mbox1, "No message");
    lv_obj_set_width(mbox1, 200);
    lv_obj_set_pos(mbox1, 0, 200);

    lv_scr_load(screenMain);
}