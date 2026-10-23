// Definición de pines para el Puente H
const int ENA = 6;  // Control de velocidad Motor A (PWM)
const int ENB = 7;  // Control de velocidad Motor B (PWM)

const int IN1 = 22; // Dirección Motor A
const int IN2 = 23; // Dirección Motor A
const int IN3 = 24; // Dirección Motor B
const int IN4 = 25; // Dirección Motor B

const int VELOCIDAD = 150; // Velocidad de prueba (0 a 255)

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  // --- MOVER HACIA ADELANTE ---
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, VELOCIDAD);
  analogWrite(ENB, VELOCIDAD);
  delay(3000); // Avanza 3 segundos

  // --- MOVER HACIA ATRÁS ---
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, VELOCIDAD);
  analogWrite(ENB, VELOCIDAD);
  delay(3000); // Retrocede 3 segundos
}