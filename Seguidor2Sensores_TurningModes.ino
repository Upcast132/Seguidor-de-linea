// Seguidor2Sensores_TurningModes.ino
// Seguidor de linea 2 sensores (A0 = izq, A1 = der) con modos de giro seleccionables.
// Pines L293N (ajusta si tu conexionado es distinto):
// IN1=13, IN2=12, ENA=11  -> motor izquierdo
// IN3=10, IN4=8,  ENB=9   -> motor derecho
//
// Umbrales (usa los valores que obtuviste en la calibracion)
const int SENSOR_IZQ_PIN = A0;
const int SENSOR_DER_PIN = A1;
const int UMBRAL_IZQ = 527;
const int UMBRAL_DER = 527;

// Histeresis y promedio
const int HISTERESIS = 20;
const int MUESTRAS = 6;

// Pines L293N
const int IN1 = 13;
const int IN2 = 12;
const int ENA = 11;
const int IN3 = 10;
const int IN4 = 8;
const int ENB = 9;

// Velocidades (0-255)
const int V_AVANCE = 200;
const int V_GIRO_SUAVE    = 140; // velocidad motor interior en modo suave
const int V_GIRO_FUERTE   = 220; // velocidad motor exterior en giros fuertes
const int V_BUSCAR        = 180; // velocidad durante la busqueda de la linea

// Modo de giro posible: descomenta uno de los siguientes (solo 1)
//#define TURN_MODE_SOFT    // Giro suave: motor interior más lento, exterior más rápido
#define TURN_MODE_STOP    // Giro deteniendo el motor interior (0) y motor exterior avanza
//#define TURN_MODE_REVERSE  // Giro agresivo: motor interior invierte, exterior avanza (giro en sitio)

// Comportamiento cuando ninguno detecta la linea:
// - gira hacia la ultima direccion donde vio la linea (lastSeen)
// - si nunca vio, gira hacia la derecha por defecto
int lastSeen = 0; // -1 = last seen left, 1 = last seen right, 0 = none yet

// estados previos para histeresis
bool estadoPrevIzq = false;
bool estadoPrevDer = false;

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  Serial.println("Seguidor 2 sensores con modos de giro");
  Serial.print("Umbral IZQ: "); Serial.println(UMBRAL_IZQ);
  Serial.print("Umbral DER: "); Serial.println(UMBRAL_DER);
#ifdef TURN_MODE_SOFT
  Serial.println("Modo de giro: SOFT (suave)");
#elif defined(TURN_MODE_STOP)
  Serial.println("Modo de giro: STOP (motor interior parado)");
#elif defined(TURN_MODE_REVERSE)
  Serial.println("Modo de giro: REVERSE (giro agresivo)");
#else
  Serial.println("Modo de giro: POR DEFECTO (SOFT)");
#endif
}

// Lectura promedio
int leerPromedio(int pin) {
  long s = 0;
  for (int i = 0; i < MUESTRAS; i++) { s += analogRead(pin); delay(2); }
  return s / MUESTRAS;
}

// Histeresis: evita parpadeo cuando la lectura está en el borde
bool detectarConHisteresis(int lectura, int umbral, bool previo) {
  if (previo) return lectura >= (umbral - HISTERESIS);
  else       return lectura >= (umbral + HISTERESIS);
}

void loop() {
  int vIzq = leerPromedio(SENSOR_IZQ_PIN);
  int vDer = leerPromedio(SENSOR_DER_PIN);

  bool detectIzq = detectarConHisteresis(vIzq, UMBRAL_IZQ, estadoPrevIzq);
  bool detectDer = detectarConHisteresis(vDer, UMBRAL_DER, estadoPrevDer);

  estadoPrevIzq = detectIzq;
  estadoPrevDer = detectDer;

  // Debug
  Serial.print("V_IZQ:"); Serial.print(vIzq);
  Serial.print(" D_IZQ:"); Serial.print(detectIzq);
  Serial.print("  |  V_DER:"); Serial.print(vDer);
  Serial.print(" D_DER:"); Serial.print(detectDer);
  Serial.println();

  // Lógica:
  // - detectIzq && detectDer -> adelante
  // - detectIzq && !detectDer -> linea a la izquierda -> girar a la izquierda
  // - !detectIzq && detectDer -> linea a la derecha -> girar a la derecha
  // - ninguno -> buscar girando hacia lastSeen (o derecha si lastSeen==0)

  if (detectIzq && detectDer) {
    // ambos detectan -> adelante
    lastSeen = 0; // estamos centrados
    moverAdelante(V_AVANCE, V_AVANCE);
  }
  else if (detectIzq && !detectDer) {
    // linea a la izquierda -> girar izquierda
    lastSeen = -1;
    girarIzquierda();
  }
  else if (!detectIzq && detectDer) {
    // linea a la derecha -> girar derecha
    lastSeen = 1;
    girarDerecha();
  }
  else {
    // ninguno detecta -> buscar
    if (lastSeen == -1) {
      // intenta girar a la izquierda buscando
      buscarIzquierda();
    } else {
      // por defecto o lastSeen == 1 -> buscar a la derecha
      buscarDerecha();
    }
  }

  delay(30);
}

// Implementaciones de movimientos:
// moverAdelante: establece dirección "adelante" según conexion. Si tu motor gira invertido,
// intercambia la polaridad (LOW/HIGH) en las rutinas correspondientes.

void moverAdelante(int velIzq, int velDer) {
  // motor izquierdo adelante
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, velIzq);
  // motor derecho adelante
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, velDer);
}

void parar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
}

void girarIzquierda() {
#ifdef TURN_MODE_SOFT
  // motor izquierdo más lento, derecho rápido
  moverAdelante(V_GIRO_SUAVE, V_GIRO_FUERTE);
#elif defined(TURN_MODE_STOP)
  // motor izquierdo parado, derecho avanza
  moverAdelante(0, V_GIRO_FUERTE);
#elif defined(TURN_MODE_REVERSE)
  // motor izquierdo en reversa, derecho adelante -> giro muy agresivo
  // invertir motor izquierdo: HIGH/LOW para atrás (según cableado)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, V_GIRO_FUERTE);
  // derecho adelante
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, V_GIRO_FUERTE);
#else
  moverAdelante(V_GIRO_SUAVE, V_GIRO_FUERTE);
#endif
}

void girarDerecha() {
#ifdef TURN_MODE_SOFT
  // motor derecho más lento, izquierdo rápido
  moverAdelante(V_GIRO_FUERTE, V_GIRO_SUAVE);
#elif defined(TURN_MODE_STOP)
  // motor derecho parado, izquierdo avanza
  moverAdelante(V_GIRO_FUERTE, 0);
#elif defined(TURN_MODE_REVERSE)
  // motor derecho en reversa, izquierdo adelante -> giro muy agresivo
  // invertir motor derecho: HIGH/LOW para atrás (según cableado)
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, V_GIRO_FUERTE);
  // izquierdo adelante
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, V_GIRO_FUERTE);
#else
  moverAdelante(V_GIRO_FUERTE, V_GIRO_SUAVE);
#endif
}

// Buscar cuando se pierde la linea
void buscarIzquierda() {
  // Gira suavemente hacia la izquierda buscando la linea
#ifdef TURN_MODE_REVERSE
  // en modo REVERSE, girar en sitio a izquierda
  girarIzquierda();
#else
  // en modos SOFT/STOP, girar con un movimiento que favorezca la izquierda
  // aquí detenemos motor izquierdo y movemos derecho para girar
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); analogWrite(ENB, V_BUSCAR);
#endif
}

void buscarDerecha() {
  // Gira suavemente hacia la derecha buscando la linea
#ifdef TURN_MODE_REVERSE
  girarDerecha();
#else
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); analogWrite(ENA, V_BUSCAR);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
#endif
}
