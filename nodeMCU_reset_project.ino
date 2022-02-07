#include <DHT.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = ""; // ENTER NETWORK SSID
char pass[] = ""; // ENTER NETWORK PASSWORD

char auth_key[] = ""; //ENTER BLYNK AUTHENTICATION KEY

//ENTER THINGSPEAK SERVER DETAILS BELOW:
unsigned long ch_id = ;
const char * api_write  = "";
const char * api_read = "";
const char * server = "api.thingspeak.com" ;

char blynk_reset = 'k';    // reset r is ok. (No blynk reset required)

WiFiClient client;

int dht_pin = 0; // D3 in NodeMCU which is 0 in GPUI
int relay_pin = 4; // D2 in NodeMCU which is 4 in GPUI

DHT dht(dht_pin, DHT11);

void wifi_connect()
{
  Serial.print("Attempting to connect to SSID : ");

  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, pass);
    Serial.print(".");
    wdt_reset();
    delay(5000);
    wdt_reset();
  }
  Serial.print("\nConnnected\n");
}


BLYNK_WRITE(V1) 
{
  switch (param.asInt())
  {
    case 1: // Item 1
      Serial.println("Soft Reset (Watchdog) is selected");
      blynk_reset = 's';              //reset variable r is set to 's' to denote soft reset.
      break;
    case 2: // Item 2
      Serial.println("Hard Reset (Relay) is selected");
      blynk_reset = 'h';              //reset variable r is set to 'h' to denote hard reset.
      break;
    default:
      Serial.println("Unknown item selected");
  }
}

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(100);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  dht.begin();
  wdt_enable(WDTO_1S);
  wifi_connect();
  Blynk.begin(auth_key,ssid,pass);
}

void loop()
{
  // put your main code here, to run repeatedly:

  Blynk.run();

  if(blynk_reset=='h')
  {
    Serial.print("Hard reset initiated");
    digitalWrite(relay_pin,HIGH);
    delay(100);
  }
  else if(blynk_reset=='s')
  {
    Serial.print("Soft reset initiated");
    while(1);
  }
  
  wdt_reset();
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Resetting the nodeMCU in 1 second");          //Reset part
    while(1);
  }
  wdt_reset();

  float hum = dht.readHumidity();       //Humidity in percentage
  float temp = dht.readTemperature();   //Temperature in Celcius

  if (isnan(hum) || isnan(temp))
  {
    Serial.println("Failed to read from dht sensor");
    return;
  }

  Serial.print("\nTemperature: ");
  Serial.print(temp);
  Serial.print("\nHumidity: ");
  Serial.print(hum);

  wdt_reset();

  if (client.connect(server, 80))
  {
    ThingSpeak.setField(1, temp);
    ThingSpeak.setField(2, hum);
    ThingSpeak.writeFields(ch_id, api_write);
    wdt_reset();
    delay(10000);
  }
  wdt_reset();
  float F1 = ThingSpeak.readFloatField(ch_id, 1, api_read);
  float F2 = ThingSpeak.readFloatField(ch_id, 2, api_read);
  
  Serial.print("\nPublished value of temp = ");
  Serial.print(F1);
  Serial.print("\nPublished value of hum = ");
  Serial.print(F2);

  if (F1 != temp || F2 != hum)
  {
    Serial.println("Resetting the nodeMCU in 1 second");          //Reset part
    while (1);
  }
  delay(10000);
  Serial.println();
}
