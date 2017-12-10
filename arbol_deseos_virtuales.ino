#include <SPI.h>
#include "LedMatrix.h"


#include <Adafruit_NeoPixel.h>

#define PIN            D3
#define TIEMPO_MAX_DESEO 100
#define NUM_LEDS       60
#define NUM_DESEOS     10
const byte LEDS_SLOTS = NUM_LEDS / NUM_DESEOS;


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// Para el contador de deseos
#define NUMBER_DEVICES 4
#define CS_PIN D8
#define DIN_PIN D7
#define CLK_PIN D5


byte deseos_colores[NUM_DESEOS][3] = {};
byte deseos_tiempo[NUM_DESEOS] = {};


void iniciar_deseos() {
  for (int deseo = 0; deseo < NUM_DESEOS; deseo++) {
    deseos_colores[deseo][0] = 0;    // R
    deseos_colores[deseo][1] = 0;    // G
    deseos_colores[deseo][2] = 0;    // B
    deseos_tiempo[deseo] = 0;        // TIEMPO

  }
  // DEMO ========
  deseos_colores[1][0] = 100; deseos_colores[1][1] = 10; deseos_colores[1][2] = 0;
  deseos_tiempo[1] = 100;
  deseos_colores[4][0] = 25; deseos_colores[4][1] = 0; deseos_colores[4][2] = 100;
  deseos_tiempo[4] = 100;
  deseos_colores[8][0] = 0; deseos_colores[8][1] = 50; deseos_colores[8][2] = 50;
  deseos_tiempo[8] = 100;
  // DEMO =========
}


void conexion_wifi(){
  
}

void setup() {

  conexion_wifi();
  Serial.begin(115000);
  pixels.begin(); // Inicializar NeoPixel
  iniciar_deseos();

}


void colorear_todos(byte r, byte g, byte b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
}


void efecto_nuevo_deseo_todos(byte r, byte g, byte b) {
  for (int i = 0; i < 10; i++) {
    colorear_todos(r, g, b);
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(500);
    colorear_todos(0, 0, 0);
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(500);
  }
}

void colorear_slot_forzado(int deseo, byte r, byte g, byte b) {
  int led_inicial = (LEDS_SLOTS * (deseo - 1)) + 1;
  int led_final = led_inicial + LEDS_SLOTS;
  for (int led = led_inicial; led <= led_final; led++) {
    pixels.setPixelColor(led, pixels.Color(r, g, b));
  }
}


void efecto_nuevo_deseo_slot(int deseo) {
  for (int i = 0; i < 5; i++) {
    colorear_slot_forzado(deseo, 0, 0, 0);
    pixels.show();
    delay(1000);
    colorear_slot_forzado(deseo, deseos_colores[deseo][0], deseos_colores[deseo][1], deseos_colores[deseo][2]);
    pixels.show();
    delay(1000);
  }
}


void add_deseo(int deseo, byte r, byte g, byte b) {
  efecto_nuevo_deseo_todos(r, g, b);
  mostrar_deseos_actuales();

  // añadimos el nuevo deseo
  deseos_colores[deseo][0] = r; deseos_colores[deseo][1] = g; deseos_colores[deseo][2] = b;
  deseos_tiempo[deseo] = TIEMPO_MAX_DESEO;

  // efecto sobre el deseo
  efecto_nuevo_deseo_slot(deseo);
}


void colorear_slot(int deseo) {
  int led_inicial = (LEDS_SLOTS * (deseo - 1)) + 1;
  int led_final = led_inicial + LEDS_SLOTS;
  for (int led = led_inicial; led <= led_final; led++) {
    pixels.setPixelColor(led, pixels.Color(deseos_colores[deseo][0], deseos_colores[deseo][1], deseos_colores[deseo][2]));
  }
}

void leds_off() {
  for (int led = 0; led < NUM_LEDS; led++) {
    pixels.setPixelColor(led, pixels.Color(0, 0, 0));
  }
}


void mostrar_deseos_actuales() {
  leds_off();
  for (int deseo = 1; deseo <= NUM_DESEOS; deseo++) {
    colorear_slot(deseo);
    pixels.show();
    delay(100);
  }
}


boolean verificar_nuevos_deseos(byte *r, byte *g, byte *b) {
  // compara la tabla actual vs tabla en web
  // si hay uno nuevo devuelve el RGB
  return (true); // return(false);
}

int buscar_slot_vacio() {
  // ramdom de slot vacio teniendo en cuenta los ya ocupados
}


void leer_deseos_web() {
  // llamar a ULR para recuperar deseos

  // comparar con tabla de deseos actuales
  byte r, g, b;
  int deseo_slot;
  if (verificar_nuevos_deseos(&r, &g, &b)) {
    deseo_slot = buscar_slot_vacio();
    add_deseo(deseo_slot, r, g, b);
  }
}


void reducir_tiempo_deseos() {
  // reduce el tabla de tiempos de deseos
  // podría reducir la intensidad o hacer efecto de apagado llegado un mínimo
}

void loop() {

  mostrar_deseos_actuales();
  leer_deseos_web();
  reducir_tiempo_deseos();

  // BLOQUE DEMO ================
  delay(5000);
  add_deseo(5, 0, 255, 0);
  delay(5000);
  add_deseo(3, 10, 45, 15);
  delay(5000);
  iniciar_deseos(); // reset demo
  // BLOQUE DEMO ===============

}
