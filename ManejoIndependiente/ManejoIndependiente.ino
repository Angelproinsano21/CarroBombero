#include <Servo.h>

// ==========================================
// 1. MAPEO DE PINES (ARDUINO MEGA)
// ==========================================
// --- Puente H L298N ---
#define ENA 6   // PWM Motor A
#define ENB 7   // PWM Motor B
#define IN1 22  // Dirección A1
#define IN2 23  // Dirección A2
#define IN3 24  // Dirección B1
#define IN4 25  // Dirección B2

// --- Sensor Ultrasónico ---
#define TRIG 9
#define ECHO 8

// --- Servomotor ---
#define PIN_SERVO_ULTRA 2

// ==========================================
// 2. PARÁMETROS DE CONFIGURACIÓN
// ==========================================
// Ajuste de compensación por llantas/variables invertidas.
// Si notas que gira hacia un lado en vez de ir recto, intercambia estos dos valores:
int VEL_MOTOR_A = 98;  
int VEL_MOTOR_B = 110; 
int VEL_GIRO = 115;    // Velocidad un poco mayor para vencer la fricción al girar

const int DISTANCIA_PARO = 25; // 25 cm para tener margen de maniobra

// Variables de objetos y lógica
Servo servoUltra;
long distanciaFrontal;

// Telemetría Anti-Atasco
long distanciaAnterior = 0;
unsigned long tiempoUltimoChequeo = 0;
int contadorAtasco = 0;

// ==========================================
// 3. CONFIGURACIÓN INICIAL (SETUP)
// ==========================================
void setup() {
  Serial.begin(9600); // Para monitorear en computadora si es necesario
  
  // Motores
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Sensor Ultrasónico
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // Servo
  servoUltra.attach(PIN_SERVO_ULTRA);
  servoUltra.write(90); // Iniciar mirando al frente
  delay(500); // Esperar a que el servo llegue a su posición
}

// ==========================================
// 4. BUCLE PRINCIPAL (LOOP)
// ==========================================
void loop() {
  distanciaFrontal = medirDistancia();

  // --- SISTEMA ANTI-ATASCO ---
  // Revisa cada 500ms si el robot realmente está avanzando
  if (millis() - tiempoUltimoChequeo > 500) {
    if (abs(distanciaFrontal - distanciaAnterior) <= 2) {
      contadorAtasco++; // Si la diferencia es menor a 2cm, sumamos advertencia
    } else {
      contadorAtasco = 0; // Si se mueve bien, reiniciamos el contador
    }
    distanciaAnterior = distanciaFrontal;
    tiempoUltimoChequeo = millis();

    // Si pasaron 1.5 seg (3 conteos) atascado...
    if (contadorAtasco >= 3) {
      maniobraEscape();
      contadorAtasco = 0; // Reiniciar tras escapar
    }
  }

  // --- LÓGICA DE NAVEGACIÓN ---
  if (distanciaFrontal > 0 && distanciaFrontal <= DISTANCIA_PARO) {
    // OBSTÁCULO DETECTADO
    rutinaEvasion();
  } else {
    // CAMINO LIBRE
    avanzar();
  }
  
  delay(50); // Pequeña pausa para estabilizar las lecturas del bucle
}

// ==========================================
// 5. FUNCIONES DE COMPORTAMIENTO
// ==========================================

void rutinaEvasion() {
  detener();
  delay(200); // Reposo inercial

  // 1. Reversa táctica para hacer espacio a la rueda loca (evitar choque trasero)
  retroceder(350); // 350 milisegundos de reversa
  detener();
  delay(200);

  // 2. Escaneo de Radar
  int mejorAngulo = barridoRadar();

  // 3. Tomar decisión de giro
  // Asumiendo que 90 es frente, > 90 es Izquierda, < 90 es Derecha
  if (mejorAngulo > 90) {
    girarIzquierda(300); // Ajusta este tiempo según cuánto quieras que gire
  } else {
    girarDerecha(300);
  }
  
  detener();
  delay(200);
}

int barridoRadar() {
  int distancias[5]; // Guardará las medidas de 30°, 60°, 90°, 120°, 150°
  int angulos[5] = {30, 60, 90, 120, 150};
  
  // Mover suavemente el servo desde 90 hasta 30 para empezar a escanear
  for(int pos = 90; pos >= 30; pos--) {
    servoUltra.write(pos);
    delay(15);
  }
  
  // Escaneo suave de 30 a 150 pasando por los puntos clave
  int indice = 0;
  for(int pos = 30; pos <= 150; pos++) {
    servoUltra.write(pos);
    delay(15); // Velocidad suave del escáner
    
    // Si el servo pasa por uno de nuestros ángulos objetivo, tomamos lectura
    if (pos == angulos[indice]) {
      delay(100); // Pausa mínima para estabilizar vibración mecánica
      distancias[indice] = medirDistancia();
      indice++;
    }
  }

  // Volver suavemente al centro (90)
  for(int pos = 150; pos >= 90; pos--) {
    servoUltra.write(pos);
    delay(15);
  }

  // Evaluar cuál de las 5 medidas fue la mayor (más espacio libre)
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
  // Cuando el robot detecta que patinó o se atascó
  detener();
  retroceder(500); // Reversa más larga y desesperada
  detener();
  delay(100);
  girarIzquierda(450); // Giro fuerte para romper la trampa
  detener();
  delay(200);
}

// ==========================================
// 6. FUNCIONES DE CONTROL DE MOTORES
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
  // Lógica invertida para que coincida con tu hardware real
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(tiempo);
}

void girarDerecha(int tiempo) {
  analogWrite(ENA, VEL_GIRO);
  analogWrite(ENB, VEL_GIRO);
  // Lógica invertida para que coincida con tu hardware real
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
// 7. FUNCIÓN DEL SENSOR ULTRASÓNICO
// ==========================================
long medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  // Timeout de 25000 microsegundos (~400cm max) para evitar bloqueos del código
  long duracion = pulseIn(ECHO, HIGH, 25000); 
  
  if (duracion == 0) {
    return 400; // Si no hay eco en rango, devolver valor alto como camino libre
  }
  
  return duracion / 58.2; // Convertir a centímetros
}