// Esta version MK.4.0   incluye el sensor de HUMO, GAS comunicandose con el esp32 del control para desplegar los valores ppm
#include <esp_now.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

int counter = 0;
//Right motor
int enableRightMotor=22; 
int rightMotorPin1=16;
int rightMotorPin2=17;
//Left motor
int enableLeftMotor=23;
int leftMotorPin1=18;
int leftMotorPin2=19;
//sensor gas

#define Buzzer 4

uint8_t broadcastAddress[] = {0x30, 0xC9, 0x22, 0x32, 0xDB, 0x70}; //C0:49:EF:6C:07:24 30:C9:22:32:DB:70 30:C9:22:32:DB:70



//motor og 200
#define MAX_MOTOR_SPEED 250

const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int rightMotorPWMSpeedChannel = 4;
const int leftMotorPWMSpeedChannel = 5;

#define SIGNAL_TIMEOUT 1000  // This is signal timeout in milli seconds. We will reset the data if no signal
unsigned long lastRecvTime = 0;

struct PacketData
{
  byte xAxisValue;
  byte yAxisValue;
  byte switchPressed;
  bool buttonPressed;
   
};
PacketData myData;




esp_now_peer_info_t peerInfo;


bool throttleAndSteeringMode = false;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  
  
  
}


// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{

  if (len == 0)
  {
    return;
  }
  memcpy(&myData, incomingData, sizeof(myData));
  String inputData ;
  inputData = inputData + "values " + myData.xAxisValue + "  " + myData.yAxisValue + "  " + myData.switchPressed;
  Serial.println(inputData);
  if (myData.switchPressed == true)
  {
    if (throttleAndSteeringMode == false)
    {
      throttleAndSteeringMode = true;
    }
    else
    {
      throttleAndSteeringMode = false;
    }
  }

  if (throttleAndSteeringMode)
  {
    throttleAndSteeringMovements();
  }
  else
  {
    simpleMovements();
  }
  
 lastRecvTime = millis(); 
 if (myData.buttonPressed== true){
  digitalWrite(Buzzer, HIGH);
  Serial.println("SI");} 
  else {
  digitalWrite(Buzzer, LOW);
  Serial.println("NO");}




  

}

void simpleMovements()
{
  if (myData.yAxisValue <= 75)       //Move car Forward
  {
    rotateMotor(MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
  }
  else if (myData.yAxisValue >= 175)   //Move car Backward
  {
    rotateMotor(-MAX_MOTOR_SPEED, -MAX_MOTOR_SPEED);
  }
  else if (myData.xAxisValue >= 175)  //Move car Right
  {
    rotateMotor(-MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
  }
  else if (myData.xAxisValue <= 75)   //Move car Left
  {
    rotateMotor(MAX_MOTOR_SPEED, -MAX_MOTOR_SPEED);
  }
  else                                      //Stop the car
  {
    rotateMotor(0, 0);
  }   
}

void throttleAndSteeringMovements()
{
  int throttle = map( myData.yAxisValue, 254, 0, -255, 255);
  int steering = map( myData.xAxisValue, 0, 254, -255, 255);  
  int motorDirection = 1;
  
  if (throttle < 0)       //Move car backward
  {
    motorDirection = -1;    
  }

  int rightMotorSpeed, leftMotorSpeed;
  rightMotorSpeed =  abs(throttle) - steering;
  leftMotorSpeed =  abs(throttle) + steering;
  rightMotorSpeed = constrain(rightMotorSpeed, 0, 255);
  leftMotorSpeed = constrain(leftMotorSpeed, 0, 255);

  rotateMotor(rightMotorSpeed * motorDirection, leftMotorSpeed * motorDirection);
}

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed)
{
  if (rightMotorSpeed < 0)
  {
    digitalWrite(rightMotorPin1,LOW);
    digitalWrite(rightMotorPin2,HIGH);    
  }
  else if (rightMotorSpeed > 0)
  {
    digitalWrite(rightMotorPin1,HIGH);
    digitalWrite(rightMotorPin2,LOW);      
  }
  else
  {
    digitalWrite(rightMotorPin1,LOW);
    digitalWrite(rightMotorPin2,LOW);      
  }
  
  if (leftMotorSpeed < 0)
  {
    digitalWrite(leftMotorPin1,LOW);
    digitalWrite(leftMotorPin2,HIGH);    
  }
  else if (leftMotorSpeed > 0)
  {
    digitalWrite(leftMotorPin1,HIGH);
    digitalWrite(leftMotorPin2,LOW);      
  }
  else
  {
    digitalWrite(leftMotorPin1,LOW);
    digitalWrite(leftMotorPin2,LOW);      
  } 

  ledcWrite(rightMotorPWMSpeedChannel, abs(rightMotorSpeed));
  ledcWrite(leftMotorPWMSpeedChannel, abs(leftMotorSpeed));    
}

void setUpPinModes()
{
  pinMode(enableRightMotor,OUTPUT);
  pinMode(rightMotorPin1,OUTPUT);
  pinMode(rightMotorPin2,OUTPUT);
  
  pinMode(enableLeftMotor,OUTPUT);
  pinMode(leftMotorPin1,OUTPUT);
  pinMode(leftMotorPin2,OUTPUT);

  //Set up PWM for motor speed
  ledcSetup(rightMotorPWMSpeedChannel, PWMFreq, PWMResolution);
  ledcSetup(leftMotorPWMSpeedChannel, PWMFreq, PWMResolution);  
  ledcAttachPin(enableRightMotor, rightMotorPWMSpeedChannel);
  ledcAttachPin(enableLeftMotor, leftMotorPWMSpeedChannel); 
  
  rotateMotor(0, 0);
}


void setup() 
{
  setUpPinModes();
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(Buzzer, OUTPUT);

 

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

 // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  esp_now_register_recv_cb(OnDataRecv);
}
 
void display(){
 lcd.setCursor(0, 0);               // Set the cursor to the first column and first row
  lcd.print("  Hello (Again)   ");     // Print some text
  lcd.setCursor(0,1);
  lcd.print(counter);
  counter = counter + 1;
          

}





void loop() 
{
  //Check Signal lost.
  unsigned long now = millis();
  if ( now - lastRecvTime > SIGNAL_TIMEOUT ) 
  {
    rotateMotor(0, 0);
  }
 

 //myData.Hi= (ppm);
 esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
 

 
  //display();
 // Serial.print("Concentración de CO2 (ppm): ");
 // Serial.println(ppm);
 //SensorRuido();
 //sensorgas();
 //Serial.println(rzero);
}






