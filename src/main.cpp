// Librerías
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

WiFiServer server(80);

const char* ssid = "POCO X3 NFC";  // SSID de la red WIFI
const char* password = "poco13579";    // contraseña de la red WIFI

int cont = 0;
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

//HTML
String Inicio = "<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8' />"
"<META HTTP-EQUIV='Refresh' CONTENT='1'>"
"<title> PROYECTO PROCESADORES DIGITALES </title>"
"</head>"
"<body>"
"<center>"
"<h1> <u> Cracterísticas de la Tarjeta SD </u> </h1>"
"<br>"
"</center>";

String Tipo = 
"<h2> <ul> <li> Tipo de SD: </li> </ul> </h2>"
"<h2><h2>";

String Tamano = 
"<h2> <ul> <li> Tamaño de SD: </li> </ul> </h2>"
"<h2><h2><br>";

String Fin = 
"</body>"
"</html>";

#define SCREEN_WIDTH 128  // ancho de la pantalla OLED  (en pixels)
#define SCREEN_HEIGHT 64  // alto de la pantall OLED    (en pixels)   
#define OLED_RESET -1     // pin de reinicio

// Inicializar objeto display con el ancho y alto definidos anteriormente con el protocolo de comunicación I2C (&Wire)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Pantallas
int displayScreenNum = 0;
int displayScreenNumMax = 1;

unsigned long lastTimer = 0;
unsigned long timerDelay = 10000; 

// Esta función conecta el módulo ES32 con el WIFI 
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando al WIFI ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void displayIndicator(int displayNumber) {
  int xCoordinates[3] = {59, 69};
  for (int i =0; i<2; i++) {
    if (i == displayNumber) {
      display.fillCircle(xCoordinates[i], 60, 2, WHITE);
    }
    else {
      display.drawCircle(xCoordinates[i], 60, 2, WHITE);
    }
  }
}

// Esta función inicializa la tarjeta microSD en los pines SPI predeterminados
void initSDCard(){
  
  // Mostrar por el terminal si hay una tarjeta SD conectada, y en ese caso el tipo de SD y el tamaño de esta

  if(!SD.begin()){
    Serial.println("Falló en el montaje de la tarjeta");
    return;
  }

  uint8_t cardType = SD.cardType();

  // Si no hay ninguna tarjeta SD conectada 
  if(cardType == CARD_NONE){
    Serial.println("No hay ninguna tarjeta SD conectada!!");
    return;
  }

  Serial.print("Tipo de tarjeta SD: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");          // Si hay una tarjeta tipo MMC
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");         // Si hay una tarjeta tipo SDSC
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");         // Si hay una tarjeta tipo SDHC
  } else {
    Serial.println("Desconocida");  // Si hay una tarjeta desconocida
  }

  // Mostrar tamaño de la tarjeta
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Tamaño de la tarjeta SD: %lluMB\n", cardSize);
}

void TipoSD(){
  uint8_t cardType = SD.cardType();

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("Tipo SD:");
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(25, 35);
  if(cardType == CARD_MMC){
    display.println("MMC");         // Si hay una tarjeta tipo MMC
  } else if(cardType == CARD_SD){
    display.println("SDSC");        // Si hay una tarjeta tipo SDSC
  } else if(cardType == CARD_SDHC){
    display.println("SDHC");        // Si hay una tarjeta tipo SDHC
  } else {
    display.println("Desconocida"); // Si hay una tarjeta desconocida
  }
  display.setCursor(0, 34);
  display.setTextSize(1);
  displayIndicator(displayScreenNum);
  display.display();
}

void TamanoSD(){
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);  

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("Tamano SD:");
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(25, 35);
  display.printf("%lluMB\n", cardSize); 
  display.setCursor(0, 34);
  display.setTextSize(1);
  displayIndicator(displayScreenNum);
  display.display();
}

void updateScreen() {
  if (displayScreenNum == 0){
    TipoSD();
  }
  else {
    TamanoSD();
  }
}

void setup() {
  Serial.begin(9600); 

  initWiFi();
  initSDCard();

 //Inicializar Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

}

void loop() {

  if ((millis() - lastTimer) > timerDelay) {
    updateScreen();
    if(displayScreenNum < displayScreenNumMax) {
      displayScreenNum++;
    }
    else {
      displayScreenNum = 0;
    }
    lastTimer = millis();
  }

  //Pagina Web
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // 
    String currentLine = "";                //
    while (client.connected()) {            // loop mientras el cliente est谩 conectado
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        Serial.write(c);                    // 
        header += c;
        if (c == '\n') {                    // si el byte es un caracter de salto de linea
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            float cardSize = SD.cardSize() / (1024 * 1024);
            float cardType = SD.cardType();
            client.println(Inicio + Tipo + String(cardType) + Tamano + String(cardSize) + Fin);
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}





