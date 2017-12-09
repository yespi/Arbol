#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>


// Globales para la tira de LEDS
#define PIN            D3
#define NUMLEDS        299
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

//Globales para el Wifi
const char* ssid     = "Yespi_Wireless";
const char* password = "Yamt1965Ssfa@1017";

//Globales para el JSON
char servername[] = "navidad.ripolab.org"; // remote server we will connect to
String result;

String color;
int deseo_id = 1;
int deseo_idant = 0; //Nº de Deseo imposible en la BBDD
int colorR = 0;
int colorG = 0;
int colorB = 0;
int ultimogrupo = 0;
int TotGrupos = 5; //Nº de grupos de LEDs a crear
int aGleds[30];
int aGcolorR[30];
int aGcolorG[30];
int aGcolorB[30];


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
  int contador_url = 0;
  int contador_estrellas = 101;

  if (contador_estrellas > 100)
  {// crear_estrellas(); // Creamos los leds de ambientación de fondo.
  } else {
    contador_estrellas++;
  }

  mostrar_agrupos();
  //Cogemos el último LED, el contador determina cuanto esperamos para buscar nuevos deseos.

  if (contador_url > 1) {
    CogeLed(0);
    contador_url = 0;
    if (Check_anterior()) {
      asigna_led();
    }
  }
  else
  { contador_url++;
  }

  mostrar_ultimogrupo();
  apagar_grupos_azar();
  delay(1000);
}

/////////////  ----------- Funciones

void mostrar_ultimogrupo()
{
  int i;
  for (i = 0; i < 10; i++)
  {
    apagar_grupo(ultimogrupo);
    encender_grupo(ultimogrupo, aGcolorR[ultimogrupo], aGcolorG[ultimogrupo], aGcolorB[ultimogrupo]);
  }
}


int Check_anterior()
{ int check_result = 0;
  if (deseo_id != deseo_idant)
  {
    check_result = 1;
    Serial.print("Nuevo Deseo asignado. Color: ");
    Serial.print(colorR);
    Serial.print(",");
    Serial.print(colorG);
    Serial.print(",");
    Serial.println(colorB);
  }
  return check_result;
}

void asigna_led()
{
  int i = 0;
  int asignado = 0;
  while ((!asignado) && (i < TotGrupos))
  {
    if ((aGcolorR[i] == 0) && (aGcolorG[i] = 0) && (aGcolorB[i] = 0))
    {
      aGcolorR[i] = colorR;
      aGcolorG[i] = colorG;
      aGcolorB[i] = colorB;
      ultimogrupo = aGleds[i];
      Serial.print("Ultimo grupo: ");
      Serial.println(ultimogrupo);
      asignado = 1;
    }
    else {
      Serial.print(".");
    }
    i++;
  }
  Serial.println(i);
  Serial.print("Ultimo Grupo: ");
  Serial.println(ultimogrupo);
}

void crear_agrupos()
{
  int i;
  for (i = 0; i < TotGrupos; i++)
  {
    aGleds[i] = (10 * i) + 5;
    aGcolorR[i] = random(255);
    aGcolorG[i] = random(255);
    aGcolorB[i] = random(255);
  }
}

void encender_grupo(int led, int colorR, int colorG, int colorB)
{
  int colorR5, colorG5, colorB5;
  int colorR3, colorG3, colorB3;
  if ((colorR - 50) > 0) {
    colorR5 = colorR - 50;
  } else {
    colorR5 = 0;
  }
  if ((colorG - 50) > 0) {
    colorG5 = colorG - 50;
  } else {
    colorG5 = 0;
  }
  if ((colorB - 50) > 0) {
    colorB5 = colorB - 50;
  } else {
    colorB5 = 0;
  }
  if ((colorR - 25) > 0) {
    colorR3 = colorR - 25;
  } else {
    colorR3 = 0;
  }
  if ((colorG - 25) > 0) {
    colorG3 = colorG - 25;
  } else {
    colorG3 = 0;
  }
  if ((colorB - 25) > 0) {
    colorB3 = colorB - 25;
  } else {
    colorB3 = 0;
  }
  pixels.setPixelColor(led, pixels.Color(colorR5, colorG5, colorB5));
  pixels.setPixelColor(led + 1, pixels.Color(colorR3, colorG3, colorB3));
  pixels.setPixelColor(led + 2, pixels.Color(colorR, colorG, colorB));
  pixels.setPixelColor(led + 3, pixels.Color(colorR3, colorG3, colorB3));
  pixels.setPixelColor(led + 4, pixels.Color(colorR5, colorG5, colorB5));
  pixels.show(); // This sends the updated pixel color to the hardware.

}

void mostrar_agrupos()
{
  int i;
  for (i = 0; i < TotGrupos; i++)
  {
    encender_grupo(aGleds[i], aGcolorR[i], aGcolorG[i], aGcolorB[i]);
  }
  atenuar_agrupos();
}

void atenuar_agrupos()
{
  int i;
  int atenuacion = 1;
  for (i = 0; i < TotGrupos; i++)
  {
    if (aGcolorR[i] > atenuacion) {
      aGcolorR[i] = aGcolorR[i] - atenuacion;
    } else {
      aGcolorR[i] = 0;
    }
    if (aGcolorG[i] > atenuacion) {
      aGcolorG[i] = aGcolorR[i] - atenuacion;
    } else {
      aGcolorG[i] = 0;
    }
    if (aGcolorB[i] > atenuacion) {
      aGcolorB[i] = aGcolorB[i] - atenuacion;
    } else {
      aGcolorB[i] = 0;
    }

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

void apagar_grupo(int led)
{
  int i;
  for (i = 0; i < 5; i++) {
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

  String deseo_id = root["id"];
  String color = root["color"];
  String ttl = root["ttl"];
  String timeS = root["date_add"];


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
  //Serial.print("Color: ");
  //Serial.print(color); Serial.print(" -> ");
  /*Serial.print(color.substring(0,2));Serial.print(",");Serial.print(colorR);Serial.print(" -- ");
    Serial.print(color.substring(2,4));Serial.print(",");Serial.print(colorG);Serial.print(" -- ");
    Serial.print(color.substring(4,7));Serial.print(",");Serial.println(colorB);
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










