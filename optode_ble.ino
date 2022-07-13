#include <ArduinoBLE.h>



// these MUST be defined here, outside
BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelChar("2A19", BLERead | BLENotify);



int oldBatteryLevel = 0;
long previousMillis = 0;



void setup()
{
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1);
  }


  // BLE services and chars
  batteryService.addCharacteristic(batteryLevelChar);
  BLE.addService(batteryService);
  batteryLevelChar.writeValue(oldBatteryLevel);


  // start BLE ADvertisement
  BLE.setLocalName("BLE_optode_1");
  BLE.setAdvertisedService(batteryService);
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
  Serial.println("my BLE MAC address is");
  Serial.println(BLE.address());


  // pins
  pinMode(LED_BUILTIN, OUTPUT);
}




void loop()
{
  // wait for a Bluetooth® Low Energy central
  BLEDevice central = BLE.central();


  if (central)
  {
    Serial.print("Connected to central: ");
    Serial.println(central.address());


    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, LOW);


    // check the battery level every 200ms while central connected
    while (central.connected())
    {
      long currentMillis = millis();
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
        updateBatteryLevel();
      }
    }


    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}



void updateBatteryLevel()
{
  int battery = analogRead(A0);
  int batteryLevel = map(battery, 0, 1023, 0, 100);
  if (batteryLevel != oldBatteryLevel)
  {
    Serial.print("Battery Level % is now: ");
    Serial.println(batteryLevel);
    batteryLevelChar.writeValue(batteryLevel);
    oldBatteryLevel = batteryLevel;
  }
}
