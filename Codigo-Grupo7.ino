//*****************************
//***    Grupo 7    ***
//Integrantes
/*
  Chunga Vargas, Manuel Alberto      19200053
  Cruz Reyes, Cristian Ricardo        19200143
  Gonzales Ruiz, Diego Alberto      19200240
  Jimenez Zavala, Axel Aldahir        19200026
  Rivera Obregon, Manuel Jonathan Omar  19200154
  Rojo Lopez, Marco Antonio         19200042
*/
//*****************************


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
DHTesp dht;
// definiendo los pines del led rgb
int Rojo = 5;
int Verde = 4;
int Azul = 0;
// definiendo pin de la bomba de agua
float bomba = 2;
// definiendo pin del sensor de humedad de suelo
#define SensorPin A0
// variable donde se almacenara el valor de la humedad de suelo
float humedad_suelo = 0;

//-----------------Datos sobre la conexion a internet---------------------
const char* ssid = "MOVISTAR_54A2"; //Nombre del router
const char* password = "4PRpDuXNfnuqrcAywsey"; // clave del router
const char* mqtt_server = "192.168.1.15"; // direccion Ip del servidor mqqt (localhost)
//--------------------------------------------
// Variables para la conexion mqqt
WiFiClient espClient;
PubSubClient client(espClient);

// Variables globales
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
// en estas variables se almacenara los datos de los sensores para enviarselo al node red
char dth22_temp[MSG_BUFFER_SIZE]; // DTH 22 Temperatura
char dth22_hum[MSG_BUFFER_SIZE];// DTH 22 Humedad
char hum[MSG_BUFFER_SIZE]; // Humedad de suelo


void setup() {
  // Inicializando pines
  pinMode(SensorPin, INPUT);
  pinMode(Rojo,OUTPUT);
  pinMode(Verde,OUTPUT);
  pinMode(Azul,OUTPUT);
  pinMode(bomba, OUTPUT);
  dht.setup(16,DHTesp::DHT22); // pin del sensor DHT22
  Serial.begin(115200);
  inicializando_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Comprobando conexion
  if (!client.connected()) {
     reconnect();
  }  
  client.loop();

  // Se envia los datos cada 25 segundos
  long tiempo = millis();
  if(tiempo-lastMsg > 25000)
  {
    lastMsg = tiempo;
  }

  // Lectura de los sensores
  
  TempAndHumidity valor = dht.getTempAndHumidity(); // Obteniendo los valores del sensor DHT22
  humedad_suelo = analogRead(SensorPin);// Obteniendo el valor del sensor humedad de suelo

  ledRGB(valor.temperature); // funcion para manipular el led rgb
  
  //Se imprimen los datos de los sensores en el monitor serial
  Serial.println(" Humedad del ambiente: " + String(valor.humidity, 1)+ "%");
  Serial.println(" Temperatura: " + String(valor.temperature, 2)+ "C°\n");
  Serial.println("Humedad del suelo: " + String(humedad_suelo));
  delay(5000);
  
  // Conversion de datos para publicarlos
  sprintf(dth22_temp, "%3.2f", valor.temperature);
  sprintf(dth22_hum, "%3.2f", valor.humidity);
  sprintf(hum, "%3.2f", humedad_suelo);

  // Publicando datos de los sensores en NODE-RED
  client.publish("esp8266/temperatura", dth22_temp);
  client.publish("esp8266/humedad", dth22_hum);
  client.publish("esp8266/humedadSuelo", hum);
  
  enciendeBomba(humedad_suelo);

  

}

//*****************************
//***    CONEXION A WIFI     ***
//*****************************
void inicializando_wifi() {
  delay(10);
  // Conectándonos a una red WiFi
  Serial.println();
  Serial.print("Conectado a ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//*****************************
//***    CONEXION MQTT      ***
//*****************************

void reconnect() {

  while (!client.connected()) {
    Serial.print("Intentando conexión Mqtt...");
    // Creamos un cliente ID
    String clientId = "ESP8266";
    // Intentamos conectar
      if (client.connect(clientId.c_str() ) ) {      
      Serial.println("Conectado!");
      //client.subscribe("esp8622IOT");
      } 
      else {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");
      delay(5000);
      }
  }
}

// Lectura de datos que llegan despues de suscribir
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje llegado[");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
}

void enciendeBomba(double sensorHumedadSuelo) 
{
  // Enciende la bomba durante 10 segundos para regar la planta si el sensor de humedad de suelo
  // es mayor a una lectura de 700 
  if(sensorHumedadSuelo>700){
    digitalWrite(bomba,LOW);
    delay(15000);
    digitalWrite(bomba,HIGH);
  }
  
}


void ledRGB(double lectura)
{
  //Funcion que cambia el color del led rgb segun la temperatura
  if(lectura < 18){   // Si temperatura menor que 18 grados centigrados activar color azul (indicando frio)
    digitalWrite(Rojo,0);
    digitalWrite(Verde,0);
    digitalWrite(Azul,255);
  }
  if(lectura >=18 || lectura < 25) // Si temperatura se mayor que 18 y menor que 25 grados centigrados activar color verde (indicando clima bueno)
  {
    digitalWrite(Rojo,0);
    digitalWrite(Verde,255);
    digitalWrite(Azul,0);
  }
  if(lectura >= 25) // Si temperatura mayor que 25 grados centigrados activar color rojo (indicando calor)
  {
    digitalWrite(Rojo,255);
    digitalWrite(Verde,0);
    digitalWrite(Azul,0);
  }
}
