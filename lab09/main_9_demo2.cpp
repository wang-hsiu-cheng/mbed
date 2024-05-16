#include "mbed.h"
using namespace std::chrono_literals;
#include <string>

#include "UTouch.h"
#include "lcd.h"
#include "lvgl/lvgl.h"

using namespace std::chrono;

//---------------------------------------------------------------------------------------------
// LCD and Touch Setup
// LCD           VCC GND  CS  RESET DC/RS  SDI/MOSI  SCK  SDO/MISO  LED
// mbed                   D10,  D9,  D8,    D11,     D13, D12,      3V3
// CS  RESET DC/RS  SDI/MOSI  SCK  SDO/MISO
LCD lcd(D10, D9, D8, D11, D13, D12);

// Touch             tclk,  tcs,          tdin,  dout,  irq
// mbed (L4S5I)PMOD. PD_1,  PD_2 (Buttom) PD_3,  PD_4,  PD_5
UTouch myTouch(PD_1, PD_2, PD_3, PD_4, PD_5);
#define LVGL_TICK 5                               // Time tick value for lvgl in 5ms (1-10ms)
const microseconds TICKER_TIME = LVGL_TICK * 1ms; // modified to miliseconds 5000us=5ms

//---------------------------------------------------------------------------------------------
// lvgl Setup
Ticker lvgl_ticker;
EventQueue queue_lvgl(32 * EVENTS_EVENT_SIZE);
Thread t_lvgl;
void lv_ticker_func();
void display_init(void);
void touchpad_init(void);
static void my_disp_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                             lv_color_t *color_p);
static bool touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
void lv_demo_widgets();
//---------------------------------------------------------------------------------------------

#define waveformLength (128)
#define lookUpTableDelay (1)
#define bufferLength (32)

AnalogOut Aout(PA_4);

float waveform[waveformLength] = { // 128 samples of a sine waveform
    0.500, 0.525, 0.549, 0.574, 0.598, 0.622, 0.646, 0.670, 0.693, 0.715, 0.737,
    0.759, 0.780, 0.800, 0.819, 0.838, 0.856, 0.873, 0.889, 0.904, 0.918, 0.931,
    0.943, 0.954, 0.964, 0.972, 0.980, 0.986, 0.991, 0.995, 0.998, 1.000, 1.000,
    0.999, 0.997, 0.994, 0.989, 0.983, 0.976, 0.968, 0.959, 0.949, 0.937, 0.925,
    0.911, 0.896, 0.881, 0.864, 0.847, 0.829, 0.810, 0.790, 0.769, 0.748, 0.726,
    0.704, 0.681, 0.658, 0.634, 0.610, 0.586, 0.562, 0.537, 0.512, 0.488, 0.463,
    0.438, 0.414, 0.390, 0.366, 0.342, 0.319, 0.296, 0.274, 0.252, 0.231, 0.210,
    0.190, 0.171, 0.153, 0.136, 0.119, 0.104, 0.089, 0.075, 0.063, 0.051, 0.041,
    0.032, 0.024, 0.017, 0.011, 0.006, 0.003, 0.001, 0.000, 0.000, 0.002, 0.005,
    0.009, 0.014, 0.020, 0.028, 0.036, 0.046, 0.057, 0.069, 0.082, 0.096, 0.111,
    0.127, 0.144, 0.162, 0.181, 0.200, 0.220, 0.241, 0.263, 0.285, 0.307, 0.330,
    0.354, 0.378, 0.402, 0.426, 0.451, 0.475, 0.500};

Mutex dac_mutex;

void playNote(int freq, float vol, bool isPoen)
{
    if (!isPoen)
        return;
    int j = waveformLength;
    int waitTime = 1000000 / waveformLength / freq;
    int i = 0.3 * freq; // play i iterations of waveform samples
    // printf("Sample iteration=%d\n", i);
    printf("Play notes %d\n", freq);
    dac_mutex.lock(); // To prevent other calls to playNote to use DAC
    while (i--)
    {
        j = waveformLength;
        while (j--)
        {
            Aout = waveform[j] * vol; // scale with volume
            wait_us(waitTime);
        }
    }
    dac_mutex.unlock();
}

// EventQueue for calling playNote()
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;
const int notes = 8; // support 8 notes
int id[notes];       // create an thread ID for each note

string NoteName[] = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};
// frequency table for C4, D4, E4, F4, G4, A4, B4, C5
const float frequency[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};

int main(void)
{
    printf("Touch keyboard demo\n");
    display_init();
    touchpad_init();
    // Call lv_tick_inc(x) every x (1-10) milliseconds in a Timer or Task.
    // It is required for the internal timing of LittlevGL.
    lvgl_ticker.attach(callback(&lv_ticker_func), 5ms);
    t_lvgl.start(callback(&queue_lvgl, &EventQueue::dispatch_forever));
    // Call lv_task_handler() periodically every few milliseconds.
    // It will redraw the screen if required, handle input devices etc.
    // Lower priority than lv_tick_inc
    queue_lvgl.call_every(5ms, lv_task_handler); // 5ms should match LVGL_TICK 5
    lv_demo_widgets();
    t.start(callback(&queue, &EventQueue::dispatch_forever));
}

void lv_ticker_func(void)
{
    lv_tick_inc(LVGL_TICK);
}

void display_init(void)
{
    // Init the touch screen display

    lcd.InitLCD();
    lcd.LCD_Clear(0x0);

    lv_init(); // Initialize the LittlevGL
    static lv_disp_buf_t disp_buf;
    // Double buffering
    static lv_color_t buf1[LV_HOR_RES_MAX * 48]; // Declare a buffer for 48 lines (1/10 of vertical resoluion)
    static lv_color_t buf2[LV_HOR_RES_MAX * 48]; // Declare a buffer for 48 lines (1/10 of vertical resoluion) (ping-pong buffer)
    lv_disp_buf_init(&disp_buf, buf1, buf2,
                     LV_HOR_RES_MAX * 48); // Initialize the display buffer

    // Implement and register a function which can copy a pixel array to an area
    // of your display
    lv_disp_drv_t disp_drv;               // Descriptor of a display driver
    lv_disp_drv_init(&disp_drv);          // Basic initialization
    disp_drv.flush_cb = my_disp_flush_cb; // Set your driver function
    disp_drv.buffer = &disp_buf;          // Assign the buffer to the display
    lv_disp_drv_register(&disp_drv);      // Finally register the driver
}

void my_disp_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                      lv_color_t *color_p)
{
    // cast lv_color_t to uint16_t pointer
    uint16_t *colors = reinterpret_cast<uint16_t *>(color_p);
    // Put all pixels to the screen one-by-one by FillArea for a specified area
    lcd.FillArea(area->x1, area->y1, area->x2, area->y2, colors);
    // IMPORTANT!!!* Inform the graphics library that you are ready with the flushing
    lv_disp_flush_ready(disp_drv);
}

void touchpad_init(void)
{
    myTouch.InitTouch();
    myTouch.SetPrecision(PREC_MEDIUM);
    lv_indev_drv_t indev_drv;               // Descriptor of an input device driver
    lv_indev_drv_init(&indev_drv);          // Basic initialization
    indev_drv.type = LV_INDEV_TYPE_POINTER; // The touchpad is pointer type device
    indev_drv.read_cb = touchpad_read;      // Set the touchpad_read function
    lv_indev_drv_register(&indev_drv);      // Register touch driver in LvGL
}

static bool touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
    // Read your touchpad
    if (myTouch.DataAvailable())
    {
        if (myTouch.Read())
        {
            data->point.x = myTouch.GetX();
            data->point.y = myTouch.GetY();
            data->state = LV_INDEV_STATE_PR;
        }
    }
    else
    {
        data->point.x = 0;
        data->point.y = 0;
        data->state = LV_INDEV_STATE_REL;
    }
    return false; // false: no data to read
}

lv_obj_t *screenMain;
lv_obj_t *mbox1;
lv_obj_t *buttons[notes];
lv_obj_t *label[notes];

int lastIndex = -1;
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    int index = -1;
    // Search matched buttons object index
    for (int i = 0; i < notes; i++)
    {
        if (obj == buttons[i])
        {
            index = i;
            break;
        }
    }
    // if (event != LV_EVENT_RELEASED)
    if (event == LV_EVENT_PRESSING || event == LV_EVENT_PRESSED)
    { // Trigger call_every only at the first press
        string note = "Playing " + string(NoteName[index]);
        // printf(note.c_str());
        lv_msgbox_set_text(mbox1, note.c_str());

        if (index != -1)
        { // Check if notes index is supported otherwise do nothing
            // queue.dispatch();
            // queue.cancel(id[index]);
            id[index] = queue.call_every(10ms, playNote, frequency[index], true, 3.3); // call playNote for 1s once
            lastIndex = index;
            // printf("Call playNote index=%d\n", index);
        }
        else
        {
            queue.cancel(id[index]);
            // printf("Exception: index=%d\n", index);
        }
    }
    // else if (event != LV_EVENT_PRESSED) {
    //     // string note="Stop playing "+string(NoteName[index]);
    //     // //printf(note.c_str());
    //     // lv_msgbox_set_text(mbox1, note.c_str());

    //     // if(index!=-1){ //Check if notes index is supported otherwise do nothing
    //     //     queue.cancel(id[index]); //stop playing a note
    //     //     //printf("Cancel playNote index=%d\n", index);
    //     // }else{
    //     //     //printf("Exception: index=%d\n", index);
    //     // }
    // }
    else if (event == LV_EVENT_RELEASED)
    {
        string note = "Stop playing " + string(NoteName[index]);
        // printf(note.c_str());
        lv_msgbox_set_text(mbox1, note.c_str());

        if (index != -1)
        {                            // Check if notes index is supported otherwise do nothing
            queue.cancel(id[index]); // stop playing a note
            // printf("Cancel playNote index=%d\n", index);
        }
        else
        {
            // printf("Exception: index=%d\n", index);
        }
    }
}

void lv_demo_widgets()
{

    screenMain = lv_obj_create(NULL, NULL);
    for (int i = 0; i < notes; i++)
    {
        buttons[i] = lv_btn_create(screenMain, NULL);
        lv_obj_set_event_cb(buttons[i], event_handler);
        label[i] = lv_label_create(buttons[i], NULL);
        lv_label_set_text(label[i], NoteName[i].c_str());
        lv_label_set_align(label[i], LV_LABEL_ALIGN_CENTER);
        lv_obj_set_size(buttons[i], 240, 36);
        lv_obj_align(buttons[i], NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -100 - 40 * i);
    }

    mbox1 = lv_msgbox_create(screenMain, NULL);
    lv_msgbox_set_text(mbox1, "No message");
    lv_obj_set_width(mbox1, 240);
    lv_obj_align(mbox1, buttons[0], LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    lv_scr_load(screenMain);
}