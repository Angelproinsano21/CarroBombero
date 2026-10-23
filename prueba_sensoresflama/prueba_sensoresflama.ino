const int SENSOR_IZQ = A0;
const int SENSOR_CEN = A1;
const int SENSOR_DER = A2;

const int UMBRAL_FUEGO = 500; 

void setup() {
  Serial.begin(9600);
  Serial.println("--- Lectura Trío de Sensores de Flama (A0, A1, A2) ---");
}

void loop() {
  int valIzq = analogRead(SENSOR_IZQ);
  int valCen = analogRead(SENSOR_CEN);
  int valDer = analogRead(SENSOR_DER);

  Serial.print("IZQ (A0): ");
  Serial.print(valIzq);
  Serial.print(" | CEN (A1): ");
  Serial.print(valCen);
  Serial.print(" | DER (A2): ");
  Serial.print(valDer);

  if (valIzq < UMBRAL_FUEGO || valCen < UMBRAL_FUEGO || valDer < UMBRAL_FUEGO) {
    if (valCen <= valIzq && valCen <= valDer) {
      Serial.println("  -> ¡FUEGO DETECTADO EN EL CENTRO!");
    } else if (valIzq < valDer) {
      Serial.println("  -> ¡FUEGO DETECTADO A LA IZQUIERDA!");
    } else {
      Serial.println("  -> ¡FUEGO DETECTADO A LA DERECHA!");
    }
  } else {
    Serial.println("  -> Sin fuego (Estado normal)");
  }

  delay(300);
}