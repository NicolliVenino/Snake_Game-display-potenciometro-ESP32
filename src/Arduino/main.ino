#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define MAX_POINTS 64
int graphData[MAX_POINTS];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define NUM_OBSTACLES 8
int obstacleX[NUM_OBSTACLES];
int obstacleY[NUM_OBSTACLES];

int obstacleSpeed[NUM_OBSTACLES];

bool collisionDetected = false;

#define BTN_PIN 32
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 800;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  pinMode(BTN_PIN, INPUT_PULLUP);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Display OLED não encontrado"));
    while (true);
  }
  display.clearDisplay();
  display.display();
  randomSeed(analogRead(0));

  for (int i = 0; i < NUM_OBSTACLES; i++) {
    obstacleX[i] = SCREEN_WIDTH - 1;
    obstacleY[i] = random(0, SCREEN_HEIGHT);
    obstacleSpeed[i] = random(1, 4);
  }
}

void updateGraph(int newValue) {
  for (int i = 0; i < MAX_POINTS - 1; i++) {
    graphData[i] = graphData[i + 1];
  }
  graphData[MAX_POINTS - 1] = newValue;
}

void drawGraph() {
  for (int x = 1; x < MAX_POINTS; x++) {
    int y1 = constrain(graphData[x - 1], 0, 63);
    int y2 = constrain(graphData[x], 0, 63);
    display.drawLine(x - 1, y1, x, y2, WHITE);
  }
}

void drawObstacle() {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
    display.fillRect(obstacleX[i], obstacleY[i], 2, 2, WHITE);
  }
}

void checkCollision() {
  int headX = MAX_POINTS - 1;               // Posição X da cabeça da cobra
  int headY = graphData[headX];             // Posição Y da cabeça da cobra

  for (int i = 0; i < NUM_OBSTACLES; i++) {
    // Verifica se o obstáculo está na mesma posição X da cabeça
    if (obstacleX[i] >= headX - 3 && obstacleX[i] <= headX) {
      // Verifica se a cabeça está até 2 pixels acima ou abaixo do obstáculo
      if (headY >= (obstacleY[i] - 3) && headY <= (obstacleY[i] + 3 + 1)) {
        collisionDetected = true;
        break;
      }
    }
  }
}

void loop() {
  if (!collisionDetected) {
    int sensor = analogRead(34);
    int y = map(sensor, 0, 4095, 63, 0);
    updateGraph(y);

    for (int i = 0; i < NUM_OBSTACLES; i++) {
      obstacleX[i] -= obstacleSpeed[i];

      if (obstacleX[i] < 0) {
        obstacleX[i] = SCREEN_WIDTH - 1;
        obstacleY[i] = random(0, SCREEN_HEIGHT);
        obstacleSpeed[i] = random(1, 4);
      }
    }

    checkCollision();

    display.clearDisplay();
    drawGraph();
    drawObstacle();
    display.display();

    delay(30);
  } else {
    // Agora o loop continua verificando o botão mesmo após a colisão
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5, 20);
    display.print("GAME OVER!");

    display.setTextSize(1);
    display.setCursor(10, 45);
    display.print("Pressione o botao.");

    display.display();

    if (digitalRead(BTN_PIN) == LOW && millis() - lastButtonPress > debounceDelay) {
      lastButtonPress = millis();
      Serial.println("Reiniciando ESP32...");
      ESP.restart();
    }
  }
}