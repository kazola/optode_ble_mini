// -----------------------------------------------------------------------------------
// src: https://wiki.seeedstudio.com/XIAO_BLE
// needs installing
//     - Arduino IDE
//     - preferences, add board sources
//         https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
//     - install board seeed nrf52 mbed
//     - install library ArduinoBLE
// -----------------------------------------------------------------------------------




#include <ArduinoBLE.h>



// pinout
//                   ___
//                  |USB|
//                ---------               
// di ---> a0 d0 |         | 5v
// wi ---> a1 d1 |         | gnd
//         a2 d2 |         | 3v3
//         a3 d3 |         | d10 ---> do
//         a4 d4 |         | d9  ---> wo
//         a5 d5 |         | d8
//            d6 |         | d7
//                ---------



// pins
#define PIN_DISPLAY_OUT   D10
#define PIN_WIFI_OUT      D9
#define PIN_DISPLAY_IN    A0
#define PIN_WIFI_IN       A1


// macros
#define _SP                   Serial.print
#define _SPN                  Serial.println
#define _dW                   digitalWrite
#define _SERIAL_FIELD_WORK_   0
#define _SERIAL_DEVELOPING_   1
#define _SERIAL_ENABLE        _SERIAL_FIELD_WORK_



// must be defined here, outside
BLEService svc("2323");
BLECharacteristic char_out("2324", BLERead | BLENotify, 10);
BLECharacteristic  char_in("2325", BLERead | BLEWrite, 10);



void _build_name(char * s)
{
    const char * _m = BLE.address().c_str();
    strncpy(s, "op_mi_", 6);
    strncpy(s + 6, _m + 0, 2);
    strncpy(s + 8, _m + 3, 2);
    strncpy(s + 10, _m + 6, 2);
    strncpy(s + 12, _m + 9, 2);
    strncpy(s + 14, _m + 12, 2);
    strncpy(s + 16, _m + 15, 2);
}



// display out button is active high
void _act_do()
{
    char_out.writeValue("do_ok");
    _dW(PIN_DISPLAY_OUT, 1);
    delay(3000);
    _dW(PIN_DISPLAY_OUT, 0);
}



// wi-fi out button is active low
void _act_wo()
{
    char_out.writeValue("wo_ok");
    _dW(PIN_WIFI_OUT, 0);
    delay(100);
    _dW(PIN_WIFI_OUT, 1);
}



void _act_di()
{
    int a = analogRead(PIN_DISPLAY_IN);
    _SP("adc display value: ");
    _SPN(a);
    if (a > 512) char_out.writeValue("di_on");
    else char_out.writeValue("di_of");
}



void _act_wi()
{
    int a = analogRead(PIN_WIFI_IN);
    _SP("adc display value: ");
    _SPN(a);
    if (a > 512) char_out.writeValue("wi_on");
    else char_out.writeValue("wi_of");
}



void _act_led()
{
    for (uint8_t i = 0; i < 10; i++)
    {
        _dW(LED_BUILTIN, HIGH);
        delay(100);
        _dW(LED_BUILTIN, LOW);
        delay(100);
    }
    

    // end up w/ LED on
    _dW(LED_BUILTIN, LOW);


    const char * a = "le_ok";
    char_out.writeValue(a);
    _SP("<- ");
    _SPN(a);
}



void setup()
{

    // pins
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_DISPLAY_OUT, OUTPUT);
    pinMode(PIN_WIFI_OUT, OUTPUT);



    // default states for buttons
    _dW(PIN_DISPLAY_OUT, 0);
    _dW(PIN_WIFI_OUT, 1);



    // serial 1 when _SERIAL_DEVELOPING_, 0 when _SERIAL_FIELD_WORK_
    #if _SERIAL_ENABLE == _SERIAL_FIELD_WORK_
        Serial.begin(9600);
        while (!Serial);
    #endif


    // hardware check
    if (!BLE.begin())
    {
        _SPN("starting BLE failed!");
        while (1);
    }
    _SP("BLE optode scan boot, my MAC is ");
    _SPN(BLE.address());



    // services and characteristics
    svc.addCharacteristic(char_out);
    svc.addCharacteristic(char_in);
    BLE.addService(svc);


    // CONN (* 1.25) -> 800 = 1s, 100 = 125 ms
    BLE.setConnectionInterval(100, 100);



    // ADV (* 0.625) -> 320 = 200 ms
    char _name[20] = {0};
    _build_name(_name);
    BLE.setLocalName(_name);
    BLE.setDeviceName(_name);
    BLE.setAdvertisedService(svc);
    BLE.setAdvertisingInterval(160);
    BLE.advertise();
}




void loop()
{
    BLEDevice central = BLE.central();
    if (central)
    {
        _SP("\n\n\nConnected to central ");
        _SPN(central.address());


        // on BLE connection, LED = LOW = shining
        _dW(LED_BUILTIN, LOW);


        // -----------------
        // command loop
        // -----------------
        
        while (central.connected())
        {
            if (char_in.written())
            {
                // ignore bad commands
                int len = char_in.valueLength();
                if (len > 2) continue;


                // ---------------------------
                // all commands have length 2
                // ---------------------------
        
                // get command and parse it
                char v[10] = {0};
                strncpy(v, (const char *)char_in.value(), len);
                _SP("-> ");
                _SPN(v);
        
        
                // parse command
                if (!strncmp("do", v, len))
                {
                    _act_do();
                }

                if (!strncmp("wo", v, len))
                {
                    _act_wo();
                }
                
                if (!strncmp("di", v, len))
                {
                    _act_di();
                }
                
                if (!strncmp("wi", v, len))
                {
                    _act_wi();
                }
                
                if (!strncmp("le", v, len))
                {
                    _act_led();
                }
            }            
        }


        // BLE disconnection
        _SP("Disconnected from central ");
        _SPN(central.address());
        _dW(LED_BUILTIN, HIGH);
  }
}
