#include <Servo.h>

// ==========================================
// 1. MAPEO DE PINES (ARDUINO MEGA 2560)
// ==========================================
// --- Puente H L298N ---
#define ENA 6   
#define ENB 7   
#define IN1 22  
#define IN2 23  
#define IN3 24  
#define IN4 25  

// --- Sensor Ultrasónico HC-SR04 ---
#define TRIG 9
#define ECHO 8

// --- Servomotores SG90 ---
#define PIN_SERVO_ULTRA 2
#define PIN_SERVO_MANGUERA 5

// --- Actuador de Potencia (Bomba de Agua) ---
#define PIN_MOSFET 4

// --- Sensores de Flama (Analógicos) KY-026 ---
#define SENSOR_IZQ A0
#define SENSOR_CEN A1
#define SENSOR_DER A2

// ==========================================
// 2. PARÁMETROS Y UMBRALES
// ==========================================
// Compensación de motores para avance rectilíneo y fuerza de giro
int VEL_MOTOR_A = 98;  
int VEL_MOTOR_B = 110; 
int VEL_GIRO = 115;    

const int DISTANCIA_PARO = 25; // Distancia límite en cm

// --- Ajuste Balístico ---
const int POTENCIA_BOMBA = 130; // Nivel de potencia de disparo (Mantenido con digitalWrite por diseño del usuario)

// --- Umbrales de Percepción de Fuego ---
const int UMBRAL_RASTREO = 550;    // < 550 activa cacería
const int UMBRAL_ATAQUE_MAX = 150; // Proximidad de detención térmica (< 150)
const int UMBRAL_ATAQUE_MIN = 100; 

// Tiempos de patrullaje
const unsigned long INTERVALO_360 = 12000; 
unsigned long tiempoUltimo360 = 0;

int ultimaDireccionFuego = 0; // Memoria: 1 = Izq, 2 = Der, 0 = Ninguna

Servo servoUltra;
Servo servoManguera;
long distanciaFrontal;

// Telemetría Anti-Atasco
long distanciaAnterior = 0;
unsigned long tiempoUltimoChequeo = 0;
int contadorAtasco = 0;

// ==========================================
// 3. CONFIGURACIÓN INICIAL (SETUP)
// ==========================================
void setup() {
  Serial.begin(9600);
  
  // Motores
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Ultrasónico
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // MOSFET Bomba
  pinMode(PIN_MOSFET, OUTPUT);
  digitalWrite(PIN_MOSFET, LOW); // Bomba apagada al iniciar

  // Sensores Flama
  pinMode(SENSOR_IZQ, INPUT);
  pinMode(SENSOR_CEN, INPUT);
  pinMode(SENSOR_DER, INPUT);

  // Inicialización de Servos con desfase para proteger picos de corriente
  servoUltra.attach(PIN_SERVO_ULTRA);
  servoUltra.write(90); 
  delay(150); 

  servoManguera.attach(PIN_SERVO_MANGUERA);
  servoManguera.write(90);
  delay(350); 

  tiempoUltimo360 = millis();
}

// ==========================================
// 4. BUCLE PRINCIPAL (LOOP)
// ==========================================
void loop() {
  distanciaFrontal = medirDistancia();
  
  int valIzq = analogRead(SENSOR_IZQ);
  int valCen = analogRead(SENSOR_CEN);
  int valDer = analogRead(SENSOR_DER);

  // Evaluación directa por mínimos (sin promedios ambientales)
  bool hayFuego = (valIzq < UMBRAL_RASTREO) || (valCen < UMBRAL_RASTREO) || (valDer < UMBRAL_RASTREO);
  bool fuegoCentrado = (valCen <= valIzq) && (valCen <= valDer) && (valCen < UMBRAL_RASTREO);
  
  bool obstaculoEnfrente = (distanciaFrontal > 0 && distanciaFrontal <= DISTANCIA_PARO);
  bool objetivoAlcanzado = fuegoCentrado && ((valCen <= UMBRAL_ATAQUE_MAX && valCen >= UMBRAL_ATAQUE_MIN) || obstaculoEnfrente);

  // Actualización de la memoria inercial para búsqueda en punto ciego
  if (valIzq < UMBRAL_RASTREO && valIzq < valCen) ultimaDireccionFuego = 1;
  else if (valDer < UMBRAL_RASTREO && valDer < valCen) ultimaDireccionFuego = 2;

  // --- SISTEMA ANTI-ATASCO ---
  if (millis() - tiempoUltimoChequeo > 500) {
    if (!objetivoAlcanzado) { 
      if (abs(distanciaFrontal - distanciaAnterior) <= 2) {
        contadorAtasco++; 
      } else {
        contadorAtasco = 0; 
      }
      
      if (contadorAtasco >= 3) {
        maniobraEscape();
        contadorAtasco = 0; 
      }
    } else {
      contadorAtasco = 0; 
    }
    distanciaAnterior = distanciaFrontal;
    tiempoUltimoChequeo = millis();
  }

  // --- MÁQUINA DE ESTADOS LÓGICOS ---
  if (objetivoAlcanzado) {
    // ESTADO 3: EXTINGUIR INCENDIO
    apagarFuego(valIzq, valCen, valDer);
    ultimaDireccionFuego = 0; 
  } 
  else if (hayFuego) {
    // ESTADO 2: CAZERÍA VECTORIAL Y ALINEACIÓN
    tiempoUltimo360 = millis(); 

    if (valIzq < valCen && valIzq < valDer) {
      girarIzquierda(120); 
      detener(); delay(30);
    } 
    else if (valDer < valCen && valDer < valIzq) {
      girarDerecha(120); 
      detener(); delay(30);
    } 
    else if (obstaculoEnfrente) {
      rutinaEvasion(); 
    } 
    else {
      avanzar();
    }
  } 
  else if (ultimaDireccionFuego != 0) {
    // ESTADO 2.5: BÚSQUEDA POR PÉRDIDA EN PUNTO CIEGO
    rutinaBusquedaPuntoCiego();
  }
  else {
    // ESTADO 1: NAVEGACIÓN AUTÓNOMA Y PATRULLAJE
    if (millis() - tiempoUltimo360 > INTERVALO_360) {
      escaneo360();
      tiempoUltimo360 = millis();
    } 
    else if (obstaculoEnfrente) {
      rutinaEvasion();
    } 
    else {
      avanzar();
    }
  }
  
  delay(30); 
}

// ==========================================
// 5. RUTINAS DE EXTINCIÓN Y PATRULLAJE
// ==========================================

void apagarFuego(int valIzq, int valCen, int valDer) {
  detener();
  delay(150);

  // 1. REPOSICIONAMIENTO BALÍSTICO (Micro-reversa para ajustar parábola)
  retroceder(250); 
  detener();
  delay(200); 

  // 2. Apuntar manguera al gradiente térmico de mayor intensidad
  int anguloObjetivo = 90;
  if (valIzq < valCen && valIzq < valDer) {
    anguloObjetivo = 30;  
  } else if (valDer < valCen && valDer < valIzq) {
    anguloObjetivo = 150; 
  } else {
    anguloObjetivo = 90;  
  }

  servoManguera.write(anguloObjetivo);
  delay(200); 

  // 3. Encender bomba de agua (con parámetro ajustado a 130)
  digitalWrite(PIN_MOSFET, POTENCIA_BOMBA);

  // 4. Abanico de disparo térmico (±10°)
  for (int ciclo = 0; ciclo < 3; ciclo++) {
    for (int pos = anguloObjetivo - 10; pos <= anguloObjetivo + 10; pos += 2) {
      servoManguera.write(pos);
      delay(30);
    }
    for (int pos = anguloObjetivo + 10; pos >= anguloObjetivo - 10; pos -= 2) {
      servoManguera.write(pos);
      delay(30);
    }
  }

  // 5. Apagar bomba de agua
  digitalWrite(PIN_MOSFET, LOW);
  delay(100);

  // 6. Regresar manguera a posición de reposo
  servoManguera.write(90);
  delay(200);

  // Nota: Se omite el retroceso final para permitir evaluación iterativa
  // Si el fuego sobrevive, el loop reinicia la secuencia desde esta misma coordenada.
}

void rutinaBusquedaPuntoCiego() {
  detener();
  int limiteIntentos = 0;
  bool reEnganchado = false;

  while (limiteIntentos < 6 && !reEnganchado) {
    if (ultimaDireccionFuego == 1) {
      girarIzquierda(100);
    } else if (ultimaDireccionFuego == 2) {
      girarDerecha(100);
    }
    detener();
    delay(40);

    int vIzq = analogRead(SENSOR_IZQ);
    int vCen = analogRead(SENSOR_CEN);
    int vDer = analogRead(SENSOR_DER);

    if (vIzq < UMBRAL_RASTREO || vCen < UMBRAL_RASTREO || vDer < UMBRAL_RASTREO) {
      reEnganchado = true; 
    }
    limiteIntentos++;
  }
  ultimaDireccionFuego = 0; 
}

void escaneo360() {
  detener();
  delay(100);
  
  for (int i = 0; i < 8; i++) {
    girarDerecha(180); 
    detener();
    delay(80);

    int vIzq = analogRead(SENSOR_IZQ);
    int vCen = analogRead(SENSOR_CEN);
    int vDer = analogRead(SENSOR_DER);

    if (vIzq < UMBRAL_RASTREO || vCen < UMBRAL_RASTREO || vDer < UMBRAL_RASTREO) {
      if (vIzq < vCen && vIzq < vDer) ultimaDireccionFuego = 1;
      else if (vDer < vCen && vDer < vIzq) ultimaDireccionFuego = 2;
      break; 
    }
  }
}

// ==========================================
// 6. RUTINAS DE EVASIÓN (ULTRASÓNICO)
// ==========================================

void rutinaEvasion() {
  detener();
  delay(200); 

  retroceder(350); 
  detener();
  delay(200);

  int mejorAngulo = barridoRadar();

  if (mejorAngulo > 90) {
    girarIzquierda(300); 
  } else {
    girarDerecha(300);
  }
  
  detener();
  delay(200);
}

int barridoRadar() {
  int distancias[5]; 
  int angulos[5] = {30, 60, 90, 120, 150};
  
  for(int pos = 90; pos >= 30; pos--) {
    servoUltra.write(pos);
    delay(15);
  }
  
  int indice = 0;
  for(int pos = 30; pos <= 150; pos++) {
    servoUltra.write(pos);
    delay(15); 
    
    if (pos == angulos[indice]) {
      delay(100); 
      distancias[indice] = medirDistancia();
      indice++;
    }
  }

  for(int pos = 150; pos >= 90; pos--) {
    servoUltra.write(pos);
    delay(15);
  }

  int maxDistancia = -1;
  int mejorAngulo = 90;
  
  for(int i = 0; i < 5; i++) {
    if(distancias[i] > maxDistancia) {
      maxDistancia = distancias[i];
      mejorAngulo = angulos[i];
    }
  }
  return mejorAngulo;
}

void maniobraEscape() {
  detener();
  retroceder(500); 
  detener();
  delay(100);
  girarIzquierda(450); 
  detener();
  delay(200);
}

// ==========================================
// 7. CONTROL DE MOTORES Y PUENTE H
// ==========================================

void avanzar() {
  analogWrite(ENA, VEL_MOTOR_A);
  analogWrite(ENB, VEL_MOTOR_B);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void retroceder(int tiempo) {
  analogWrite(ENA, VEL_MOTOR_A);
  analogWrite(ENB, VEL_MOTOR_B);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(tiempo);
}

void girarIzquierda(int tiempo) {
  analogWrite(ENA, VEL_GIRO);
  analogWrite(ENB, VEL_GIRO);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(tiempo);
}

void girarDerecha(int tiempo) {
  analogWrite(ENA, VEL_GIRO);
  analogWrite(ENB, VEL_GIRO);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(tiempo);
}

void detener() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ==========================================
// 8. LECTURA SENSOR ULTRASÓNICO
// ==========================================
long medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  long duracion = pulseIn(ECHO, HIGH, 25000); 
  
  if (duracion == 0) {
    return 400; 
  }
  
  return duracion / 58.2; 
}
