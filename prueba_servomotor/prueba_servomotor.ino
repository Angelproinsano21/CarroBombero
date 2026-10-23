#include <Servo.h>

const int PIN_SERVO = 2;
Servo servoPrueba;

void setup() {
  Serial.begin(9600);
  servoPrueba.attach(PIN_SERVO);
  servoPrueba.write(90); // Posición inicial en el centro
  delay(1000);
  Serial.println("--- Prueba de Servomotor SG90 ---");
}

void loop() {
  Serial.println("Posición: 0 grados");
  servoPrueba.write(0);
  delay(1000);

  Serial.println("Posición: 90 grados (Centro)");
  servoPrueba.write(90);
  delay(1000);

  Serial.println("Posición: 180 grados");
  servoPrueba.write(180);
  delay(1000);

  // Barrido suave para evaluar caídas de corriente
  Serial.println("Iniciando barrido progresivo...");
  for (int pos = 0; pos <= 180; pos += 5) {
    servoPrueba.write(pos);
    delay(30);
  }
  for (int pos = 180; pos >= 0; pos -= 5) {
    servoPrueba.write(pos);
    delay(30);
  }
  delay(1000);
}