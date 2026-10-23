// ====================================================
// Código de Prueba de Motores con L298N y Arduino Mega
// ====================================================

// Definición de Pines Motor A (Izquierdo)
const int ENA = 2; // Pin PWM para velocidad
const int IN1 = 3; // Dirección 1
const int IN2 = 4; // Dirección 2

// Definición de Pines Motor B (Derecho)
const int IN3 = 5; // Dirección 1
const int IN4 = 6; // Dirección 2
const int ENB = 7; // Pin PWM para velocidad

void setup() {
  // Configurar todos los pines como salidas
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  // 1. Avanzar a Velocidad Media (150 de 255) durante 2 segundos
  moverAdelante(150);
  delay(2000);

  // 2. Parar durante 1 segundo
  detener();
  delay(1000);

  // 3. Avanzar a Máxima Velocidad (255 de 255) durante 2 segundos
  moverAdelante(255);
  delay(2000);

  // 4. Parar 1 segundo
  detener();
  delay(1000);

  // 5. Retroceder a Velocidad Media durante 2 segundos
  moverAtras(150);
  delay(2000);

  // 6. Parar 2 segundos antes de repetir el ciclo
  detener();
  delay(2000);
}

// ----------------------------------------------------
// Funciones de Control de Movimiento
// ----------------------------------------------------

void moverAdelante(int velocidad) {
  // Motor A Adelante
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, velocidad);

  // Motor B Adelante
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, velocidad);
}

void moverAtras(int velocidad) {
  // Motor A Atrás
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, velocidad);

  // Motor B Atrás
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, velocidad);
}

void detener() {
  // Apagar ambos motores
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
}