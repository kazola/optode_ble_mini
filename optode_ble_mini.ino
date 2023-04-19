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
#define _SP                     Serial.print
#define _SPN                    Serial.println
#define _dW                     digitalWrite
#define _SERIAL_FIELD_WORK_     0
#define _SERIAL_DEVELOPING_     1
#define _SERIAL_ENABLE_         _SERIAL_FIELD_WORK_



// must be defined here, outside
BLEService svc("2323");
BLECharacteristic char_out("2324", BLERead | BLENotify, 10);
BLECharacteristic  char_in("2325", BLERead | BLEWrite, 10);



static void _mini_led_on()
{
    // LED in the black mini board is active LOW
    _dW(LED_BUILTIN, LOW);
}



static void _mini_led_off()
{
    _dW(LED_BUILTIN, HIGH);  
}



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
    _dW(PIN_DISPLAY_OUT, 1);
    delay(3000);
    _dW(PIN_DISPLAY_OUT, 0);
    char_out.writeValue("do_ok");
}



// wi-fi out button is active low
void _act_wo()
{
    _dW(PIN_WIFI_OUT, 0);
    delay(100);
    _dW(PIN_WIFI_OUT, 1);
    char_out.writeValue("wo_ok");
}



// display button but quicker to make wheels scan
void _act_wheel()
{
    _dW(PIN_DISPLAY_OUT, 1);
    delay(100);
    _dW(PIN_DISPLAY_OUT, 0);
    char_out.writeValue("wh_ok");
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
        _mini_led_on();
        delay(100);
        _mini_led_off();
        delay(100);
    }
    

    // end up w/ LED on
    _mini_led_on();
    

    const char * a = "le_ok";
    char_out.writeValue(a);
    _SP("<- ");
    _SPN(a);
}



static void _leds_used_for_debug()
{
    while (1)
    {
      _mini_led_on();
      delay(1000);
      _mini_led_off();
      delay(1000);
    }
}



static void _leds_boot_sequence()
{
    for(uint8_t i = 0; i < 10; i++)
    {
      _mini_led_on();
      delay(100);
      _mini_led_off();
      delay(100);      
    }
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



    #if _SERIAL_ENABLE_ == _SERIAL_DEVELOPING_
        // we DON'T want to enter here when _SERIAL_FIELD_WORK_    
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


    // boot visual hint
    _leds_boot_sequence();


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
        _mini_led_on();


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

                if (!strncmp("wh", v, len))
                {
                    _act_wheel();
                }
            }            
        }


        // BLE disconnection
        _SP("Disconnected from central ");
        _SPN(central.address());
        _mini_led_off();

  }
}
