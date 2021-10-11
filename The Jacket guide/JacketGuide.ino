#include <Servo.h>
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT22
const int tempSensorPin = A0;
const int windSensorPin = A3;
DHT dht(DHTPIN, DHTTYPE);
Servo myservo;
int angle =0;
int index =0;

void setup() {
 // put your setup code here, to run once:
 myservo.attach(9);
 Serial.begin(9600);
 dht.begin();
 pinMode(13, OUTPUT);
 pinMode(12, OUTPUT);
 pinMode(11, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
  float h = dht.readHumidity();
  int motorVal = analogRead(windSensorPin);
  int sensorVal = analogRead(tempSensorPin);
  float degrees = 0;  
  digitalWrite(13, LOW);
  digitalWrite(12, LOW);
  if(!isnan(motorVal)){
    Serial.print("\n Wind speed");
    Serial.print(motorVal);
  }
  else
  {
    Serial.print("\n Failed to get wind reading");
  }
    

  if(!isnan(sensorVal)){
    float voltage = (sensorVal/1024.0) *5.0;
    degrees = (voltage - 0.5) * 100;
    Serial.print("\n Degrees");
    Serial.print(degrees);
  }
  else
  {
    Serial.print("\n Failed to get temp reading");
  }

  if(!isnan(h))
  {
    Serial.print("Humidity is");
    Serial.print(h);
    
  }
  else
  {
    Serial.print("\n Failed to get humidity reading");
  }
  boolean cold = false;
  boolean rain =false;
  boolean wind =false;
  
  if(degrees <20)
    cold = true;
  if(h>=50)
    rain =true;
  if(motorVal)
    wind =true;

  int angle_map[2][2][2]={{45, 70, 90, 70}, {120, 120, 140, 140}};
  
  /*   
   *    45= No jackets necessary
   *    70= Wind cheater
   *    90= Rain-poncho
   *    120= Warm jacket
   *    140= Waterproof jacket
   */

   
  angle = angle_map[cold][rain][wind];
  if(angle> 90)
  { 
      digitalWrite(13, HIGH);
  }
  else
  {
      digitalWrite(12, HIGH);
  }
  myservo.write(angle);
  Serial.print("\n Angle ");
  Serial.print(angle);    

}
