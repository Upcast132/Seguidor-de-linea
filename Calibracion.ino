// CalibracionSensores.ino
// Lee 4 sensores TCRT5000 en A0..A3 y calcula valores medios para BLANCO y NEGRO
// Uso: conecta sensores a A0..A3. Abre Serial, sigue instrucciones (escribe 'w' y Enter cuando esté sobre BLANCO,
// luego coloca sobre NEGRO y escribe 'b' y Enter). Al final imprime umbrales por sensor.

const int sensorPins[4] = {A0, A1, A2, A3};
const int muestras = 200;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("Calibracion TCRT5000 - 4 sensores"));
  Serial.println(F("Conecta sensores a A0 A1 A2 A3."));
  Serial.println(F("Cuando estés listo coloca sensores sobre BLANCO y escribe 'w' y Enter"));
}

long leerPromedio(int pin, int n) {
  long suma = 0;
  for (int i = 0; i < n; i++) {
    suma += analogRead(pin);
    delay(2);
  }
  return suma / n;
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'w') {
      // Leer blanco
      long blanco[4];
      Serial.println(F("Midiendo BLANCO... mantén los sensores sobre el blanco"));
      for (int i = 0; i < 4; i++) {
        blanco[i] = leerPromedio(sensorPins[i], muestras);
      }
      Serial.println(F("Listo. Ahora coloca los sensores sobre la LINEA (NEGRO) y escribe 'b' y Enter"));
      // esperar 'b'
      while (!Serial.available()) { delay(10); }
      // limpiar buffer
      while (Serial.available()) Serial.read();
      // esperar 'b' comando
      while (true) {
        if (Serial.available()) {
          char d = Serial.read();
          if (d == 'b') break;
        }
        delay(10);
      }
      long negro[4];
      Serial.println(F("Midiendo NEGRO... mantén los sensores sobre la linea"));
      for (int i = 0; i < 4; i++) {
        negro[i] = leerPromedio(sensorPins[i], muestras);
      }
      Serial.println(F("Calculos:"));
      for (int i = 0; i < 4; i++) {
        long umbral = (blanco[i] + negro[i]) / 2;
        Serial.print("Sensor "); Serial.print(i+1);
        Serial.print(" -> BLANCO: "); Serial.print(blanco[i]);
        Serial.print("  NEGRO: "); Serial.print(negro[i]);
        Serial.print("  UMBRAL: "); Serial.println(umbral);
      }
      Serial.println(F("Copia los valores UMBRAL para el seguidor y pégalos en el archivo del seguidor."));
      Serial.println(F("Si la diferencia BLANCO-NEGRO es muy pequeña (ej. <30) intenta ajustar potenciómetro o distancia."));
      Serial.println(F("Para repetir, coloca sobre BLANCO y escribe 'w' y Enter"));
    } // if 'w'
    // vaciar buffer
    while (Serial.available()) Serial.read();
  }
  delay(50);
}