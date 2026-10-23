const int TRIG = 9;
const int ECHO = 8;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  Serial.println("--- Prueba de Lectura Sensor Ultrasónico HC-SR04 ---");
}

void loop() {
  // Generar pulso de activación
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // Mide el tiempo de respuesta en microsegundos
  long duracion = pulseIn(ECHO, HIGH, 30000);
  int distancia = duracion * 0.034 / 2;

  if (duracion == 0) {
    Serial.println("Error: Sin lectura (Revisar conexiones VCC/GND/ECHO)");
  } else {
    Serial.print("Duración: ");
    Serial.print(duracion);
    Serial.print(" us | Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }

  delay(500);
}