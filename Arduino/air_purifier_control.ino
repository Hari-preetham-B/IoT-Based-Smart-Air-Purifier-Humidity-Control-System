#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11

#define FAN 8
#define PUMP 9

LiquidCrystal lcd(12, 11, 5, 4, 3, 6);

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  lcd.begin(16,2);
  Serial.begin(9600);

  dht.begin();

  pinMode(FAN, OUTPUT);
  pinMode(PUMP, OUTPUT);

  lcd.setCursor(0,0);
  lcd.print("Smart Air System");
  delay(2000);
  lcd.clear();
}

void loop()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.print(temperature);
  lcd.print("C ");

  lcd.setCursor(0,1);
  lcd.print("Hum:");
  lcd.print(humidity);
  lcd.print("% ");

  Serial.print("Temperature: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  // Fan control based on temperature
  if(temperature > 30)
  {
    digitalWrite(FAN, HIGH);
  }
  else
  {
    digitalWrite(FAN, LOW);
  }

  // Humidifier control based on humidity
  if(humidity < 40)
  {
    digitalWrite(PUMP, HIGH);
  }
  else
  {
    digitalWrite(PUMP, LOW);
  }

  delay(2000);
}
