/*Correr 
 * pipenv shell
 * platformio init --board uno
 * platformio run 
 * platformio run --target upload
 */

#include <Arduino.h>

// Definición de pines para los registros de desplazamiento 595
const int latchPin = 2;
const int clockPin = 3;
const int dataPin = 4;

// Definición de pines para el control de NES
const int latchPinNES = 9;
const int clockPinNES = 10;
const int dataPinNES = 11;
int estado = 0;
int control[8];
int i = 0;

// Definición de pin para el buzzer
const int buzzer = 13;

// Tamaño de la matriz de LED
const int matrixSize = 8;

// Variables para la posición de la serpiente y la fruta
int snakeX, snakeY;
int fruitX, fruitY;

// Dirección de movimiento de la serpiente
int direction;

// Longitud inicial de la serpiente y longitud máxima
int snakeLength;
const int maxSnakeLength = 64;

// Arreglo para representar la serpiente (x, y) y (x, y+1) y (x, y+2), ...
int snake[maxSnakeLength][2];

void spawnFruit ();
void updateSnake ();
void resetGame ();
void drawGame ();

void setup() {
  //Serial.begin(9600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(latchPinNES, OUTPUT);
  pinMode(clockPinNES, OUTPUT);
  pinMode(dataPinNES, INPUT); // El pin de datos del control de NES pull-up

  randomSeed(analogRead(0));

  // Inicializar la posición de la serpiente y la fruta
  snakeX = matrixSize / 2;
  snakeY = matrixSize / 2;
  direction = 1; // Iniciar moviéndose hacia la derecha

  spawnFruit();

  // Inicializar la serpiente con una longitud inicial
  snakeLength = 1;
  snake[0][0] = snakeX;
  snake[0][1] = snakeY;
}

void loop() {
  // Actualizar la posición de la serpiente
  updateSnake();

  // Dibujar la serpiente y la fruta en la matriz de LED
  drawGame();

  // Pequeño retardo para la visualización
  delay(300);
}

int readNESController() {
  digitalWrite(latchPinNES, HIGH);
  delayMicroseconds(12); // Espera para asegurar que el cambio de pin latch se complete
  digitalWrite(latchPinNES, LOW);
  delayMicroseconds(6); // Espera para asegurar que el cambio de pin latch se complete
  int controllerState = 0;
  for (i=0; i<8; i++){
    digitalWrite(clockPinNES, LOW);
    delayMicroseconds(6);
    // A B Se St U D L R
    estado = digitalRead(dataPinNES);
    if (estado == HIGH){
		if (i==0){
			controllerState = controllerState+128;
		}
		if (i==1){
			controllerState = controllerState+64;
		}
		if (i==2){
			controllerState = controllerState+32;
		}
		if (i==3){
			controllerState = controllerState+16;
		}
		if (i==4){
			controllerState = controllerState+8;
		}
		if (i==5){
			controllerState = controllerState+4;
		}
		if (i==6){
			controllerState = controllerState+2;
		}
		if (i==7){
			controllerState = controllerState+1;
		}
		}
    digitalWrite(clockPinNES, HIGH);
    delayMicroseconds(6);
  }
  //int controllerState = shiftIn(dataPinNES, clockPinNES, MSBFIRST);
   //Serial.println(controllerState);
   //Serial.print("******************");
  return controllerState;
}

void playCoinSound() {
  // Tocar el sonido de la moneda de Mario Bros.
  tone(buzzer, 1568, 100);  // Frecuencia de 1568 Hz (nota E7) durante 100 ms
  delay(120);  // Pequeño retardo para evitar que el sonido se corte abruptamente
}

void playDeathSound() {
  // Tocar la melodía de la muerte de Mario Bros.
  tone(buzzer, 1245, 150);  // Frecuencia de 1245 Hz (nota D#7) durante 150 ms
  delay(200);  // Retardo para simular la pausa entre notas
  tone(buzzer, 0, 150);  // Silencio durante 150 ms
  delay(200);  // Retardo para simular la pausa entre notas
  tone(buzzer, 1245, 150);  // Frecuencia de 1245 Hz (nota D#7) durante 150 ms
  delay(200);  // Retardo para simular la pausa entre notas
  tone(buzzer, 0, 150);  // Silencio durante 150 ms
  delay(200);  // Retardo para simular la pausa entre notas
  tone(buzzer, 1245, 150);  // Frecuencia de 1245 Hz (nota D#7) durante 150 ms
  delay(200);  // Retardo para simular la pausa entre notas
  tone(buzzer, 0, 150);  // Silencio durante 150 ms
  delay(200);  // Retardo para simular la pausa entre notas
}

void updateSnake() {

 int controllerState = readNESController();

  // Verificar los botones presionados en el control de NES
  if (controllerState == 247) {
    direction = 0; // Mover hacia arriba
  } else if (controllerState == 251) {
    direction = 2; // Mover hacia abajo
  } else if (controllerState == 254) {
    direction = 3; // Mover hacia la izquierda
  } else if (controllerState == 253) {
    direction = 1; // Mover hacia la derecha
  }

  // Actualizar la posición de la cabeza de la serpiente
  switch (direction) {
    case 0: // Arriba
      snakeY--;
      break;
    case 1: // Derecha
      snakeX++;
      break;
    case 2: // Abajo
      snakeY++;
      break;
    case 3: // Izquierda
      snakeX--;
      break;
  }

  // Verificar colisiones con las paredes
  if (snakeX < 0 || snakeX >= matrixSize || snakeY < 0 || snakeY >= matrixSize) {
    // Reiniciar juego si choca con las paredes
    resetGame();
    return;
  }

  // Verificar colisión con la fruta
  if (snakeX == fruitX && snakeY == fruitY) {
    // Aumentar longitud de la serpiente y respawn de la fruta
    if (snakeLength < maxSnakeLength) {
      snakeLength++;
    }
    spawnFruit();
  }

  // Mover el cuerpo de la serpiente
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];
  }

  // Actualizar la posición de la cabeza de la serpiente en el arreglo
  snake[0][0] = snakeX;
  snake[0][1] = snakeY;

  // Verificar colisión con el cuerpo de la serpiente
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX == snake[i][0] && snakeY == snake[i][1]) {
      // Reiniciar juego si choca con su propio cuerpo
      resetGame();
      return;
    }
  }
}

void drawGame() {
  // Crear un mapa de bits para la matriz de LED
  byte display[matrixSize] = {0};

  // Dibujar la serpiente en el mapa de bits
  for (int i = 0; i < snakeLength; i++) {
    display[snake[i][1]] |= (1 << (matrixSize - 1 - snake[i][0]));
  }

  // Dibujar la fruta en el mapa de bits
  display[fruitY] |= (1 << (matrixSize - 1 - fruitX));

  // Enviar datos a la matriz de LED utilizando registros de desplazamiento
  digitalWrite(latchPin, LOW);
  for (int i = matrixSize - 1; i >= 0; i--) {
    shiftOut(dataPin, clockPin, MSBFIRST, display[i]);
  }
  digitalWrite(latchPin, HIGH);
}

void spawnFruit() {
  // Generar una nueva posición para la fruta
  fruitX = random(matrixSize);
  fruitY = random(matrixSize);

  // Hacer sonar el sonido de la moneda de Mario Bros. cuando la serpiente come una manzana
  playCoinSound();

  // Verificar que la nueva posición no esté ocupada por la serpiente
  for (int i = 0; i < snakeLength; i++) {
    if (fruitX == snake[i][0] && fruitY == snake[i][1]) {
      // Si la posición está ocupada, volver a generarla
      spawnFruit();
      return;
    }
  }
}

void resetGame() {
  // Reiniciar las variables del juego
  snakeX = matrixSize / 2;
  snakeY = matrixSize / 2;
  direction = 1;

  spawnFruit();

  snakeLength = 1;
  snake[0][0] = snakeX;
  snake[0][1] = snakeY;

  // Hacer sonar la melodía de la muerte de Mario Bros. cuando muera la serpiente
  playDeathSound();
}
