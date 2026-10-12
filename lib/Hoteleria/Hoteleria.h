#ifndef HOTELERIA_H
#define HOTELERIA_H

#include <Arduino.h>

struct NivelHoteleria {
  int nivel;
  int numLeds;
};

extern const NivelHoteleria hoteleriaNiveles[];
extern const int NUM_NIVELES_HOTELERIA;

// Devuelve la cantidad de LEDs de un nivel de hotelería
int obtenerLedsHoteleria(int nivel);

#endif