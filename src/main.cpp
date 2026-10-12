#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <FastLED.h>

#include "config.h"
#include "Departamentos.h"
#include "Hoteleria.h"
#include "Pasillos.h"

// ============================================================
// CANTIDAD DE LEDS Y CONSTANTES
// ============================================================
#define NUM_LEDS_ALBERCAS   34
#define NUM_LEDS_LOBI       11
#define NUM_LEDS_SALON      5
#define NUM_LEDS_PB         21
#define NUM_LEDS_SOTANO      8

// Ahora son 10 niveles de departamentos (Se agregó Nivel 6 al inicio con 18 leds)
const int NUM_LEDS_DEPTO_NIVEL[10] = {
  18, 36, 36, 36, 36, 37, 39, 40, 39, 39 //TENGO EN 40 el nivel 14 por mal conexión, ya no se
                                         //puede solucionar físicamente debido a que ya está pegada la maqueta
                                         //se queda en 40
};

// Constantes lógicas de pasillos
const int NUM_LEDS_PASILLOS_MAIN = 15; // N1 a N5, las tiras de los pasillos del N1-N5 están unificados en un mismo PIN, por lo cual lógicamente solo es una tira de 13 leds
const int INDICES_PASILLO_N6[] = {6, 7, 8, 9, 10, 11, 12, 14};
const int NUM_LEDS_PASILLO_N6 = 8;

// ============================================================
// ARRAYS DE LEDS
// ============================================================
CRGB tiraAlbercas[NUM_LEDS_ALBERCAS];
CRGB ledsAcceso[NUM_LEDS_LOBI];
CRGB ledsSalon[NUM_LEDS_SALON];
CRGB ledsPB[NUM_LEDS_PB];
CRGB ledsSotano[NUM_LEDS_SOTANO];

CRGB ledsHotel[5][30];           // Niveles 1 al 5
CRGB ledsPasillosUnificados[13]; // 11 por nivel (N1 - N5) + 2 (Residential Pool)
CRGB ledsDeptos[10][40];         // Niveles 6 al 16

// ============================================================
// ETHERNET 
// ============================================================
#define MAX_UDP_BUFFER 512
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 200);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
EthernetUDP Udp;
char packetBuffer[MAX_UDP_BUFFER];

// ============================================================
// FUNCIONES AUXILIARES
// ============================================================
CRGB getColorGrupo(uint8_t brillo, bool encender = true) {
  if (!encender) {
    return CRGB::Black;
  }
  return CRGB(brillo, brillo, brillo);
}

void setColorZona(CRGB* tira, int numLeds, CRGB color) {
  for (int i = 0; i < numLeds; i++) {
    tira[i] = color;
  }
}

int getIndiceDeptoNivel(int nivel) {
  switch (nivel) {
    case 6:  return 0; // Nivel 6 (NUEVO)
    case 7:  return 1;
    case 8:  return 2;
    case 9:  return 3;
    case 10: return 4;
    case 11: return 5;
    case 12: return 6;
    case 14: return 7;
    case 15: return 8;
    case 16: return 9;
    default: return -1;
  }
}

// ============================================================
// CONTROL DE TODA LA MAQUETA
// ============================================================
void encenderTodo() {
  
  // 1. Departamentos (Base) - Enciende todo incluyendo Nivel 6
  CRGB colorRes = getColorGrupo(BRIGHTNESS_RESIDENCIAS, true);
  for (int i = 0; i < 10; i++) {
    setColorZona(ledsDeptos[i], NUM_LEDS_DEPTO_NIVEL[i], colorRes);
  }

  // 2. Amenidades Físicas (Incluye automáticamente los LEDs de los Restaurantes)
  CRGB colorA = getColorGrupo(BRIGHTNESS_AMENIDADES, true);
  setColorZona(tiraAlbercas, NUM_LEDS_ALBERCAS, colorA);
  setColorZona(ledsAcceso, NUM_LEDS_LOBI, colorA);
  setColorZona(ledsSalon, NUM_LEDS_SALON, colorA);

  // 3. Amenidades Lógicas (Sobrescribe Nivel 6 y tira de pasillos)
  ledsPasillosUnificados[11] = colorA; // RP
  ledsPasillosUnificados[12] = colorA; // RP
  ledsPasillosUnificados[13] = colorA; // RP
  ledsPasillosUnificados[14] = colorA; // RP
  ledsDeptos[0][0]  = colorA; // LB1
  ledsDeptos[0][18] = colorA; // LB2
  ledsDeptos[0][15] = colorA; // LB3
  ledsDeptos[0][13] = colorA; // RG
  ledsDeptos[0][16] = colorA; // OL
  ledsDeptos[0][17] = colorA; // OL

  // 4. Planta Baja y Sótano
  setColorZona(ledsPB, NUM_LEDS_PB, getColorGrupo(BRIGHTNESS_PB, true));
  setColorZona(ledsSotano, NUM_LEDS_SOTANO, getColorGrupo(BRIGHTNESS_SOTANO, true));

  // 5. Hotel N1-N5
  for (int nivel = 1; nivel <= 5; nivel++) {
    setColorZona(ledsHotel[nivel - 1], obtenerLedsHoteleria(nivel), getColorGrupo(BRIGHTNESS_HOTEL, true));
  }

  // 6. Pasillos Unificados y Pasillos Lógicos (Sobrescribe Nivel 6)
  CRGB colorP = getColorGrupo(BRIGHTNESS_PASILLOS, true);
  setColorZona(ledsPasillosUnificados, NUM_LEDS_PASILLOS_MAIN, colorP);
  for (int i = 0; i < NUM_LEDS_PASILLO_N6; i++) {
    ledsDeptos[0][INDICES_PASILLO_N6[i]] = colorP;
  }

  FastLED.show();
}

void apagarTodo() {
  FastLED.clear();
  FastLED.show();
}

// ============================================================
// PROCESAR LISTAS
// ============================================================
void aplicarComandoLista(String lista, bool encender, String categoria) {
  int inicio = 0;

  while (inicio < lista.length()) {
    int coma = lista.indexOf(',', inicio);
    String token;

    if (coma == -1) {
      token = lista.substring(inicio);
      inicio = lista.length();
    } else {
      token = lista.substring(inicio, coma);
      inicio = coma + 1;
    }

    token.trim();
    if (token.length() == 0) continue;

    // HOTEL
    if (categoria == "HOTEL") {
      CRGB color = getColorGrupo(BRIGHTNESS_HOTEL, encender);
      int nivel = token.toInt();

      if (nivel >= 1 && nivel <= 5) { // Nivel 6 fue separado
        setColorZona(ledsHotel[nivel - 1], obtenerLedsHoteleria(nivel), color);
      } else {
        Serial.printf("Nivel de hotel inválido o reasignado: %s\n", token.c_str());
      }
    }

    // DEPARTAMENTO / RESIDENCIAS
    else if (categoria == "DEP") {
      CRGB color = getColorGrupo(BRIGHTNESS_RESIDENCIAS, encender);
      int idxDepto = obtenerIndiceDepartamento(token);

      if (idxDepto == -1) {
        Serial.printf("Departamento no encontrado: %s\n", token.c_str());
        continue;
      }

      const Departamento& d = departamentos[idxDepto];
      int idxNivel = getIndiceDeptoNivel(d.nivel);

      if (idxNivel == -1) {
        Serial.printf("Nivel de departamento inválido: %d\n", d.nivel);
        continue;
      }

      for (int i = 0; i < d.numLeds; i++) {
        int ledIdx = d.leds[i];
        if (ledIdx >= 0 && ledIdx < NUM_LEDS_DEPTO_NIVEL[idxNivel]) {
          ledsDeptos[idxNivel][ledIdx] = color;
        } else {
          Serial.printf("LED fuera de rango en departamento %s: %d\n", d.nombre, ledIdx);
        }
      }
    }
  }
}

// ============================================================
// COMANDO PRINCIPAL
// ============================================================
void procesarComando(String comando) {
  comando.trim();
  comando.toUpperCase();

  Serial.println();
  Serial.println("==============================");
  Serial.print("Comando: ");
  Serial.println(comando);

  if (comando == "ALL_ON") {
    encenderTodo();
    return;
  }
  if (comando == "ALL_OFF") {
    apagarTodo();
    return;
  }

  int firstColon = comando.indexOf(':');
  int secondColon = comando.indexOf(':', firstColon + 1);

  if (firstColon <= 0) {
    Serial.println("ERROR: formato de comando inválido.");
    return;
  }

  String categoria = comando.substring(0, firstColon);
  String accion;
  String parametros;

  if (secondColon > 0) {
    accion = comando.substring(firstColon + 1, secondColon);
    parametros = comando.substring(secondColon + 1);
  } else {
    accion = comando.substring(firstColon + 1);
  }

  accion.trim();
  parametros.trim();
  bool encender = (accion == "ON");

  if (accion != "ON" && accion != "OFF") {
    Serial.print("ERROR: acción desconocida: ");
    Serial.println(accion);
    return;
  }

  CRGB colorA = getColorGrupo(BRIGHTNESS_AMENIDADES, encender);

  // ==========================================================
  // AMENIDADES UNIFICADAS (ALL y LISTAS)
  // ==========================================================
  if (categoria == "AMENIDADES") {
    
    if (parametros == "ALL" || parametros == "") {
      setColorZona(tiraAlbercas, NUM_LEDS_ALBERCAS, colorA);
      setColorZona(ledsAcceso, NUM_LEDS_LOBI, colorA);
      setColorZona(ledsSalon, NUM_LEDS_SALON, colorA);
      
      ledsPasillosUnificados[11] = colorA; // RP
      ledsPasillosUnificados[12] = colorA; // RP
      ledsPasillosUnificados[13] = colorA; // RP
      ledsPasillosUnificados[14] = colorA; // RP
      
      ledsDeptos[0][0]  = colorA; // LB1
      ledsDeptos[0][18] = colorA; // LB2
      ledsDeptos[0][15] = colorA; // LB3
      ledsDeptos[0][13] = colorA; // RG
      ledsDeptos[0][16] = colorA; // OL
      ledsDeptos[0][17] = colorA; // OL
    } 
    else {
      int inicio = 0;
      while (inicio < parametros.length()) {
        int coma = parametros.indexOf(',', inicio);
        String token;
        
        if (coma == -1) {
          token = parametros.substring(inicio);
          inicio = parametros.length();
        } else {
          token = parametros.substring(inicio, coma);
          inicio = coma + 1;
        }
        
        token.trim();
        if (token.length() == 0) continue;
        
        // Amenidades excluyendo los LEDs de Restaurantes
        if (token == "ALBERCAS") {
          setColorZona(tiraAlbercas, NUM_LEDS_ALBERCAS - 4, colorA);         
          ledsAcceso[9] = colorA; //esto porque de la tira lobi (acceso) estoy quitanto esos 3 lógicamente
                                //para que se unifiquen con el comando ALBERCA 
          ledsAcceso[8] = colorA;
          ledsAcceso[7] = colorA;
        } 
        else if (token == "ACCESO") setColorZona(ledsAcceso, NUM_LEDS_LOBI - 4, colorA);
        else if (token == "SALON") setColorZona(ledsSalon, NUM_LEDS_SALON - 1, colorA); // SALLROOM
        
        // Restaurantes (apuntando a los últimos índices correspondientes)
        else if(token == "O_SPA") { // OUTDOOR SPA
          tiraAlbercas[30] = colorA;
        }
        else if (token == "POOL_BG") {  // POOL BAR & GRILL
          tiraAlbercas[31] = colorA;
          tiraAlbercas[32] = colorA; 
          tiraAlbercas[33] = colorA; 
        }
        else if (token == "REST_LEBAN") ledsSalon[4] = colorA; // SPECIALTY RESTAURANT LEBANESE / MEDITERRANEAN
        else if (token == "REST_YERBAMORA") ledsAcceso[10] = colorA; // SPECIALTY RESTAURANT YERBAMORA
        
        // Otras Amenidades Lógicas
        else if (token == "RP") { 
          ledsPasillosUnificados[11] = colorA; 
          ledsPasillosUnificados[12] = colorA; 
          ledsPasillosUnificados[13] = colorA; 
          ledsPasillosUnificados[14] = colorA; 
        }
        else if (token == "LB1") ledsDeptos[0][0] = colorA;
        else if (token == "LB2") ledsDeptos[0][18] = colorA;
        else if (token == "LB3") ledsDeptos[0][15] = colorA;
        else if (token == "RG") ledsDeptos[0][13] = colorA; 
        else if (token == "OL") { 
          ledsDeptos[0][16] = colorA; 
          ledsDeptos[0][17] = colorA; 
        }
        else {
          Serial.printf("Amenidad no reconocida en lista: %s\n", token.c_str());
        }
      }
    }
  }

  // ==========================================================
  // PASILLOS
  // ==========================================================
  else if (categoria == "PASILLOS") {
    if (parametros == "ALL" || parametros == "") {
      CRGB colorP = getColorGrupo(BRIGHTNESS_PASILLOS, encender);
      setColorZona(ledsPasillosUnificados, 11, colorP);
      for (int i = 0; i < NUM_LEDS_PASILLO_N6; i++) {
        ledsDeptos[0][INDICES_PASILLO_N6[i]] = colorP;
      }
    } else {
      Serial.println("Comando de lista por pasillo descartado.");
    }
  }

  // ==========================================================
  // ZONAS GENERALES
  // ==========================================================
  else if (categoria == "PB") {
    setColorZona(ledsPB, NUM_LEDS_PB, getColorGrupo(BRIGHTNESS_PB, encender));
  }
  else if (categoria == "SOTANO") {
    setColorZona(ledsSotano, NUM_LEDS_SOTANO, getColorGrupo(BRIGHTNESS_SOTANO, encender));
  }
  else if (categoria == "HOTEL") {
    if (parametros == "ALL") {
      CRGB colorH = getColorGrupo(BRIGHTNESS_HOTEL, encender);
      for (int nivel = 1; nivel <= 5; nivel++) {
        setColorZona(ledsHotel[nivel - 1], obtenerLedsHoteleria(nivel), colorH);
      }
    } else {
      aplicarComandoLista(parametros, encender, "HOTEL");
    }
  }
  // ==========================================================
  // ZONAS GENERALES
  // ==========================================================
  // ... (código previo) ...

  else if (categoria == "DEP") {
    if (parametros == "ALL") {
      CRGB colorR = getColorGrupo(BRIGHTNESS_RESIDENCIAS, encender);
      for (int i = 0; i < 10; i++) {
        setColorZona(ledsDeptos[i], NUM_LEDS_DEPTO_NIVEL[i], colorR);
      }
    } else {
      // --- LÓGICA DE REEMPLAZO ---
      // Si el comando es para ENCENDER una lista nueva, primero apagamos todos los departamentos
      if (encender) {
        CRGB colorApagado = CRGB::Black;
        for (int i = 0; i < 10; i++) {
          setColorZona(ledsDeptos[i], NUM_LEDS_DEPTO_NIVEL[i], colorApagado);
        }
      }
      
      // Después de limpiar (o si era un comando OFF), aplicamos la lista
      aplicarComandoLista(parametros, encender, "DEP");
    }
  }
  else {
    Serial.print("ERROR: categoría desconocida: ");
    Serial.println(categoria);
    return;
  }

  FastLED.show();
  Serial.println("OK");
}

// ============================================================
// ETHERNET
// ============================================================
void inicializarEthernet() {
  pinMode(ETH_RST_PIN, OUTPUT);
  digitalWrite(ETH_RST_PIN, LOW);
  delay(100);
  digitalWrite(ETH_RST_PIN, HIGH);
  delay(200);

  SPI.begin(ETH_SCK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);
  Ethernet.init(ETH_CS_PIN);
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  delay(100);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) Serial.println("ERROR: W5500 no detectado.");
  else Serial.println("W5500 detectado.");

  if (Ethernet.linkStatus() == LinkOFF) Serial.println("ADVERTENCIA: cable Ethernet desconectado.");
  else Serial.println("Enlace Ethernet activo.");

  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
  Udp.begin(UDP_PORT);
  Serial.print("UDP escuchando en puerto: ");
  Serial.println(UDP_PORT);
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n================================");
  Serial.println(" SISTEMA DE ILUMINACIÓN MAQUETA");
  Serial.println(" ESP32-S3 + W5500 + WS2815");
  Serial.println("================================");
  delay(200);

  inicializarEthernet();

  // AMENIDADES INDIVIDUALES
  FastLED.addLeds<WS2812B, PIN_LEDS_AMENIDADES_ALBERCAS, COLOR_ORDER>(tiraAlbercas, NUM_LEDS_ALBERCAS);
  FastLED.addLeds<WS2812B, PIN_LEDS_AMENIDADES_ACCESO, COLOR_ORDER>(ledsAcceso, NUM_LEDS_LOBI);
  FastLED.addLeds<WS2812B, PIN_LEDS_AMENIDADES_SALON, COLOR_ORDER>(ledsSalon, NUM_LEDS_SALON);

  // ZONAS GENERALES
  FastLED.addLeds<WS2812B, PIN_LEDS_PB, COLOR_ORDER>(ledsPB, NUM_LEDS_PB);
  FastLED.addLeds<WS2812B, PIN_LEDS_SOTANO, COLOR_ORDER>(ledsSotano, NUM_LEDS_SOTANO);

  // HOTELERIA (Niveles 1 a 5)
  FastLED.addLeds<WS2812B, PIN_HOTEL_NIVEL_1, COLOR_ORDER>(ledsHotel[0], 30);
  FastLED.addLeds<WS2812B, PIN_HOTEL_NIVEL_2, COLOR_ORDER>(ledsHotel[1], 30);
  FastLED.addLeds<WS2812B, PIN_HOTEL_NIVEL_3, COLOR_ORDER>(ledsHotel[2], 30);
  FastLED.addLeds<WS2812B, PIN_HOTEL_NIVEL_4, COLOR_ORDER>(ledsHotel[3], 30);
  FastLED.addLeds<WS2812B, PIN_HOTEL_NIVEL_5, COLOR_ORDER>(ledsHotel[4], 30);

  // PASILLOS UNIFICADOS
  FastLED.addLeds<WS2812B, PIN_LEDS_CORREDORES, COLOR_ORDER>(ledsPasillosUnificados, 15);

  // DEPARTAMENTOS / RESIDENCIAS (Agregado Nivel 6 al inicio)
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_6, COLOR_ORDER>(ledsDeptos[0], 19);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_7, COLOR_ORDER>(ledsDeptos[1], 36);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_8, COLOR_ORDER>(ledsDeptos[2], 36);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_9, COLOR_ORDER>(ledsDeptos[3], 36);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_10, COLOR_ORDER>(ledsDeptos[4], 36);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_11, COLOR_ORDER>(ledsDeptos[5], 33);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_12, COLOR_ORDER>(ledsDeptos[6], 39);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_14, COLOR_ORDER>(ledsDeptos[7], 40);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_15, COLOR_ORDER>(ledsDeptos[8], 39);
  FastLED.addLeds<WS2812B, PIN_LEDS_NIVEL_16, COLOR_ORDER>(ledsDeptos[9], 39);

  FastLED.setBrightness(255);
  FastLED.clear(true);
  delay(50);
  FastLED.clear(true);
  //FastLED.show();
  Serial.println("Sistema inicializado con brillos independientes por grupo.");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize <= 0) return;

  int len = Udp.read(packetBuffer, MAX_UDP_BUFFER - 1);
  if (len <= 0) return;

  packetBuffer[len] = '\0';
  String comando = String(packetBuffer);

  Serial.print("UDP recibido: ");
  Serial.println(comando);
  procesarComando(comando);
}