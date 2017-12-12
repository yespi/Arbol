#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>


// Globales para la tira de LEDS
#define PIN            D3
#define NUMLEDS        360
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

//Globales para el Wifi
const char* ssid     = "XXXXXX";
const char* password = "XXXXXX";

//Globales para el JSON
char servername[] = "navidad.ripolab.org"; // remote server we will connect to
String result;

String color;
int colorR = 0;
int colorG = 0;
int colorB = 0;
int ultimogrupo = 0;
String deseo_id = "";
String deseo_idant = "#"; //Nº de Deseo imposible en la BBDD

int TotGrupos = 62; //Nº de grupos de LEDs a crear (Max. 60)

// Si queremos asignar los Grupos de LEDs a mano, creamos el siguiente Array
// y comentamos en "Crea_agrupos" la linea que modifica el valor
//int aGleds[10]={5,23,56,75,100,130,150,180,220,270};
int aGleds[100];  
int aGcolorR[100];
int aGcolorG[100];
int aGcolorB[100];
int contador_url = 0;
int contador_estrellas = 100;
long contador_cambios=0;
int cpaseante=0;

WiFiClient client;

void setup() {

  Serial.begin(115200);
  delay(2000);
  Serial.print("Conectando a ");

  Serial.println(ssid);
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Conectado");
  Serial.println(WiFi.localIP());

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show(); // Limpia la tira.
  crear_agrupos(); //Definimos unos grupos iniciales al azar
}

void loop() {

  mostrar_agrupos();
  
  //Cogemos el último LED, el contador determina cuanto esperamos para buscar nuevos deseos.
  if(contador_url<5) {
    contador_url++;
  }
  else
  {
    CogeLed(0);
    if (Check_anterior()) {
      asigna_led();
      contador_cambios=0;
    }
    contador_url = 0;
  }
  
  // Si llevan tiempo sin añadir Deseos, los añadimos nosotros
  if (contador_cambios<70) {
    contador_cambios++;
   }
  else
  {  asigna_led();
    contador_cambios=0;
  }
  
  //Hacemos que el último grupo se vea mejor (Parpadea)
  mostrar_ultimogrupo();

  //apagar_grupos_azar();
  if (contador_estrellas < 100)
  {
    contador_estrellas++;
  } else {
    //crear_estrellas(); // Creamos los leds de ambientación de fondo.
    contador_estrellas = 0;
  }
  paseante();
  //delay(500);
}

/////////////  ----------- Funciones

void mostrar_ultimogrupo()
{
    apagar_grupo(aGleds[ultimogrupo]);
    //delay(100);
    encender_grupo(aGleds[ultimogrupo], aGcolorR[ultimogrupo], aGcolorG[ultimogrupo], aGcolorB[ultimogrupo]);
  
}


int Check_anterior()
{ int check_result = 0;
  if (deseo_id != deseo_idant)
  {
    check_result = 1;
    //Serial.print("Nuevo Deseo asignado.");
    deseo_idant = deseo_id;
  }
  return check_result;
}

int asigna_led()
{
  int i = random(TotGrupos);
  int asignado = 0;

  while ((asignado == 0) && (i < TotGrupos))
  {
    //Buscamos un grupo Vacío, si lo encontramos, completamos con valor.
    if (((aGcolorR[i] == 0) && (aGcolorG[i] == 0)) && (aGcolorB[i] == 0))
    {
      aGcolorR[i] = colorR;
      aGcolorG[i] = colorG;
      aGcolorB[i] = colorB;
      ultimogrupo = i;
      asignado = 1;
    }
    i++;
  }

}

void crear_agrupos()
{
  int i;
  // Creamos todos los grupos pero sólo activamos algunos

  for (i = 1; i < TotGrupos+1; i++)
  {
    aGleds[i] = (6 * i);
    if (random(2) == 1) //(1==1)//
    {
      aGcolorR[i] = random(100);
      aGcolorG[i] = random(100);
      aGcolorB[i] = random(100);
    } else
    {
      aGcolorR[i] = 0;
      aGcolorG[i] = 0;
      aGcolorB[i] = 0;
    }
  }
}

void encender_grupo(int led, int colorR, int colorG, int colorB)
{
  int colorR5, colorG5, colorB5;
  int colorR3, colorG3, colorB3;
  if (colorR > 50) {
    colorR5 = colorR - 50;
  } else {
    colorR5 = 0;
  }
  if (colorG > 50) {
    colorG5 = colorG - 50;
  } else {
    colorG5 = 0;
  }
  if (colorB > 50) {
    colorB5 = colorB - 50;
  } else {
    colorB5 = 0;
  }
  if (colorR > 25) {
    colorR3 = colorR - 25;
  } else {
    colorR3 = 0;
  }
  if (colorG > 25) {
    colorG3 = colorG - 25;
  } else {
    colorG3 = 0;
  }
  if (colorB > 25) {
    colorB3 = colorB - 25;
  } else {
    colorB3 = 0;
  }
  pixels.setPixelColor(led, pixels.Color(colorR5, colorG5, colorB5));
  pixels.setPixelColor(led + 1, pixels.Color(colorR3, colorG3, colorB3));
  pixels.setPixelColor(led + 2, pixels.Color(colorR, colorG, colorB));
  pixels.setPixelColor(led + 3, pixels.Color(colorR, colorG, colorB));
  pixels.setPixelColor(led + 4, pixels.Color(colorR3, colorG3, colorB3));
  pixels.setPixelColor(led + 5, pixels.Color(colorR5, colorG5, colorB5));
  pixels.show(); // This sends the updated pixel color to the hardware.

}

void mostrar_agrupos()
{
  int i;
  for (i = 0; i < TotGrupos; i++)
  {
    encender_grupo(aGleds[i], aGcolorR[i], aGcolorG[i], aGcolorB[i]);
    atenuar_agrupo(i);
  }

}

void atenuar_agrupo(int igrp)
{
  int atenuacion = 1;
  if (aGcolorR[igrp] > atenuacion) {
    aGcolorR[igrp] = aGcolorR[igrp] - atenuacion;
  } else {
    aGcolorR[igrp] = 0;
  }
  if (aGcolorG[igrp] > atenuacion) {
    aGcolorG[igrp] = aGcolorG[igrp] - atenuacion;
  } else {
    aGcolorG[igrp] = 0;
  }
  if (aGcolorB[igrp] > atenuacion) {
    aGcolorB[igrp] = aGcolorB[igrp] - atenuacion;
  } else {
    aGcolorB[igrp] = 0;
  }

}

void apagar_grupos_azar()
{
  int i;
  for (i = 0; i < TotGrupos; i++)
  {
    if (random(6) == 1) {
      apagar_grupo(aGleds[i]);
    }
  }

}

void paseante()
{
  if (cpaseante<NUMLEDS)
  {
    pixels.setPixelColor(cpaseante, pixels.Color(80, 80, 230)); // Moderately bright green color.
    cpaseante++;
  }
  else
  {
    cpaseante=0;
  }
  
}

void apagar_grupo(int led)
{
  int i;
  for (i = 0; i < 6; i++) {
    pixels.setPixelColor(led + i, pixels.Color(0, 0, 0)); // Moderately bright green color.
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void crear_estrellas()
{
  int i;
  for (i = 0; i < 100; i++) {
    pixels.setPixelColor(random(299), pixels.Color(random(25), random(25), random(25))); // Moderately bright green color.
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}


///////// ------------------------------------------------

void CogeLed(int numLed) //client function to send/receive GET request data.
{
  String sURL;
  //Serial.println("Cogiendo datos Arbol");
  //http://navidad.ripolab.org/api/deseos/read.php
  //http://navidad.ripolab.org/api/deseos/read_one.php?id=6
  //http://navidad.ripolab.org/api/deseos/read_last.php

  sURL = "GET http://navidad.ripolab.org/api/deseos/read_one.php?id=" + String(numLed);
  if (numLed == 0) { // Cogemos el últimos ID
    sURL = "GET http://navidad.ripolab.org/api/deseos/read_last.php";
  }

  result = "";
  if (client.connect(servername, 80)) {  //starts client connection, checks for connection
    //Serial.println(sURL);
    client.println(sURL);
    //client.println("GET /data/2.5/forecast?id="+CityID+"&units=metric&cnt=1&APPID="+APIKEY);
    //client.println("GET /data/2.5/weather?q=Ripollet,es&units=metric&cnt=1&APPID="+APIKEY);
    client.println("Host: navidad.ripolab.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Conexion: cerrada");
    client.println();
  }
  else {
    Serial.println("conexion a Web fallida"); //error message if no client connect
    //Serial.println();
  }

  while (client.connected() && !client.available()) delay(1); //waits for data
  //Serial.println("Esperando datos...");

  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
    result = result + c;
  }

  client.stop(); //stop client
  result.replace('[', ' '); result.replace(']', ' ');
  //Serial.println(result);

  char jsonArray [result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';

  StaticJsonBuffer<1024> json_buf;
  JsonObject &root = json_buf.parseObject(jsonArray);
  if (!root.success())
  {
    Serial.println("parseObject() fallido");
  }
  String id = root["id"];
  String color = root["color"];
  String ttl = root["ttl"];
  String timeS = root["date_add"];

  deseo_id = id;
  timeS = convertGMTTimeToLocal(timeS);

  int length = color.length();
  //if(length==7)
  //{
  color = color.substring(1, 7);
  colorR = hexToDec(color.substring(0, 2));
  colorG = hexToDec(color.substring(2, 4));
  colorB = hexToDec(color.substring(4, 7));
  //}

  //Serial.print("Led: ");
  //Serial.println(id);
  /*Serial.print("Color: ");
    Serial.print(color); Serial.print(" -> ");
    Serial.print(color.substring(0, 2)); Serial.print(","); Serial.print(colorR); Serial.print(" -- ");
    Serial.print(color.substring(2, 4)); Serial.print(","); Serial.print(colorG); Serial.print(" -- ");
    Serial.print(color.substring(4, 7)); Serial.print(","); Serial.println(colorB);
  */
  //Serial.print("TTL: ");
  //Serial.println(ttl);
  //Serial.print("Fecha: ");
  //Serial.println(timeS);
}

String convertGMTTimeToLocal(String timeS)
{
  int length = timeS.length();
  timeS = timeS.substring(length - 8, length - 6);
  int time = timeS.toInt();
  timeS = String(time) + ":00";
  return timeS;
}

unsigned long hexToDec(String hexString) {
  unsigned long decValue = 0;
  char nextInt;
  for ( long i = 0; i < hexString.length(); i++ ) {
    nextInt = toupper(hexString[i]);
    if ( isxdigit(nextInt) ) {
      if (nextInt >= '0' && nextInt <= '9') nextInt = nextInt - '0';
      if (nextInt >= 'A' && nextInt <= 'F') nextInt = nextInt - 'A' + 10;
      decValue = (decValue << 4) + nextInt;
    }
  }
  return decValue;
}










