#include "Hoteleria.h"

const NivelHoteleria hoteleriaNiveles[] = {
  {1, 30},
  {2, 30},
  {3, 30},
  {4, 30},
  {5, 30}
  //
  //{6, 4} //probablemente ya no se cuente como hotel si no como residencia
  //Nivel 6
  //
  // este grupo está separado de la siguiente manera
  //
};

const int NUM_NIVELES_HOTELERIA = sizeof(hoteleriaNiveles) / sizeof(hoteleriaNiveles[0]);

int obtenerLedsHoteleria(int nivel) {
  for (int i = 0; i < NUM_NIVELES_HOTELERIA; i++) {
    if (hoteleriaNiveles[i].nivel == nivel) {
      return hoteleriaNiveles[i].numLeds;
    }
  }
  return 0;
}