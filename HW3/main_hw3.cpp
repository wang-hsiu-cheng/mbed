#include "mbed.h"
#include "UTouch.h"
#include "lcd.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>

#include "drivers/DigitalOut.h"

#include "accelerometer.h"
#include "gyro.h"

#define LVGL_TICK 5 // Time tick value for lvgl in 5ms (1-10ms)
#define ITEMS 2
#define MAXIMUM_BUFFER_SIZE 6
#define waveformLength (128)
#define lookUpTableDelay (10)
#define bufferLength (32)

using namespace std::chrono;

// LCD           VCC GND  CS  RESET DC/RS  SDI/MOSI  SCK  SDO/MISO  LED
// mbed          5V, GND, D7, D6,   D8,    D11,      D13, D12,      3V3
LCD lcd(D6, D5, D8, D11, D13, D12);
// Touch             tclk,  tcs,          tdin,  dout,  irq
// mbed (L4S5I)PMOD. PD_1,  PD_2 (Buttom) PD_3,  PD_4,  PD_5
UTouch myTouch(PD_1, PD_2, PD_3, PD_4, PD_5);

// SPISlave device(PD_4, PD_3, PD_1, PD_0); // mosi, miso, sclk, cs; PMOD pins
// SPI spi(D11, D12, D13);                  // mosi, miso, sclk
// UTouch device(A5, A4, A3, A2); // mosi, miso, sclk, cs; PMOD pins
// UTouch spi(D5, D14, D15);      // mosi, miso, sclk

// DigitalOut cs(A0);
AnalogOut Aout(D7); // audio out
InterruptIn keyboard0(D2);
InterruptIn keyboard1(D3);
InterruptIn keyboard2(D4);
InterruptIn button(BUTTON1);
EventQueue queue(32 * EVENTS_EVENT_SIZE);

static BufferedSerial machine(D10, D9); // tx, rx  D10:tx  D9:rx
static BufferedSerial pc(D1, D0);       // tx, rx  D1:tx   D0:rx
static BufferedSerial serial_port(USBTX, USBRX);

Thread thread_master;
Thread thread_slave;
Thread t;

const microseconds TICKER_TIME = LVGL_TICK * 1ms; // modified to miliseconds 5000us=5ms

void lv_ticker_func();
void display_init(void);
void touchpad_init(void);
static void my_disp_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
static bool touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
void lv_demo_items();
void InputChange(int);
void playNote(int, int);
void add_item_amount(uint8_t);
uint8_t check_item_state();

const float frequency[]={261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
int itemPrice[ITEMS] = {20, 30};
int itemNumbers[ITEMS] = {5, 5};
int inputChange = 0;
int money = 0;
bool isAngleLarge = false;

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

int slave()
{
    // device.format(8, 3);
    // device.frequency(1000000);

    while (1)
    {
        // if (device.DataAvailable())
        // {
        //     // money += device.touch_ReadData(); // Read byte from master
        //     printf("%d\n", money);
        // }
        if (serial_port.readable())
        {
            char input;
            char toMachine;
            uint32_t num = serial_port.read(&input, 1);
            // printf("%c", input);
            // pc.write(&input, 1);
            // add_item_amount(0);
            // check_item_state();
            if (input == '0')
            {
                toMachine = 'c';
                pc.write(&toMachine, 1);
            }
            else if (input == '1')
            {
                toMachine = 's';
                pc.write(&toMachine, 1);
            }
        }
    }
}

void add_item_amount(uint8_t itemCode)
{
    itemNumbers[itemCode]++;
    lv_demo_items();
    return;
}
uint8_t check_item_state()
{
    int itemState = money * 100 + itemNumbers[0] * 10 + itemNumbers[1];
    pc.write(&itemState, 1);
    return (money * 100 + itemNumbers[0] * 10 + itemNumbers[1]);
}



void InputChange(int changes)
{
    switch(changes)
    {
        case 50:
            playNote(frequency[4], 100);
            break;
        case 10:
            playNote(frequency[5], 100);
            break;
        case 1:
            playNote(frequency[6], 100);
            break;
    }
    inputChange += changes;
    char str[40];
    sprintf(str, "%d", inputChange);
    char temp[80] = "changes:";
    strcat(temp, str);
    // lv_msgbox_set_text(mbox1, temp);
    lv_demo_items();
}

void playNote(int freq, int duration)
{
    int i = duration;
    int j = waveformLength;
    int waitTime = 1000000 / waveformLength / freq;
    printf("Play notes %d\n", freq);
    while (i--)
    {
        j = waveformLength;
        while (j--)
        {
            Aout = waveform[j];
            wait_us(waitTime);
        }
    }
}

void lv_ticker_func()
{
    lv_tick_inc(LVGL_TICK);
    // Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be between 1 and 10).
    // It is required for the internal timing of LittlevGL.
    lv_task_handler();
    // Call lv_task_handler() periodically every few milliseconds.
    // It will redraw the screen if required, handle input devices etc.
}

void display_init(void)
{
    // Init the touch screen display
    lcd.InitLCD();
    lcd.LCD_Clear(0x0);

    lv_init(); // Initialize the LittlevGL
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10];                  // Declare a buffer for 10 lines
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10); // Initialize the display buffer

    // Implement and register a function which can copy a pixel array to an area of your display
    lv_disp_drv_t disp_drv;               // Descriptor of a display driver
    lv_disp_drv_init(&disp_drv);          // Basic initialization
    disp_drv.flush_cb = my_disp_flush_cb; // Set your driver function
    disp_drv.buffer = &disp_buf;          // Assign the buffer to the display
    lv_disp_drv_register(&disp_drv);      // Finally register the driver
}

void my_disp_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
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
    myTouch.SetPrecision(PREC_HI);
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
    return false; // false: no more data to read because we are no buffering
}

lv_obj_t *screenMain;
lv_obj_t *mbox1;
lv_obj_t *mbox2;
char str[80];
char str1[80];

static void event_handler_btn1(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        if (inputChange <= 0)
        {
            char temp[100] = "not enough money";
            lv_msgbox_set_text(mbox1, temp);
            printf("not enough money\n");
            return;
        }
        inputChange -= itemPrice[0];
        itemNumbers[0] -= 1;
        // cs = 1;
        // cs = 0;
        // spi.touch_WriteData(itemPrice[0]);
        char temp[100] = "Cola change: ";
        sprintf(str, "%d", inputChange);
        strcat(temp, str);
        lv_msgbox_set_text(mbox1, temp);
        printf("cola change: %d\n", inputChange);
        playNote(131, 100);
        sprintf(str, "%d", itemPrice[0]);
        sprintf(str1, "%d", itemNumbers[0]);
        char ch1[100] = "Cola price: ";
        char ch2[30] = " amount: ";
        strcat(ch1, str);
        strcat(ch1, ch2);
        strcat(ch1, str1);
        // lv_label_set_text(label1, ch1);
        lv_demo_items();
        inputChange = 0;
    }
}

static void event_handler_btn2(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        if (inputChange < itemPrice[1])
        {
            char temp[100] = "not enough money";
            lv_msgbox_set_text(mbox1, temp);
            printf("not enough money\n");
            return;
        }
        inputChange -= itemPrice[1];
        itemNumbers[1] -= 1;
        // cs = 1;
        // cs = 0;
        // spi.touch_WriteData(itemPrice[1]);
        char temp[100] = "7up change: ";
        sprintf(str, "%d", inputChange);
        strcat(temp, str);
        lv_msgbox_set_text(mbox1, temp);
        printf("7up change: %d\n", inputChange);
        playNote(131, 100);
        sprintf(str, "%d", itemPrice[1]);
        sprintf(str1, "%d", itemNumbers[1]);
        char ch1[100] = "7up price: ";
        char ch2[30] = " amount: ";
        strcat(ch1, str);
        strcat(ch1, ch2);
        strcat(ch1, str1);
        // lv_label_set_text(label2, ch1);
        lv_demo_items();
        inputChange = 0;
    }
}

void lv_demo_items()
{

    lv_obj_t *btn1;
    lv_obj_t *btn2;

    lv_obj_t *label;
    screenMain = lv_obj_create(NULL, NULL);

    label = lv_label_create(screenMain, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_label_set_text(label, "vending machine");
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_size(label, 240, 40);
    lv_obj_set_pos(label, 0, 15);

    btn1 = lv_btn_create(screenMain, NULL);
    lv_obj_set_event_cb(btn1, event_handler_btn1);
    lv_obj_set_width(btn1, 240);
    lv_obj_set_height(btn1, 32);
    lv_obj_set_pos(btn1, 32, 100);

    lv_obj_t *label1 = lv_label_create(btn1, NULL);
    sprintf(str, "%d", itemPrice[0]);
    sprintf(str1, "%d", itemNumbers[0]);
    char ch1[100] = "Cola price: ";
    char ch2[30] = " amount: ";
    strcat(ch1, str);
    strcat(ch1, ch2);
    strcat(ch1, str1);
    lv_label_set_text(label1, ch1);
    printf("price: %d left: %d\n", itemPrice[0], itemNumbers[0]);

    btn2 = lv_btn_create(screenMain, NULL);
    lv_obj_set_event_cb(btn2, event_handler_btn2);
    lv_obj_set_width(btn2, 240);
    lv_obj_set_height(btn2, 32);
    lv_obj_set_pos(btn2, 32, 150);

    lv_obj_t *label2 = lv_label_create(btn2, NULL);
    sprintf(str, "%d", itemPrice[1]);
    sprintf(str1, "%d", itemNumbers[1]);
    char ch3[100] = "7up price: ";
    char ch4[30] = " amount: ";
    strcat(ch3, str);
    strcat(ch3, ch4);
    strcat(ch3, str1);
    lv_label_set_text(label2, ch3);
    printf("price: %d left: %d\n", itemPrice[1], itemNumbers[1]);

    mbox1 = lv_msgbox_create(screenMain, NULL);
    sprintf(str, "%d", inputChange);
    char temp[80] = "changes:";
    strcat(temp, str);
    lv_msgbox_set_text(mbox1, temp);
    lv_obj_set_width(mbox1, 400);
    lv_obj_set_pos(mbox1, 0, 200);
    printf("changes: %d \n", inputChange);

    mbox2 = lv_msgbox_create(screenMain, NULL);
    lv_obj_set_width(mbox2, 200);
    lv_obj_set_pos(mbox2, 0, 300);
    if (isAngleLarge)
        lv_msgbox_set_text(mbox2, "angle warning");
    else
        lv_msgbox_set_text(mbox2, " ");

    lv_scr_load(screenMain);
}
// void lv_demo_mbox2()
// {
//     mbox2 = lv_msgbox_create(screenMain, NULL);
//     lv_obj_set_width(mbox2, 200);
//     lv_obj_set_pos(mbox2, 0, 300);
//     if (isAngleLarge)
//         lv_msgbox_set_text(mbox2, "angle warning");
//     else
//         lv_msgbox_set_text(mbox2, " ");
//     lv_scr_load(screenMain);
// }


// int main()
// {
//     // Set desired properties (9600-8-N-1).
//     machine.set_baud(9600);
//     machine.set_format(
//         /* bits */ 8,
//         /* parity */ BufferedSerial::None,
//         /* stop bit */ 1);

//     // Set desired properties (9600-8-N-1).
//     pc.set_baud(9600);
//     pc.set_format(
//         /* bits */ 8,
//         /* parity */ BufferedSerial::None,
//         /* stop bit */ 1);

//     thread_slave.start(slave);
//     // thread_master.start(master);
// }
int main()
{
    // spi.format(8, 3);
    // spi.frequency(1000000);

    printf("LittlevGL DEMO\n");
    display_init();
    touchpad_init();
    lv_demo_items();
    // lv_demo_mbox2();

    Accelerometer accelerometer;
    Gyro gyroscope;
    double rawAccelerationData[3];
    double calibratedAccelerationData[3];
    double rawGyroData[3];
    double calibratedGyroData[3];

    t.start(callback(&queue, &EventQueue::dispatch_forever));
    keyboard0.fall(queue.event(InputChange, 50));
    keyboard1.fall(queue.event(InputChange, 10));
    keyboard2.fall(queue.event(InputChange, 1));

        // Set desired properties (9600-8-N-1).
    machine.set_baud(9600);
    machine.set_format(
        /* bits */ 8,
        /* parity */ BufferedSerial::None,
        /* stop bit */ 1);

    // Set desired properties (9600-8-N-1).
    pc.set_baud(9600);
    pc.set_format(
        /* bits */ 8,
        /* parity */ BufferedSerial::None,
        /* stop bit */ 1);

    thread_slave.start(slave);

    while (true)
    {
        lv_ticker_func();
        ThisThread::sleep_for(5ms);

        if (machine.readable())
        {
            char receive;
            machine.read(&receive, 1);
            if (receive == 'c')
            {
                add_item_amount(0);
                check_item_state();
            }
            else if (receive == 's')
            {
                add_item_amount(1);
                check_item_state();
            }
            printf("%c\n", receive);
        }
        // Read raw gyroscope data
        gyroscope.GetGyroSensor(rawGyroData);
        // Calibrate gyroscope data
        gyroscope.GetGyroCalibratedData(calibratedGyroData);

        if (calibratedGyroData[2] > 1 && !isAngleLarge)
        {
            isAngleLarge = true;
            lv_demo_items();
            // lv_msgbox_set_text(mbox2, "angle warning");
            
        }
        else if (calibratedGyroData[2] <= 1 && isAngleLarge)
        {
            isAngleLarge = false;
           lv_demo_items();
            // lv_msgbox_set_text(mbox2, " ");
            
        }
    }
}