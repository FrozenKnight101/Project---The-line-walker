#include <DHT.h>

#define DHTPIN 11 // DHT22 PIN
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

// Define variables for DHT22 sensor
float humidity;  //Stores humidity value
float temperature; //Stores temperature value

// Define variables for ultrasonic sensors
const int echoPinFront = 9;
const int trigPinFront = 10;
long duration;
int distance;

// Define variables for IR sensors
const int irPinLeft = 8;
const int irPinRight = 7;

// Define variables for Motors
const int leftMotorF = 3;
const int leftMotorB = 2;
const int rightMotorF = 5;
const int rightMotorB = 4;

// For serial receive
const byte numChars = 6;
char receivedChars[numChars]; // an array to store the received data
String received; // the data as a string
boolean newData = false;

void setup() {

  dht.begin();

  pinMode(trigPinFront, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPinFront, INPUT); // Sets the echoPin as an Input

  // Define IR sensor pins as input
  pinMode(irPinLeft, INPUT);
  pinMode(irPinRight, INPUT);

  // Define motor pins as output
  pinMode(leftMotorF, OUTPUT);
  pinMode(leftMotorB, OUTPUT);
  pinMode(rightMotorF, OUTPUT);
  pinMode(rightMotorB, OUTPUT);

  Serial.begin(9600); 
  
  while(received != "START")
  {
     recvWithEndMarker(); 
  }
}

void loop() { 
  
  int frontDistance = ultrasonic(echoPinFront,trigPinFront);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  String message = "";
  message = message + "[" + humidity + "%," + temperature + "C," + frontDistance + "cm]\n";
  
  Serial.print(message);

  if(frontDistance <= 8)
  {
    Stop();
  }
  else
  {
    if(digitalRead(irPinLeft) == HIGH  &&  digitalRead(irPinRight) == LOW)
    {
      Left();
//      Serial.println("left");
    }
    else if(digitalRead(irPinRight) == HIGH && digitalRead(irPinLeft) == LOW)
    {
      Right();
//      Serial.println("right");
    }
    else if (digitalRead(irPinRight) == HIGH && digitalRead(irPinLeft) == HIGH)
    {
      Forward();
//      Serial.println("forward"); 
    }
    else
    {
      Stop();
//      Serial.println("Stopped"); 
    }
  }
  delay(100);
}

void recvWithEndMarker() 
{
 static byte ndx = 0;
 char endMarker = '\n';
 char rc;
 while (Serial.available() > 0 && newData == false) 
 {
  rc = Serial.read();
  Serial.print(rc);
  if (rc != endMarker) 
  {
    receivedChars[ndx] = rc;
    ndx++;
    if (ndx >= numChars) 
    {
      ndx = numChars - 1;
    }
  }
  else 
  {
    receivedChars[ndx] = '\0'; // terminate the string
    received = String(receivedChars);
    ndx = 0;
    newData = true;
  }
 }
}


void Stop()
{
  // Stop Left Motor
  digitalWrite(leftMotorF, LOW);
  digitalWrite(leftMotorB, LOW);

  // Stop Right Motor
  digitalWrite(rightMotorF, LOW);
  digitalWrite(rightMotorB, LOW);  
}

void Forward(){
  // Run Left Motor In Forward Direction
  digitalWrite(leftMotorF, HIGH);
  digitalWrite(leftMotorB, LOW);

  //Run Right Motor in Forward Direction
  digitalWrite(rightMotorF, HIGH);
  digitalWrite(rightMotorB, LOW);  
}

void Left(){
  // Stop Left Motor
  digitalWrite(rightMotorF, LOW);
  digitalWrite(rightMotorB, LOW);

  // Run Right Motor in Forward Direction
  digitalWrite(leftMotorF, HIGH);
  digitalWrite(leftMotorB, LOW);
}

void Right(){
  //Stop Right Motor
  digitalWrite(leftMotorF, LOW);
  digitalWrite(leftMotorB, LOW);

  // Run Left Motor in Forward Direction
  digitalWrite(rightMotorF, HIGH);
  digitalWrite(rightMotorB, LOW);
}

// Function to get data from ultrasonic sensor
int ultrasonic(int echoPin, int trigPin) {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  return distance;
}
