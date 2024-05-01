#include "mbed.h"
#include "UTouch.h"
#include "lcd.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>

#include "drivers/DigitalOut.h"

// #include "accelerometer.h"
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
void InputCode(int);
void CheckPassword();
uint8_t check_item_state();

int itemPrice[ITEMS] = {20, 30};
int itemNumbers[ITEMS] = {5, 5};
int inputChange = 0;
int money = 0;
int student_ID = 111061237;
char books[3][100] = {"__", "__", "__"};
bool isAngleLarge = false;
int buyBooks[10];
int buyBooksNumber = 0;
int currentBook = 10;
int password[3] = {0, 0, 0};
int passwordNumber = 0;
int correctPassword[3] = {1, 1, 1};
bool isCorrect = false;

int slave()
{
    while (1)
    {
        if (serial_port.readable())
        {
            char check;
            serial_port.read(&check, 1);
            if (check == '1')
            {
                int input;
                uint32_t num = serial_port.read(&input, 1);
                serial_port.read((char *)&num, sizeof(num));
                // Print the received integer
                printf("Received integer: %d\n", num);
                student_ID = input;
                lv_demo_items();
            }
            for (int i = 0; i < 3; i++)
            {
                int num_bytes = serial_port.read(buffer, sizeof(buffer));
                if (num_bytes > 0)
                {
                    // Null-terminate the buffer to make it a string
                    buffer[num_bytes] = '\0';
                    // Print the received string
                    printf("Received string: %s\n", buffer);
                    for (int j = 0; j < num_bytes; j++)
                        book[i][j] = buffer[j];
                }
            }
        }
        // pc.write(&input, 1);

        // if (input == '0')
        // {
        //     toMachine = 'c';
        //     pc.write(&toMachine, 1);
        // }
        // else if (input == '1')
        // {
        //     toMachine = 's';
        //     pc.write(&toMachine, 1);
        // }
    }
}
}

uint8_t check_item_state()
{
    int itemState = money * 100 + itemNumbers[0] * 10 + itemNumbers[1];
    pc.write(&itemState, 1);
    return (money * 100 + itemNumbers[0] * 10 + itemNumbers[1]);
}

void CheckPassword()
{
    int checkBit = 0;
    for (int i = 0; i < 3; i++)
    {
        if (password[i] == correctPassword[i])
            checkBit++;
    }
    if (checkBit == 3)
        isCorrect = true;
    else
        isCorrect = false;
    return;
}

void InputCode(int code)
{
    password[passwordNumber] = code;
    passwordNumber++;
    printf("%d\n", code);
    if (passwordNumber == 3)
        void CheckPassword();
    lv_demo_items();
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
        lv_msgbox_set_text(mbox1, books[0]);
        // printf("cola change: %d\n", inputChange);
        currentBook = 0;
        // lv_demo_items();
    }
}

static void event_handler_btn2(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        lv_msgbox_set_text(mbox1, books[1]);
        currentBook = 1;
        // lv_demo_items();
    }
}
static void event_handler_btn3(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        lv_msgbox_set_text(mbox1, books[2]);
        currentBook = 2;
        // lv_demo_items();
    }
}

static void event_handler_buy_bt(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        if (currentBook == 0 || currentBook == 1 || currentBook == 2)
        {
            buyBooks[buyBooksNumber] = currentBook;
            buyBooksNumber++;
            printf("currentBook: %s\n", books[buyBooks[buyBooksNumber]]);
            lv_msgbox_set_text(mbox1, books[buyBooks[buyBooksNumber]]);
            // lv_demo_items();
        }
    }
}

void lv_demo_items()
{

    lv_obj_t *btn1;
    lv_obj_t *btn2;
    lv_obj_t *btn3;
    lv_obj_t *buy_bt;
    lv_obj_t *label;
    lv_obj_t *label1;
    lv_obj_t *label2;
    lv_obj_t *label3;
    lv_obj_t *label_buy;
    screenMain = lv_obj_create(NULL, NULL);

    label = lv_label_create(screenMain, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    sprintf(str, "%d", student_ID);
    lv_label_set_text(label, str);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_size(label, 240, 40);
    lv_obj_set_pos(label, 0, 15);

    btn1 = lv_btn_create(screenMain, NULL);
    lv_obj_set_event_cb(btn1, event_handler_btn1);
    lv_obj_set_width(btn1, 70);
    lv_obj_set_height(btn1, 30);
    lv_obj_set_pos(btn1, 10, 100);

    label1 = lv_label_create(btn1, NULL);
    lv_label_set_text(label1, books[0]);
    // printf("price: %d left: %d\n", itemPrice[0], itemNumbers[0]);

    btn2 = lv_btn_create(screenMain, NULL);
    lv_obj_set_event_cb(btn2, event_handler_btn2);
    lv_obj_set_width(btn2, 70);
    lv_obj_set_height(btn2, 30);
    lv_obj_set_pos(btn2, 90, 100);

    label2 = lv_label_create(btn2, NULL);
    lv_label_set_text(label2, books[1]);
    // printf("price: %d left: %d\n", itemPrice[1], itemNumbers[1]);

    btn3 = lv_btn_create(screenMain, NULL);
    lv_obj_set_event_cb(btn3, event_handler_btn3);
    lv_obj_set_width(btn3, 70);
    lv_obj_set_height(btn3, 30);
    lv_obj_set_pos(btn3, 10, 150);

    label3 = lv_label_create(btn3, NULL);
    lv_label_set_text(label3, books[2]);
    // printf("price: %d left: %d\n", itemPrice[0], itemNumbers[0]);

    buy_bt = lv_btn_create(screenMain, NULL);
    lv_obj_set_event_cb(buy_bt, event_handler_buy_bt);
    lv_obj_set_width(buy_bt, 70);
    lv_obj_set_height(buy_bt, 30);
    lv_obj_set_pos(buy_bt, 90, 150);

    label_buy = lv_label_create(buy_bt, NULL);
    lv_label_set_text(label_buy, "buy");

    mbox1 = lv_msgbox_create(screenMain, NULL);
    if (currentBook == 0 || currentBook == 1 || currentBook == 2)
        lv_msgbox_set_text(mbox1, books[currentBook]);
    lv_obj_set_width(mbox1, 400);
    lv_obj_set_pos(mbox1, 0, 300);

    mbox2 = lv_msgbox_create(screenMain, NULL);
    lv_obj_set_width(mbox2, 200);
    lv_obj_set_pos(mbox2, 0, 430);
    char temp[80] = "password: ";
    for (int i = 0; i < 3; i++)
    {
        sprintf(str, "%d", password[i]);
        strcat(temp, str);
    }
    lv_msgbox_set_text(mbox2, temp);
    if (passwordNumber == 2 && isCorrect)
    {
        lv_msgbox_set_text(mbox2, "correct");
        passwordNumber = 0;
    }
    else if (passwordNumber == 2 && !isCorrect)
    {
        lv_msgbox_set_text(mbox2, "wrong");
        passwordNumber = 0;
    }

    lv_scr_load(screenMain);
}

int main()
{
    printf("LittlevGL DEMO\n");
    display_init();
    touchpad_init();
    lv_demo_items();

    // Accelerometer accelerometer;
    // Gyro gyroscope;
    // double rawAccelerationData[3];
    // double calibratedAccelerationData[3];
    // double rawGyroData[3];
    // double calibratedGyroData[3];

    t.start(callback(&queue, &EventQueue::dispatch_forever));
    keyboard0.fall(queue.event(InputCode, 1));
    keyboard1.fall(queue.event(InputCode, 2));
    keyboard2.fall(queue.event(InputCode, 3));

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

        // if (machine.readable())
        // {
        //     char receive;
        //     machine.read(&receive, 1);
        //     if (receive == 'c')
        //     {
        //         add_item_amount(0);
        //         check_item_state();
        //     }
        //     else if (receive == 's')
        //     {
        //         add_item_amount(1);
        //         check_item_state();
        //     }
        //     printf("%c\n", receive);
        // }
        // // Read raw gyroscope data
        // gyroscope.GetGyroSensor(rawGyroData);
        // // Calibrate gyroscope data
        // gyroscope.GetGyroCalibratedData(calibratedGyroData);

        // if (calibratedGyroData[2] > 1 && !isAngleLarge)
        // {
        //     isAngleLarge = true;
        //     lv_demo_items();
        //     // lv_msgbox_set_text(mbox2, "angle warning");

        // }
        // else if (calibratedGyroData[2] <= 1 && isAngleLarge)
        // {
        //     isAngleLarge = false;
        //    lv_demo_items();
        //     // lv_msgbox_set_text(mbox2, " ");

        // }
    }
}