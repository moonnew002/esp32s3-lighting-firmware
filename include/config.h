#pragma once

// Definición de pines para tiras LED WS2815 del ESP32-S3

//CORREDORES
#define PIN_LEDS_CORREDORES 37

//SOTANO
#define PIN_LEDS_SOTANO     39

// AMENIDADES
//#define PIN_LEDS_AMENIDADES 47
#define PIN_LEDS_AMENIDADES_ALBERCAS 42
#define PIN_LEDS_AMENIDADES_ACCESO   41
#define PIN_LEDS_AMENIDADES_SALON    40

// DEPARTAMENTOS
#define PIN_LEDS_NIVEL_7    0
#define PIN_LEDS_NIVEL_8    1
#define PIN_LEDS_NIVEL_9    2
#define PIN_LEDS_NIVEL_10   3
#define PIN_LEDS_NIVEL_11   15
#define PIN_LEDS_NIVEL_12   18
#define PIN_LEDS_NIVEL_14   16
#define PIN_LEDS_NIVEL_15   17
#define PIN_LEDS_NIVEL_16   21

// HOTELERÍA
#define PIN_HOTEL_NIVEL_1   36
#define PIN_HOTEL_NIVEL_2   35
#define PIN_HOTEL_NIVEL_3   34
#define PIN_HOTEL_NIVEL_4   33
#define PIN_HOTEL_NIVEL_5   43
//#define PIN_HOTEL_NIVEL_6   43

#define PIN_LEDS_NIVEL_6    44

// PLANTA BAJA (Nueva definición)
#define PIN_LEDS_PB         38

// Parámetros globales de la iluminación
#define BRIGHTNESS         180
#define COLOR_ORDER        GRB

// ================= PINES ETHERNET W5500 (Waveshare ESP32-S3) =================
#define ETH_CS_PIN   14
#define ETH_MISO_PIN 12
#define ETH_MOSI_PIN 11
#define ETH_SCK_PIN  13
#define ETH_RST_PIN  9

// ================= CONFIGURACIÓN RED UDP =================
#define UDP_PORT       8888

// ============================================================
// BRILLOS INDEPENDIENTES POR GRUPO (Valores de 0 a 255)
// Ajustar aquí según la preferencia del arquitecto:
// ============================================================
#define BRIGHTNESS_AMENIDADES   204 // %80
#define BRIGHTNESS_PB           153 // %60
#define BRIGHTNESS_SOTANO       153 // %60
#define BRIGHTNESS_HOTEL        128 // %60
#define BRIGHTNESS_PASILLOS     51 // %20
#define BRIGHTNESS_RESIDENCIAS  180  // Departamentos