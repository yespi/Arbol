
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
int TotGrupos = 10; // Se calcula con (NUM_LEDS / TamGrupo) //Nº de grupos de LEDs a crear (Max. 60)
int MinGrupos = 2; //Mínimo de grupos que deben quedar siempre ON
int TimeoutGrupo = 100; //Ciclos de vida de cada DESEO // Def.100
int TiempoCheckDeseo = 5; //Frecuencia de consulta a la web. // Def. 5
int TiempoCheckInactividad = 20; //Frecuencia de generacion de nuevos eventos // Def.20
int atenuacion = 2; // Cantidad de atenuacion del Grupo por Ciclo // Def. 2
int ContEstrellas = 100; // Nº de estrellas a generar


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
int cpaseante = 0;
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

  mostrar_agrupos();
  //Cogemos el último LED, el contador determina cuanto esperamos para buscar nuevos deseos.
  if (contador_url < TiempoCheckDeseo) {
    contador_url++;
  }
  else
  {
    CogeLed(0);
    if (Check_anterior()) {
      asigna_led(1);
      contador_cambios = 0;
    }
    contador_url = 0;
  }

  // Si llevan tiempo sin añadir Deseos, los añadimos nosotros
  if (contador_cambios < TiempoCheckInactividad) {
    contador_cambios++;
  }
  else
  { asigna_led(0);
    contador_cambios = 0;
  }

  //Hacemos que el último grupo se vea mejor (Parpadea)
  mostrar_ultimogrupo();

  //desactivar_grupos_azar();
  /*    if (contador_estrellas < 100)
      {
       contador_estrellas++;
      } else {
       //crear_estrellas(); // Creamos los leds de ambientación de fondo.
       contador_estrellas = 0;
      }
  */
  //paseante();
  //delay(500);
}

/////////////  ----------- Funciones

void mostrar_ultimogrupo()
{
  for (int i = 0; i < 5; i++)
  {
    desactivar_grupo(ultimogrupo);
    delay(50);
    activar_grupo(ultimogrupo);
  }
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

int asigna_led(int tipoasignacion)
{
  // Tipoasignacion=0 <- Generar al azar
  // Tipoasignacion=1 <- Generar con el color leído de la web

  int i = random(TotGrupos);
  int asignado = 0;


  while ((asignado == 0) && (i < TotGrupos))
  {
    if (((aGcolorR[i] == 0) && (aGcolorG[i] == 0)) && (aGcolorB[i] == 0)) // Comprobamos si el LED está apagado.
    {
      if (tipoasignacion == 0) {
        aGcolorR[i] = random8(255);
        aGcolorG[i] = random8(255);
        aGcolorB[i] = random8(255);
      }
      else
      { aGcolorR[i] = colorR;
        aGcolorG[i] = colorG;
        aGcolorB[i] = colorB;
        ultimogrupo = i;
        efecto_nuevodeseo();
      }
      aGleds_Timeout[i] = TimeoutGrupo;
      asignado = 1;
    }
    i++;
  }

}

void efecto_nuevodeseo()
{
    // draw a generic, no-name rainbow
  static uint8_t starthue = 0;
  //fill_rainbow( leds , NUM_LEDS, --starthue, 20);
  //FastLED.show();
  //fill_rainbow( leds, NUM_LEDS, 0, 0);
  
}
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
    leds[i].r = 0; leds[i].g = 0; leds[i].b = 0;
    i++;
  }

  //Vamos a crear como mínimo "MinGrupos" y como máximo TotGrupos-2
  int numGrupos = random(TotGrupos - MinGrupos - 2) + MinGrupos;

  for (i = 0; i < numGrupos; i++)
  {
    x = random8(TotGrupos - 1); //Generamos un grupo al azar
    x = x * TamGrupo; // Calculamos la posición Led donde va el grupo
    aGleds[x] = x;
    // Elegimos el color
    aGcolorR[x] = random8();
    aGcolorG[x] = random8();
    aGcolorB[x] = random8();
    for (int j = 0; j < TamGrupo; j++) // Rellenamos el color del grupo
    {
      leds[x + j].r = aGcolorR[x];
      leds[x + j].g = aGcolorG[x];
      leds[x + j].b = aGcolorB[x];
    }

  }

}

void activar_grupo(int grupoled)
{
  int pos = grupoled * TamGrupo;
  // Vamos a iluminar el grupogrupoled y guardamos los colores en el Array por si luego
  // queremos recuperarlos.
  for (int i = 0; i < TamGrupo; i++) {
    leds[pos + i].r = aGcolorR[grupoled];
    leds[pos + i].g = aGcolorG[grupoled];
    leds[pos + i].b = aGcolorB[grupoled];
  }
}

void mostrar_agrupos()
{
  int i;
  for (i = 0; i < TotGrupos; i++)
  {
    if (((aGcolorR[i] == 0) && (aGcolorG[i] == 0)) && (aGcolorB[i] == 0)) // Comprobamos si el LED está apagado.
    {
    } else
    {
      activar_grupo(i);
      atenuar_agrupo(i);
    }

  }
  FastLED.show();


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

  int pos = igrp * TamGrupo;
  for (int i = 0; i < TamGrupo; i++) { // Asignamos a todo el grupo el color atenuado
    leds[pos + i].r = aGcolorR[igrp];
    leds[pos + i].g = aGcolorG[igrp];
    leds[pos + i].b = aGcolorB[igrp];
  }

}

void desactivar_grupos_azar()
{
  int igrp;
  for (igrp = 0; igrp < TotGrupos; igrp++)
  {
    if (random(6) == 1) {
      desactivar_grupo(igrp);
    }
  }

}

void paseante()
{
  if (cpaseante < NUM_LEDS)
  {
    leds[cpaseante].r = 80;
    leds[cpaseante].g = 80;
    leds[cpaseante].b = 80;
    cpaseante++;
  }
  else
  {
    cpaseante = 0;
  }

}

void desactivar_grupo(int grupoled)
{
  int i;
  int pos = grupoled * TamGrupo;
  aGcolorR[grupoled] = 0;
  aGcolorG[grupoled] = 0;
  aGcolorB[grupoled] = 0;
  for (i = 0; i < TamGrupo; i++) {
    leds[pos + i].r = aGcolorR[grupoled]; //Apagamos el LED
    leds[pos + i].g = aGcolorG[grupoled]; //Apagamos el LED
    leds[pos + i].b = aGcolorB[grupoled]; //Apagamos el LED
  }
}

void crear_estrellas()
{
  int i;
  for (i = 0; i < 100; i++) {
    leds[i] = random8();
  }
  //pixels.show(); // This sends the updated pixel color to the hardware.
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










