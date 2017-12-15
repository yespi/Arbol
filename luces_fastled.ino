
/***********************************************************************************************
   Proyecto Arbol de los Deseos - Ripolab HackLab

   La idea es adornar un arbol de navidad con una tira de led y configurar una placa con Wifi
   para que vaya generando secuencias animadas de color.
   Cuando un usuario conecte a http:\\navidad.ripolab.org, elegirá un color (Deseo)
   y al enviarlo, la placa lo detectará y lo mostrará en la tira de Leds durante un tiempo determinado.
*/

// Necesitamos este parametro para que funcione la NODEMCU (ESP-12E) (LOLIN) con la libreria FastLed
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include "FastLED.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>


/// Configuración del Arbol
int TamGrupo = 5; //Nº de Leds que tiene cada grupo
int TotGrupos = 12; // Se calcula con (NUM_LEDS / TamGrupo) //Nº de grupos de LEDs a crear (Max. 60)
int MinGrupos = 3; //Mínimo de grupos que deben quedar siempre ON
int TimeoutGrupo = 100; //Ciclos de vida de cada DESEO // Def.100
int TiempoCheckDeseo = 5; //Frecuencia de consulta a la web. // Def. 5
int TiempoCheckInactividad = 20; //Frecuencia de generacion de nuevos eventos // Def.20
int atenuacion = 1; // Cantidad de atenuacion del Grupo por Ciclo // Def. 2
int ContEstrellas = 100; // Nº de estrellas a generar
int fadeval = 5; // 1=muy lento - 20=muy rapido.

// Configuración de la tira de LEDS y su conexión a placa
#define PIN            D3
#define NUM_LEDS       360
#define COLOR_ORDER RGB

CRGB leds[NUM_LEDS]; // Array para los Leds
//CRGB leds_copia[NUM_LEDS];
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

//Globales para el Wifi
const char* ssid     = "Yespi_Wireless";
const char* password = "Yamt1965Ssfa@1017";



//Globales para el JSON
char servername[] = "navidad.ripolab.org"; // remote server we will connect to
String result;

// Si queremos asignar los Grupos de LEDs a mano, creamos el siguiente Array
// y comentamos en "Crea_agrupos" la linea que modifica el valor
//int aGleds[10]={5,23,56,75,100,130,150,180,220,270};
int aGleds[100];
int aGleds_Timeout[100];
int aGcolorR[100];
int aGcolorG[100];
int aGcolorB[100];
int contador_url = 0;
long contador_cambios = 0;
String color;
int ultimogrupo = 0;
String deseo_id = "";
String deseo_idant = "#"; //Nº de Deseo imposible en la BBDD
int colorR;
int colorG;
int colorB;


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
  FastLED.addLeds<WS2811, PIN>(leds, NUM_LEDS);  // Definicion de la tira
  crear_agrupos(); //Definimos unos grupos iniciales al azar
}

void loop() {

  //mostrar_agrupos();
  atenuar_agrupos();
  //Cogemos el último LED, el contador determina cuanto esperamos para buscar nuevos deseos.
  if (contador_url < TiempoCheckDeseo) {
    contador_url++;
  }
  else
  { Serial.println("Leyendo Web");
    CogeLed(0);   // El valor 0 indica coger el último LED
    if (Check_anterior()) {
      Serial.println("Asignando Led nuevo");
      asigna_led(1);  // El valor 1 indica DESEO, el valor 0 indica al azar
      //Hacemos que el último grupo se vea mejor (Parpadea)
      mostrar_ultimogrupo();
      contador_cambios = 0;
    }
    contador_url = 0;
  }

  // Si llevan tiempo sin añadir Deseos, los añadimos nosotros
  if (contador_cambios < TiempoCheckInactividad) {
    contador_cambios++;
  }
  else
  { asigna_led(0); // El valor 1 indica DESEO, el valor 0 indica al azar
    contador_cambios = 0;
  }

  if (checkGruposActivos() < MinGrupos) {
    asigna_led(0);
  }

}

/////////////  ----------- Funciones

void crear_agrupos()
{
  int i ;
  int x;
  int z = 0;
  int contCreados;
  // Creamos todos los grupos pero sólo activamos algunos

  i = 0;
  // Creamos todos los grupos vacios con un Timeout aleatorio
  // Primero borramos los Leds de toda la tira
  while (i < (TotGrupos - 1)) {
    aGleds[i] = (TamGrupo * i);
    aGleds_Timeout[i] = random8();
    aGcolorR[i] = 0; aGcolorG[i] = 0; aGcolorB[i] = 0;
    i++;
  }
  setAll(0, 0, 0);

  //Vamos a crear como mínimo "MinGrupos" y como máximo TotGrupos-2
  int numGrupos = MinGrupos + random(TotGrupos - MinGrupos) ;

  Serial.print("Grupos a Crear: ");
  Serial.println(numGrupos);

  i = 0;
  while (i < numGrupos)
  {
    x = random(TotGrupos); //Generamos un grupo al azar
    x = x * TamGrupo; // Calculamos la posición Led donde va el grupo
    if (aGcolorR[i] == 0) // Comprobamos si el led está apagado. Si lo está, buscamos otro
    {
      aGleds[i] = x;
      // Elegimos el color
      aGcolorR[i] = random8(100) + 1;
      aGcolorG[i] = random8(100) + 1;
      aGcolorB[i] = random8(100) + 1;
      setGrupo(i, aGcolorR[i], aGcolorG[i], aGcolorB[i]);
      showStrip();
      i++;
    }
  }
  Serial.print("Check grupos Activos: ");
  Serial.println(checkGruposActivos());

  delay(5000);
}

void mostrar_agrupos()
{
  int i;
  for (i = 0; i < TotGrupos; i++)
  {
    if (aGcolorR[i] != 0)
    {
      setGrupo(i, aGcolorR[i], aGcolorG[i], aGcolorB[i]);
      atenuar_agrupo(i);

    }

  }
  showStrip();
}

int checkGruposActivos()
{
  int val = 0;
  for (int i = 0; i < TotGrupos; i++)
  { if (aGcolorR[i] != 0) // Comprobamos si el LED está apagado.
    {
      val++;
    }

  }
  return val;
}

void atenuar_agrupos()
{
  for (int i = 0; i < TotGrupos; i++)
  {
    atenuar_agrupo(i);

  }
  showStrip();
}

int Check_anterior()
{ int check_result = 0;
  if (deseo_id != deseo_idant)
  {
    check_result = 1;
    deseo_idant = deseo_id;
  }
  return check_result;
}

void asigna_led(int tipoasignacion)
{
  // Tipoasignacion=0 <- Generar al azar
  // Tipoasignacion=1 <- Generar con el color leído de la web

  int i = random(TotGrupos);
  int asignado = 0;


  while ((asignado == 0) && (i < TotGrupos))
  { if (aGcolorR[i] == 0) // Comprobamos si el LED está apagado.
    {
      if (tipoasignacion == 0) {
        aGcolorR[i] = random8(100);
        aGcolorG[i] = random8(100);
        aGcolorB[i] = random8(100);
      }
      else
      { Serial.println("Creando COLORES deseo");
        aGcolorR[i] = colorR;
        aGcolorG[i] = colorG;
        aGcolorB[i] = colorB;
        Serial.print("Asignado Deseo");
        Serial.println(i);
        Serial.print(colorR);
        Serial.print(",");
        Serial.print(colorG);
        Serial.print(",");
        Serial.println(colorB);

        ultimogrupo = i;
      }
      setGrupo(i, colorR, colorG, colorB); // Encendemos el grupo en la tira de LEDS
      aGleds_Timeout[i] = TimeoutGrupo;
      asignado = 1;
    }
    else {
      i++;
      if ((i >= TotGrupos) && (asignado == 0)) {
        i = 0;
      }

    }
  }
}

void mostrar_ultimogrupo()
{
  for (int cont = 0; cont < 5; cont++) {
    ///FadeInOut_Grupo(ultimogrupo, 210, 0x77, 210);
    setGrupo(ultimogrupo, 0, 0, 0);
    delay(10);
    setGrupo(ultimogrupo, aGcolorR[ultimogrupo], aGcolorG[ultimogrupo], aGcolorB[ultimogrupo]);
  }

}



void atenuar_agrupo(int igrp)
{
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

  setGrupo(aGleds[igrp], aGcolorR[igrp], aGcolorG[igrp], aGcolorB[igrp]); // Asignamos a todo el grupo el color atenuado

}

///// Esta función al ejecutarse 3 o 4 veces produce un Crash, revisar
void FadeInOut_Grupo(int igrp, byte red, byte green, byte blue) {
  float r, g, b;

  Serial.println("Fade");
  for (int k = 0; k < 256; k = k + fadeval) {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setGrupo(aGleds[igrp], r, g, b);
    showStrip();
  }

  for (int k = 256; k >= 0; k = k - fadeval) {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setGrupo(aGleds[igrp], r, g, b);
    showStrip();
  }
}


void showStrip() {
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.show();
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  FastLED.show();
#endif
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.setPixelColor(Pixel, strip.Color(red, green, blue));
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
#endif
}

void setGrupo(int igrp, byte red, byte green, byte blue) {
  for (int i = aGleds[igrp]; i < (aGleds[igrp] + TamGrupo); i++ ) {
    setPixel(i, red, green, blue);
  }
  showStrip();
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  //showStrip();
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
  color = color.substring(1, 7);
  colorR = hexToDec(color.substring(0, 2));
  colorG = hexToDec(color.substring(2, 4));
  colorB = hexToDec(color.substring(4, 7));

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



