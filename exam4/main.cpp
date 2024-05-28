#include "mbed.h"
#include "drivers/DigitalOut.h"

#include "erpc_simple_server.hpp"
#include "erpc_basic_codec.hpp"
#include "erpc_crc16.hpp"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "exam4_server.h"

#include "accelerometer.h"

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "gatt_server_process.h"
#include "mbed-trace/mbed_trace.h"

#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

using mbed::callback;
using namespace std::literals::chrono_literals;
static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);

mbed::DigitalOut led1(LED1, 1);
mbed::DigitalOut led2(LED2, 1);
mbed::DigitalOut led3(LED3, 1);
mbed::DigitalOut* leds[] = { &led1, &led2, &led3 };

Accelerometer accelerometer;
double rawAccelerationData[3];
double calibratedAccelerationData[3];
int rate;
bool isSampling = false;

WiFiInterface *wifi;
InterruptIn btn2(BUTTON1);
//InterruptIn btn3(SW3);
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;

const char* topic0 = "start";
const char* topic1 = "stop";
const char* topic2 = "see";

Thread mqtt_thread(osPriorityHigh);
EventQueue mqtt_queue;

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char msg[300];
    sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf(msg);
    ThisThread::sleep_for(2000ms);
    char payload[300];
    sprintf(payload, "Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    printf(payload);
    ++arrivedcount;
}

void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {
    // Accelerometer accelerometer;
    // double calibratedAccelerationData[3];
    message_num++;
    MQTT::Message message;
    char buff[100];
    // accelerometer.GetAcceleromterCalibratedData(calibratedAccelerationData);
    sprintf(buff, "QoS0 Hello, Python! #%d\n %g, %g, %g\n", message_num, calibratedAccelerationData[0], calibratedAccelerationData[1], calibratedAccelerationData[2]);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) buff;
    message.payloadlen = strlen(buff) + 1;
    int c = client->publish(topic2, message);

    printf("rc:  %d\r\n", c);
    printf("Puslish message: %s\r\n", buff);
}

void close_mqtt() {
    closed = true;
}


class GattServerDemo : ble::GattServer::EventHandler {

    const static uint16_t EXAMPLE_SERVICE_UUID         = 0xA390;
    const static uint16_t WRITABLE_CHARACTERISTIC_UUID = 0xA391;

public:
    GattServerDemo()
    {
        const UUID uuid = WRITABLE_CHARACTERISTIC_UUID;
        _writable_characteristic = new ReadWriteGattCharacteristic<uint8_t> (uuid, &_characteristic_value);

        if (!_writable_characteristic) {
            printf("Allocation of ReadWriteGattCharacteristic failed\r\n");
        }
    }

    ~GattServerDemo()
    {
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        const UUID uuid = EXAMPLE_SERVICE_UUID;
        GattCharacteristic* charTable[] = { _writable_characteristic };
        GattService example_service(uuid, charTable, 1);

        ble.gattServer().addService(example_service);

        ble.gattServer().setEventHandler(this);

        printf("Example service added with UUID 0xA000\r\n");
        printf("Connect and write to characteristic 0xA001\r\n");
    }

private:
    /**
     * This callback allows the LEDService to receive updates to the ledState Characteristic.
     *
     * @param[in] params Information about the characterisitc being updated.
     */
    virtual void onDataWritten(const GattWriteCallbackParams &params)
    {
        if ((params.handle == _writable_characteristic->getValueHandle()) && (params.len == 1)) {
            printf("New characteristic value written: %c\r\n", *(params.data));
            StartSampling((int)*params.data - 48);
        }
    }

private:
    ReadWriteGattCharacteristic<uint8_t> *_writable_characteristic = nullptr;
    uint8_t _characteristic_value = 0;
};

/****** erpc declarations *******/

void StartSampling(uint8_t tr)
{
    printf("e");
    rate  = tr;
    isSampling = true;
}
void StopSampling()
{
    isSampling = false;
}
uint16_t SensorData()
{
    int x, y, z;
    x = rawAccelerationData[0] * 1000;
    y = rawAccelerationData[1] * 1000;
    z = rawAccelerationData[2] * 1000;
    return x + 1000 * y + 1000000 * z;
}

/** erpc infrastructure */
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;

/** LED service */
exam4Service_service exam4_service;


int main(void) {

    mbed_trace_init();

    BLE &ble = BLE::Instance();

    printf("\r\nGattServer demo of a writable characteristic\r\n");

    GattServerDemo demo;

    /* this process will handle basic setup and advertising for us */
    GattServerProcess ble_process(event_queue, ble);

    /* once it's done it will let us continue with our demo*/
          ble_process.on_init(callback(&demo, &GattServerDemo::start));

    ble_process.start();
    // Initialize the rpc server
    uart_transport.setCrc16(&crc16);

    printf("Initializing server.\n");
    rpc_server.setTransport(&uart_transport);
    rpc_server.setCodecFactory(&basic_cf);
    rpc_server.setMessageBufferFactory(&dynamic_mbf);

    // Add the led service to the server
    printf("Adding LED server.\n");
    rpc_server.addService(&exam4_service);
    

    // Run the server. This should never exit
    printf("Running server.\n");
    rpc_server.run();

    while (isSampling)
    {
        accelerometer.GetAcceleromterSensor(rawAccelerationData);
        for (int i = 0; i < 1000/rate; i++)
            ThisThread::sleep_for(1ms);
    }
}