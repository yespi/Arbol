
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
int TamGrupo = 6; //Nº de Leds que tiene cada grupo
int TotGrupos = 10; // Se calcula con (NUM_LEDS / TamGrupo) //Nº de grupos de LEDs a crear (Max. 60)
int MinGrupos = 5; //Mínimo de grupos que deben quedar siempre ON
int TimeoutGrupo = 1000; //Ciclos de vida de cada DESEO // Def.100
int TiempoCheckDeseo = 50; //Frecuencia de consulta a la web. // Def. 5
int TiempoCheckInactividad = 5000; //Frecuencia de generacion de nuevos eventos // Def.20
int TiempoAtenu = 20; //Frecuencia de atenuación (cada cuantos ciclos atenuamos un grupo)
int TiempoUG = 50; //Frecuencia de atenuación (cada cuantos ciclos atenuamos un grupo)
int atenuacion = 3; // Cantidad de atenuacion del Grupo por Ciclo // Def. 2
int fadeval = 5; // 1=muy lento - 20=muy rapido.
// Si queremos asignar los Grupos de LEDs a mano, creamos el siguiente Array
// y comentamos en "Crea_agrupos" la linea que modifica el valor
//int aGleds[10]={5,23,56,75,100,130,150,180,220,270};
int aGleds[100];
int aGleds_Timeout[100];
int aGcolorR[100];
int aGcolorG[100];
int aGcolorB[100];
unsigned long contador_url = 0;
unsigned long contador_cambios = 0;
unsigned long contador_ciclos = 0;
unsigned long contador_activos = 0;
unsigned long contador_atenu = 0;
unsigned long contador_UG = 0;
String color;
int ultimogrupo = 0;
String deseo_id = "";
String deseo_idant = "#"; //Nº de Deseo imposible en la BBDD
byte colorR;
byte colorG;
byte colorB;

// Configuración de la tira de LEDS y su conexión a placa
#define PIN            D3
#define NUM_LEDS       60
#define COLOR_ORDER    RGB

CRGB leds[NUM_LEDS]; // Array para los Leds
//CRGB leds_copia[NUM_LEDS];
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

//Globales para el Wifi
const char* ssid     = "xxxxxxxxxxxx";
const char* password = "xxxxx";



//Globales para el JSON
char servername[] = "navidad.ripolab.org"; // remote server we will connect to
String result;


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

  //  FastLED.addLeds<WS2811, PIN>(leds, NUM_LEDS);  // Definicion de la tira
  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  crear_agrupos(); //Definimos unos grupos iniciales al azar

  efecto_random(2);// En la primera ejecución, mostramos efectos al azar
  //efecto_todos();
}

void loop() {
  // Comprobamos la web para ver si hay Deseos nuevos
  check_nuevodeseo();
  // Vamos reduciendo la intensidad de los grupos para que vayan desapareciendo.
  atenuar_agrupos();

  // Si llevan tiempo sin añadir Deseos, los añadimos nosotros
  check_inactividad();
  // Comprobamos si quedan pocos grupos, si es así añadimos uno.
  check_mingrupos();
  // Mostramos el efecto del último deseo creado.
  contador_ciclos++;
  mostrar_ultimogrupo();
  delay(30);
}

/////////////  ----------- Funciones  ------------------------------ ////////////////////////////////////////////////////////////

int check_mingrupos()
{
  contador_activos = checkGruposActivos();
  if (contador_activos < MinGrupos) {
    asigna_led(0);
  }

}
int check_inactividad()
{
  //Serial.println("Check_inactividad");
  if (contador_cambios < TiempoCheckInactividad) {
    contador_cambios++;
  }
  else
  { efecto_random(2);
    contador_cambios = 0;
    asigna_led(0);
  }


}
int check_nuevodeseo()
{
  if (contador_url < TiempoCheckDeseo) {
    contador_url++;
  }
  else
  { // Si hay nuevo DESEO, lo mostramos
    //Serial.print("contador_url:"); Serial.print(contador_url); Serial.print(" - TiempoCheckDeseo: "); Serial.println(TiempoCheckDeseo);
    contador_url = 0;
    contador_cambios = 0;
    if (Nuevo_Led() == 1) {
      //Serial.print("ultimogrupo antes:");Serial.println(ultimogrupo);
      //mostrar_ultimogrupo();
      Strobe(aGcolorR[ultimogrupo],aGcolorG[ultimogrupo],aGcolorB[ultimogrupo],3,300,10);
       //showCounters(1);
      //Serial.println("Nuevo Led asignado");
    }

  }
}

void showCounters(int val)
{
  switch (val) {
    case 1: Serial.println("Nuevo DESEO normal (TiempoCheckDeseo)"); Serial.print("Contador URL:");
      Serial.println(contador_url);

      break;
    case 2: Serial.println("Nuevo DESEO inac (TiempoCheckInactividad)");  Serial.print("Contador cambios:");
      Serial.println(contador_cambios);
      break;
    case 3: Serial.println("Nuevo DESEO-AZAR tras inactividad");  Serial.print("Contador cambios:");
      Serial.println(contador_cambios);
      break;
    case 4: Serial.println("Nuevo DESEO-MINGRUPOS "); Serial.print("Contador activos:");
      Serial.println(contador_activos); Serial.print("Mingrupos:");
      Serial.println(MinGrupos);
      break;
  }
  Serial.print("Contador ciclos:");
  Serial.println(contador_ciclos);

  Serial.print("TimeoutGrupo:"); Serial.print(TimeoutGrupo); Serial.print(",CheckDes:"); Serial.print(TiempoCheckDeseo); Serial.print(",Inac:"); Serial.println(TiempoCheckInactividad);
  Serial.println("----------------");
  Serial.println("Fin_ShowCounters");
}


int Nuevo_Led()
{
  Serial.println("CHECK_Nuevo_led");
  int val = 0;
  CogeLed(0);  // El valor 0 indica coger el último LED
  //Serial.println("check_anterior");
  if (Check_anterior()) {
    asigna_led(1);  // El valor 1 indica DESEO, el valor 0 indica al azar
    val = 1;
  }
  return val;
}

void crear_agrupos()
{
  //Serial.println("crear_agrupos");
  int i ;
  int x;
  int z = 0;
  int contCreados;
  // Creamos todos los grupos pero sólo activamos algunos

  i = 0;
  // Creamos todos los grupos vacios con un Timeout aleatorio
  // Primero borramos los Leds de toda la tira
  while (i < TotGrupos) {
    aGleds[i] = (TamGrupo * i);
    //aGleds_Timeout[i] = random8();
    aGcolorR[i] = 0; aGcolorG[i] = 0; aGcolorB[i] = 0;
    i++;
  }
  setAll(0, 0, 0);

  //Vamos a crear como mínimo "MinGrupos" y como máximo TotGrupos-2
  int numGrupos = MinGrupos; // + random(TotGrupos - MinGrupos) ;

  i = 0; //randomSeed(TotGrupos - 1);

  while (i < numGrupos)
  {
    x = random(TotGrupos); //Generamos un grupo al azar
    //x = x * TamGrupo; // Calculamos la posición Led donde va el grupo
    if (aGcolorR[x] == 0) // Comprobamos si el led está apagado. Si lo está, buscamos otro
    {
      //aGleds[i] = x;
      // Elegimos el color
      aGcolorR[x] = random8(100);
      aGcolorG[x] = random8(100);
      aGcolorB[x] = random8(100);
      setGrupo(x, aGcolorR[x], aGcolorG[x], aGcolorB[x]);

      i++;
    }
  }
  showStrip();
}

int checkGruposActivos() // Nº de Grupos de Leds que tienen color
{
  int numActivos = 0;
  int res = 0;
  for (int i = 0; i < TotGrupos; i++)
  {
    res = check_negro(aGcolorR[i], aGcolorG[i], aGcolorB[i]);
    if (res == 0) {
      numActivos++;
    }
  }
  return numActivos;
}

byte check_negro(byte ledr, byte ledg, byte ledb) // Comprobamos si el LEDRGB está apagado.
{ int cn = 0; // False
  if (ledr == 0 && ledg == 0 && ledb == 0)
  {
    cn++;
  }
  return cn;
}


void atenuar_agrupos()
{
  if (contador_atenu < TiempoAtenu) {
    contador_atenu++;
  } else
  { for (int i = 0; i < TotGrupos; i++)
    {
      atenuar_agrupo(i);
  
    }
    showStrip();
    contador_atenu = 0;
  }
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

int asigna_led(int tipoasignacion)
{ int res = 0;
  //randomSeed(1,TotGrupos);
  int i = random(1, TotGrupos);
  int asignado = 0;

  while (i < TotGrupos)
  {
    if (asignado == 0)
    {
      //Buscamos un grupo Vacío, si lo encontramos, completamos con valor.
      res = check_negro(aGcolorR[i], aGcolorG[i], aGcolorB[i]);
      if (res == 1) { // Si tenemos un grupo vacío.
        // Se ha solicitado un nuevo DESEO
        if (tipoasignacion == 1) {

          aGcolorR[i] = colorR;
          aGcolorG[i] = colorG;
          aGcolorB[i] = colorB;
          ultimogrupo = i;
          
        }

        // Nos hemos quedado sin grupos, mostramos uno nuevo
        if (tipoasignacion == 0)
        {
          //randomSeed(256);
          aGcolorR[i] = random(100);
          aGcolorG[i] = random(100);
          aGcolorB[i] = random(100);
        }
        setGrupo(i, aGcolorR[i], aGcolorG[i], aGcolorB[i]);
        showStrip();

        //asignado = 1;
        i = TotGrupos; // Ya podemos salir del While
      }
    }
    i++;
  }
}


void mostrar_ultimogrupo()
{    //for (int x=0;x<2;x++){
  if (contador_UG<TiempoUG){contador_UG++;}
  else {
  if (ultimogrupo!=0){
      setGrupo(ultimogrupo,aGcolorR[ultimogrupo], aGcolorG[ultimogrupo], aGcolorB[ultimogrupo]);
      showStrip();
      delay(100);
      setGrupo(ultimogrupo,0,0,0);
      showStrip();
      delay(30);
     }
     contador_UG=0;
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

  setGrupo(igrp, aGcolorR[igrp], aGcolorG[igrp], aGcolorB[igrp]); // Asignamos a todo el grupo el color atenuado

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

}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  //showStrip();
}

void efecto_random(int repetir)
{
  for (int i = 0; i < repetir; i++) {
    randomSeed(7);
    int var = random(1, 7) + 1;
    randomSeed(255);
    int r = random8();
    int g = random8();
    int b = random8();

    switch (var) {
      case 1: CylonBounce(r, 0, 0, 4, 10, 50);
        break;
      case 2: Sparkle(r, g, b, 0);
        break;
      case 3: Twinkle(0, 0, b, 10, 100, false);
        break;
      case 4: TwinkleRandom(20, 100, false);
        break;
      case 5: RunningLights(r, g, b, 50);
        break;
      case 6: colorWipe(0x00, r, 0x00, 50);
        colorWipe(0x00, 0x00, r, 50);
        break;
      case 7:  SnowSparkle(0x10, 0x10, 0x10, 20, random(100, 1000));
        break;

    }
  }

}

void efecto_todos()
{
  randomSeed(255);
  int r = random8();
  int g = random8();
  int b = random8();

  CylonBounce(r, 0, 0, 4, 10, 50);
  Sparkle(r, g, b, 0);
  Twinkle(0, 0, b, 10, 100, false);
  TwinkleRandom(20, 100, false);
  RunningLights(r, g, b, 50);
  colorWipe(0x00, r, 0x00, 50);
  colorWipe(0x00, 0x00, r, 50);
  SnowSparkle(0x10, 0x10, 0x10, 20, random(50, 300));

}



///////// ------------------------------------------------

void CogeLed(int numLed) //client function to send/receive GET request data.
{ int con_ok = 1;
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
    con_ok = 0;
  }

  //while (client.connected() && !client.available()) delay(1); //waits for data
  //Serial.println("Esperando datos...");

  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
    result = result + c;
  }

  client.stop(); //stop client
  result.replace('[', ' '); result.replace(']', ' ');
  
  char jsonArray [result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';

  StaticJsonBuffer<128> json_buf;
  JsonObject &root = json_buf.parseObject(jsonArray);
  if (!root.success())
  {
    Serial.println("parseObject() fallido - No asignamos color");
    con_ok = 0;
  }
  if (con_ok == 1)
  {
    String id = root["id"];
    String color = root["color"];
    String ttl = root["ttl"];
    String timeS = root["date_add"];

    deseo_id = id;
    //timeS = convertGMTTimeToLocal(timeS);

    int length = color.length();
    //if(length==7)
    //{
    color = color.substring(1, 7);
    colorR = hexToDec(color.substring(0, 2));
    colorG = hexToDec(color.substring(2, 4));
    colorB = hexToDec(color.substring(4, 7));
  }

}


String convertGMTTimeToLocal(String timeS)
{
  int length = timeS.length();
  timeS = timeS.substring(length - 8, length - 6);
  int time = timeS.toInt();
  timeS = String(time) + ":00";
  return timeS;
}

byte hexToDec(String hexString) {
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

/////////////////////////////// EFECTOS ESPECIALES

void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause) {
  for (int j = 0; j < StrobeCount; j++) {
    setAll(red, green, blue);
    showStrip();
    delay(FlashDelay);
    setAll(0, 0, 0);
    showStrip();
    delay(FlashDelay);
  }
  setAll(red, green, blue);
  delay(EndPause);
}

void BouncingBalls(byte red, byte green, byte blue, int BallCount) {
  float Gravity = -9.81;
  int StartHeight = 1;

  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];

  for (int i = 0 ; i < BallCount ; i++) {
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0;
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i) / pow(BallCount, 2);
  }

  while (true) {
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i] / 1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

      if ( Height[i] < 0 ) {
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();

        if ( ImpactVelocity[i] < 0.01 ) {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round( Height[i] * (NUM_LEDS - 1) / StartHeight);
    }

    for (int i = 0 ; i < BallCount ; i++) {
      setPixel(Position[i], red, green, blue);
    }

    showStrip();
    setAll(0, 0, 0);
  }
}

void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {

  for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++) {
    setAll(0, 0, 0);
    setPixel(i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, red, green, blue);
    }
    setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--) {
    setAll(0, 0, 0);
    setPixel(i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, red, green, blue);
    }
    setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);
}

void Twinkle(byte red, byte green, byte blue, int Count, int SpeedDelay, boolean OnlyOne) {
  setAll(0, 0, 0);

  for (int i = 0; i < Count; i++) {
    setPixel(random(NUM_LEDS), red, green, blue);
    showStrip();
    delay(SpeedDelay);
    if (OnlyOne) {
      setAll(0, 0, 0);
    }
  }

  delay(SpeedDelay);
}

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne) {
  setAll(0, 0, 0);

  for (int i = 0; i < Count; i++) {
    setPixel(random(NUM_LEDS), random(0, 255), random(0, 255), random(0, 255));
    showStrip();
    delay(SpeedDelay);
    if (OnlyOne) {
      setAll(0, 0, 0);
    }
  }

  delay(SpeedDelay);
}



void Sparkle(byte red, byte green, byte blue, int SpeedDelay) {
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel, red, green, blue);
  showStrip();
  delay(SpeedDelay);
  setPixel(Pixel, 0, 0, 0);
}



void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay) {
  setAll(red, green, blue);

  int Pixel = random(NUM_LEDS);
  setPixel(Pixel, 0xff, 0xff, 0xff);
  showStrip();
  delay(SparkleDelay);
  setPixel(Pixel, red, green, blue);
  showStrip();
  delay(SpeedDelay);
}



void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position = 0;

  for (int i = 0; i < NUM_LEDS * 2; i++)
  {
    Position++; // = 0; //Position + Rate;
    for (int i = 0; i < NUM_LEDS; i++) {
      // sine wave, 3 offset waves make a rainbow!
      //float level = sin(i+Position) * 127 + 128;
      //setPixel(i,level,0,0);
      //float level = sin(i+Position) * 127 + 128;
      setPixel(i, ((sin(i + Position) * 127 + 128) / 255)*red,
               ((sin(i + Position) * 127 + 128) / 255)*green,
               ((sin(i + Position) * 127 + 128) / 255)*blue);
    }

    showStrip();
    delay(WaveDelay);
  }
}

void RunningLights_Deseo(int igrp, byte red, byte green, byte blue, int WaveDelay) {
  int Position = igrp+(TamGrupo/2);

  for (int i = igrp; i < (igrp + TamGrupo)*2; i++)
  {
    Position++; // = 0; //Position + Rate;
    for (int i = igrp; i < (igrp + TamGrupo); i++) {
      // sine wave, 3 offset waves make a rainbow!
      //float level = sin(i+Position) * 127 + 128;
      //setPixel(i,level,0,0);
      //float level = sin(i+Position) * 127 + 128;
      setPixel(i, ((sin(i + Position) * 127 + 128) / 255)*red,
               ((sin(i + Position) * 127 + 128) / 255)*green,
               ((sin(i + Position) * 127 + 128) / 255)*blue);
    }

    showStrip();
    delay(WaveDelay);
  }
}

void colorWipe(byte red, byte green, byte blue, int SpeedDelay) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    setPixel(i, red, green, blue);
    showStrip();
    delay(SpeedDelay);
  }
}


