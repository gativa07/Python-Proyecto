

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

IRsend irsend(4);


//ESP8266
const char* ssid = "HOME-0437";
const char* password = "40441500";
const char* mqtt_server = "192.168.1.74";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int a;

//Variables para lectura
int Entrada10 = 0; //D3
int Salida11 = 2; //D4  paso personas
int contadorentrada = 0;
int contadorsalida = 0;
int contadortotal = 0;
int BanderaEntradaSalida = 0;
int outputpin = A0;  //A0 temp
int Ledpin = 4;      //D2 televisor

//
char enviocontadorentrada[100];
char enviocontadorsalida[100];
char enviocontadortotal[100];
char enviotemperatura[100];
//

//Funcion Lectura Pin
void Lectura(){
  
  int EstadoEntrada = digitalRead(Entrada10);
  int EstadoSalida = digitalRead(Salida11);
  Serial.println("Entrada: ");
  Serial.println(EstadoEntrada);
  Serial.println("Salida: ");
  Serial.println(EstadoSalida);
  delay(10);
  if((EstadoEntrada != 1)&&(EstadoSalida != 0)){
      contadorentrada = 1+contadorentrada;
      BanderaEntradaSalida = 1;
      delay(3000);
  }
  if((EstadoEntrada != 0)&&(EstadoSalida != 1)){
      contadorsalida = 1+contadorsalida;
      delay(3000);
  }
  contadortotal = contadorentrada - contadorsalida;
  
  //stringcontadorentrada = String(contadorentrada);
  snprintf (enviocontadorentrada, 100, "Personas que han entrado: ", contadorentrada);
  snprintf (enviocontadorsalida, 100, "Personas que han salido: ", contadorsalida);
  snprintf (enviocontadortotal, 100, "Personas actuales: ", contadortotal);

  client.publish("proyecto/contadorpersonas/entrada", enviocontadorentrada);
  client.publish("proyecto/contadorpersonas/salida", enviocontadorsalida);
  client.publish("proyecto/contadorpersonas/total", enviocontadortotal);
/*  
  Serial.println("Personas Que Han Entrado: ");
  Serial.println(contadorentrada);
  Serial.println("Personas Que Han Salido: ");
  Serial.println(contadorsalida);
  Serial.println("Personas Dentro De La Sala: ");
  Serial.println(enviocontadortotal);
*/
}
//

void temperatura()
{
int analogValue = analogRead(outputpin);
float millivolts = (analogValue/1024.0) * 3000; //3300 is the voltage provided by NodeMCU
float celsius = millivolts/10;
Serial.print("Temperatura: ");
Serial.println(celsius);
delay(1000);
snprintf (enviotemperatura, 100, "Temperatura actual: ", celsius);
client.publish("proyecto/temperatura", enviotemperatura);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  a=int(payload[0]);
  Serial.println(topic);
  Serial.println(a);
  if(a==49){
    delay(1000);
    irsend.sendSAMSUNG(0xE0E040BF, 32);
    Serial.println("Encender/Apagar");
    }
  if(a==50){
    delay(1000);
    irsend.sendSAMSUNG(0xE0E0E01F, 32);
    Serial.println("+");
    }
  if(a==51){
    delay(1000);
    irsend.sendSAMSUNG(0xE0E0D02F, 32);
    Serial.println("-");
  }
  if(a==52){  
    delay(1000);
    irsend.sendSAMSUNG(0xE0E048B7, 32);
    Serial.println("CH+");
  }
  if(a==53){
    delay(1000);
    irsend.sendSAMSUNG(0xE0E008F7, 32);
    Serial.println("CH-");
  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe("Proyecto/tv2");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  irsend.begin();
  Serial.begin(9600);
  pinMode(Entrada10, INPUT);
  pinMode(Salida11, INPUT); 
  pinMode(outputpin, INPUT); 
  pinMode(Ledpin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
}

void loop() {
while(true){
  if (!client.connected()) {
    reconnect();
  }
  Lectura();
  temperatura();
  client.loop();
  }
}
