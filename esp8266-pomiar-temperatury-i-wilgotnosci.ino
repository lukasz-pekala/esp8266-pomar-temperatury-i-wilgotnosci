 #include <DHT.h>

#define DHTPIN 12         // wyjście danych sensora jest dołączone do linii D6 nodemcu

DHT dht;

void setup()
{
  Serial.begin(115200);
  
  dht.setup(DHTPIN);

  Serial.print("Status czujnika: ");
  Serial.println(dht.getStatusString());
  Serial.print("Model czujnika: ");
  Serial.println(dht.getModel());
  Serial.print("minimumSamplingPeriod czujnika: ");
  Serial.println(dht.getMinimumSamplingPeriod());
  delay(2000);
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());
    
  float t = dht.getTemperature();
  float h = dht.getHumidity();

  if (isnan(t) || isnan(h))
  {
    Serial.println("ERROR: Blad odczytu danych z czujnika!");
  }
  else
  {
    Serial.print("Wilgotnosc: ");
    Serial.print(h);
    Serial.print(" % ");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");
  }

  //digitalWrite(ledPin, HIGH);
  //delay(1000);                  
  //digitalWrite(ledPin, LOW);
  //delay(1000);                  
}
