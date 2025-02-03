// LA VERSION MK.2 TENDRA ESTABLECIDA LA CONEXION ESP NOW PARA EL MPU PROTOTIPO Y OTRO ESP32 QUE SIMULE EL ESTIMULADOR

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <Adafruit_MPU6050.h>
//#include <Adafruit_Sensor.h>
// Definición de la dirección I2C de la pantalla LCD
LiquidCrystal_I2C lcd(0x3F, 16, 2);

//Adafruit_MPU6050 mpu;

//int16_t ax, ay, az;
//float accY;
//String angleString = "";

//const int kneeMin = 0;  // Ángulo mínimo de la rodilla (en grados)
//const int kneeMax = 100;
//float lastAngle = 0;    // Ángulo máximo de la rodilla (en grados)
// Definición de pines para el joystick y los pulsadores

#define PIN_Y  33
#define BUTTON_PIN 14
#define Verde 17
#define ledrojo 12
#define ledblanco 13

// Variables globales

int menuIndex = 0; 
bool inSubMenu = false; // Indica si estamos dentro del submenu
int subMenuIndex = 0; // Índice del submenu
bool buttonPressed = false; // Índice del menú seleccionado
bool botonverde= false;
String lcdBuffer[4];
uint8_t broadcastAddress[] = {0x30, 0xC9, 0x22, 0x32, 0xDB, 0x70}; 
// Protocolo de comunicacion WIFI ESP NOW 

typedef struct struct_message {
  
  
  float hi;
  float mpu; 
  
  
  
} struct_message;

// el nombre del struct es my Data
struct_message myData;

esp_now_peer_info_t peerInfo;

//callback cuando reciba la info 
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(& myData, incomingData, sizeof( myData));
 
 
  //Hi=myData.Hi;
  
  
}


// callback cuando la info se envie
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  
  
}



void setup() {
  Wire.begin();
  lcd.init();
 lcd.backlight();
  lcd.clear();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(Verde,INPUT_PULLUP);
  pinMode(ledrojo,OUTPUT);
  pinMode(ledblanco,OUTPUT);
  Serial.begin(115200);
  // setup para esp now
  WiFi.mode(WIFI_STA);
 if (esp_now_init() != ESP_OK) {
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
  else
  {
    Serial.println("Succes: Added peer");
  } 

   
 
 esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  
  // Lectura del joystick
  int yVal = analogRead(PIN_Y);

  // Ajuste de la señal del joystick a un rango específico
  int mappedValue = mapJoystickValue(yVal);

 if (mappedValue >= 220) {  // Mover hacia arriba
    if (!inSubMenu) {
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2; // Retrocede al último proceso si estamos en el primero
    } else {
      subMenuIndex--;
      if (subMenuIndex < 0) subMenuIndex = 2; // Retrocede al último proceso si estamos en el primero
    }
    delay(250);  // Debounce
  } else if (mappedValue <= 60) {  // Mover hacia abajo
    if (!inSubMenu) {
      menuIndex++;
      if (menuIndex > 2) menuIndex = 0; // Regresa al primer proceso si estamos en el último
    } else {
      subMenuIndex++;
      if (subMenuIndex > 2) subMenuIndex = 0; // Regresa al primer proceso si estamos en el último
    }
    delay(250);  // Debounce
  }

  // Manejo del botón
  if (digitalRead(BUTTON_PIN) == LOW) { // Verifica si el botón ha sido presionado
     // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) { // Verifica nuevamente
      buttonPressed = true;
    }
  } else {
    buttonPressed = false;
  }
 if (digitalRead(Verde)==LOW){
  
  if (digitalRead(Verde)==LOW);{
  botonverde=true;}
 } else {
   botonverde=false;
 }
   
  // Verificar si se debe ingresar al submenú
   if (!inSubMenu && buttonPressed) {
    switch(menuIndex) {
      case 0:
        inSubMenu = true;
        subMenuIndex = 0;
        break;
      case 1:
        inSubMenu = true;
        subMenuIndex = 0;
        break;
      case 2:
        inSubMenu = true;
        subMenuIndex = 0;// Código para proceso 3 (si lo deseas)
        break;
    }
  } else if (inSubMenu && botonverde) {
    inSubMenu = false;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Seleccione:");

  // Mostrar los procesos con la flecha seleccionada
  switch (menuIndex) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(" EMS");
      // Código para proceso 1 y submenú correspondiente
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(" TENS");
      // Código para proceso 2 y submenú correspondiente
    break;
    case 2:
     lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(" MEDIR ANGULO");
      // Código para proceso 3
      break;
      
  }
  if (inSubMenu) {
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.setCursor(0, 3);
    lcd.print(" ");
    switch (menuIndex) {
      case 0:
        switch (subMenuIndex) {
          case 0:
            lcd.setCursor(0, 1);
            lcd.print("> NIVEL 1");
            digitalWrite(ledrojo, LOW);
            digitalWrite(ledblanco, LOW);
            break;
          case 1:
            lcd.setCursor(0, 1);
            lcd.print("> NIVEL 2");
            digitalWrite(ledrojo, HIGH);
            digitalWrite(ledblanco, HIGH);
          break;
          case 2:
            lcd.setCursor(0, 3);
            lcd.print("> NIVEL 3");
            digitalWrite(ledrojo, HIGH);
            digitalWrite(ledblanco, LOW);
            break;
        }
        break;
      case 1:
        switch (subMenuIndex) {
          case 0:
            lcd.setCursor(0, 1);
            lcd.print("> Opciones 1");
            break;
          case 1:
            lcd.setCursor(0, 1);
            lcd.print("> Opciones 2");
            break;
          case 2:
            lcd.setCursor(0, 3);
            lcd.print("> Opciones 3");
            break;
        }
        break;
      case 2:
        switch (subMenuIndex) {
          case 0:
            lcd.setCursor(0, 3);
            lcd.print("El angulo es");
            lcd.setCursor(12,1);
            lcd.print(myData.mpu);
            delay(500); 
           
          break;
         
        }
        break;
    }
  }
 
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
 //int val= 50;
  myData.hi= 100;
  Serial.print(myData.hi);
  Serial.print (myData.mpu);
  // Esperar un breve periodo de tiempo para evitar el parpadeo
 delay(100);

}
 


  // Actualizar la pantalla LCD con el menú
 // lcd.clear();
  //lcd.setCursor(0, 0);
  //lcd.print("Seleccione:");
  //lcd.setCursor(0, 1);
  //lcd.print(menuIndex == 0 ? "> Proceso 1" : "  Proceso 1");
  //lcd.setCursor(0, 2);
  //lcd.print(menuIndex == 1 ? "> Proceso 2" : "  Proceso 2");

  // Esperar un breve periodo de tiempo para evitar el parpadeo


// Función para ajustar la señal del joystick a un rango específico
int mapJoystickValue(int value) {
  if (value >= 2200) {
    value = map(value, 2200, 4095, 127, 254);
  } else if (value <= 1800) {
    value = map(value, 1800, 0, 127, 0);  
  } else {
    value = 127;
  }

  return value;
}


