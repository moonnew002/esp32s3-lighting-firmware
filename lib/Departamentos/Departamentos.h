#ifndef DEPARTAMENTOS_H
#define DEPARTAMENTOS_H

#include <Arduino.h>

// Estructura de cada departamento
struct Departamento {
  const char* nombre;      // ej: "710", "1214A"
  int nivel;          // Nivel (7, 8, 10, 12, etc.)
  int numLeds;        // Cantidad de LEDs asignados
  int leds[7];        // Índices de los LEDs
};

// 'extern' le dice al compilador que los datos están definidos en el .cpp
extern const Departamento departamentos[];
extern const int NUM_DEPARTAMENTOS;

// Prototipos de funciones útiles
int obtenerIndiceDepartamento(const String& nombreDepto);

#endif