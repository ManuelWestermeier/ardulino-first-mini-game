#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define MAX_SCREEN_X 19
#define MAX_SCREEN_Y 3
#define MIN_RENDER_COUNT 5
#define DEFAULT_RENDER_COUNT 20

int xPin = A0;      // Pin-Eingang für die horizontale Richtung vom Joystick
int yPin = A1;      // Pin-Eingang für die vertikale Richtung vom Joystick
int swPin = 2;      // Pin-Eingang vom Joystick, der prüft, ob gedrückt wi
int buttonPin = 3;  // Pin-Eingang für den Push-Button
int ledPin = 9;     // Pin-Ausgang für die LED

int jumpForce = 2;
int renderCount = DEFAULT_RENDER_COUNT;

int score = 0;

byte manCharacter[8] = {
  B01110, B01110, B00100, B11111, B01110, B01110,
  B01010,
  B01010
};

byte pointCharacter[8] = {
  B01011, B00111, B00100, B01110, B11111, B11111,
  B11111,
  B01110
};

byte enemyCharacter[8] = {
  B00100,
  B00100,
  B00100,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100
};

byte background[8] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111
};

byte pocal[8] = {
  B11111, B11111, B11111, B01110, B00100, B00100,
  B01110,
  B11111
};

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, manCharacter);
  lcd.createChar(1, pointCharacter);
  lcd.createChar(2, enemyCharacter);
  lcd.createChar(3, background);
  lcd.createChar(4, pocal);

  pinMode(swPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Draw();
}

void Clear() {
  lcd.clear();
}

void loop() {
  Update();
  delay(100);
}

struct Pos {
  int x = 0;
  int y = 0;

  Pos(int cx, int cy) {
    x = cx;
    y = cy;
  }

  bool canDraw() {
    //is in screen
    return x > -1 && x < 20 && y < 4 && y > -1;
  }

  bool collidesWith(Pos other) {
    //the same position
    return other.x == x && other.y == y;
  }
};

struct Player {
  Pos pos = { 2, MAX_SCREEN_Y };
  bool isColliding = true;

  // to jump smooth
  int jumpState = 0;

  bool canFallDown() {
    //on bottom of screen
    return pos.y < MAX_SCREEN_Y;
  }

  bool isCollidningWithGround() {
    //on bottom of the screen
    return pos.y == MAX_SCREEN_Y;
  }

  void Update(int h, int v, bool jump) {

    // joystick is pressed or verticalInput is up
    if ((jump || v > 950) && isColliding) {
      pos.y--;
      jumpState = jumpForce;
    }

    //hump every frame while it is jumping
    if (jumpState-- > 0)
      pos.y--;

    else if (canFallDown())
      pos.y++;

    //horizontal movement

    if (h > 900 && pos.x < 19)
      pos.x++;

    if (h < 100 && pos.x > 0)
      pos.x--;

    //check colliding
    isColliding = isCollidningWithGround();
  }
};

struct Ball {
  //random spawn
  Pos pos = { random(10, MAX_SCREEN_X), random(10, 4) };
  Pos renderPos = { pos.x, pos.y };

  int state = 15;

  void Update() {
    //smooth change position
    if (!pos.collidesWith(renderPos)) {
      if (pos.x > renderPos.x) renderPos.x++;
      if (pos.x < renderPos.x) renderPos.x--;
      if (pos.y > renderPos.y) renderPos.y++;
      if (pos.y < renderPos.y) renderPos.y--;
    }

    if (--state > 0) return;

    //random move
    pos = { random(0, MAX_SCREEN_X), random(0, 4) };

    //go to random position if it goes out of the screen
    if (!pos.canDraw())
      pos = { random(0, MAX_SCREEN_X), random(0, 4) };

    state = 15;
  }
};

struct Enemy {
  //random spawn
  Pos pos = { random(10, MAX_SCREEN_X), random(10, 4) };
  Pos renderPos = { pos.x, pos.y };

  int state = 10;

  void Update() {
    //smooth change position
    if (!pos.collidesWith(renderPos)) {
      if (pos.x > renderPos.x) renderPos.x++;
      if (pos.x < renderPos.x) renderPos.x--;
      if (pos.y > renderPos.y) renderPos.y++;
      if (pos.y < renderPos.y) renderPos.y--;
    }

    if (--state > 0) return;

    //random move
    pos = { random(0, MAX_SCREEN_X), random(0, 4) };

    //go to random position if it goes out of the screen
    if (!pos.canDraw())
      pos = { random(0, MAX_SCREEN_X), random(0, 4) };

    state = 10;
  }
};

Player player;
Ball ball;
Enemy enemy;

struct ScoreSystem {
  void Init() {
    ball.pos = { random(10, MAX_SCREEN_X), random(0, 4) };
    ball.renderPos = { ball.pos.x, ball.pos.y };
    enemy.pos = { random(10, MAX_SCREEN_X), random(0, 4) };
    enemy.renderPos = { enemy.pos.x, enemy.pos.y };
    player.pos = { 2, MAX_SCREEN_Y };
  }

  void Blink() {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
  }

  void Win() {
    Blink();

    // LED blink pattern for win animation
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }

    // Scroll "You Scored!" message
    lcd.clear();
    String message = "!!!You Scored!!!";
    message[0] = 4;
    message[1] = 4;
    message[2] = 4;
    message[13] = 4;
    message[14] = 4;
    message[15] = 4;
    for (int position = 0; position < 16; position++) {
      lcd.setCursor(position, 1);
      lcd.print(message);
      delay(200);
      lcd.clear();
    }

    // Display score
    score++;
    lcd.setCursor(6, 2);
    lcd.print("Score: ");
    lcd.print(score);
    delay(1000);
    digitalWrite(ledPin, LOW);

    // Make game faster
    if (renderCount > MIN_RENDER_COUNT) {
      renderCount--;
    }

    Clear();
    Init();
  }

  void Loose() {
    Blink();

    // LED blink pattern for loose animation
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }

    // Scroll "!!!You Loose!!!" message
    lcd.clear();
    String message = "!!!You Loose!!!";
    message[0] = 2;
    message[1] = 2;
    message[2] = 2;
    message[12] = 2;
    message[14] = 2;
    message[15] = 2;
    for (int position = 0; position < 16; position++) {
      lcd.setCursor(position, 1);
      lcd.print(message);
      delay(200);
      lcd.clear();
    }

    // Scroll "Score: X" message
    String scoreMessage = "Score: " + String(score);
    for (int position = 0; position < 16; position++) {
      lcd.setCursor(position, 2);
      lcd.print(scoreMessage);
      delay(200);
      lcd.clear();
    }

    // Final display of the loose message and score without scrolling
    lcd.setCursor(2, 1);
    lcd.print("!!!You Loose!!!");
    lcd.setCursor(6, 2);
    lcd.print("Score: ");
    lcd.print(score);
    delay(2000);
    digitalWrite(ledPin, LOW);

    // Reset score and render count
    score = 0;
    renderCount = DEFAULT_RENDER_COUNT;

    Clear();
    Init();
  }
};

ScoreSystem scoreSystem;

void Update() {
  int h = analogRead(xPin);               // get the horizontal joystick input
  int v = analogRead(yPin);               // get the vertical joystick input
  bool jump = digitalRead(swPin) == LOW;  // check if the joystick button is pressed

  player.Update(h, v, jump);

  //collids with ball
  if (player.pos.collidesWith(ball.renderPos)) {
    scoreSystem.Win();
  }

  //collides with enemy
  if (player.pos.collidesWith(enemy.renderPos)) {
    scoreSystem.Loose();
  }

  ball.Update();

  enemy.Update();

  Draw();
}

void Draw() {
  lcd.noCursor();
  Clear();
  lcd.home();
  lcd.print(score);

  //use *.pos.canDraw() to check if the object is in screen to prevent errors
  //draw the figures

  if (player.pos.canDraw()) {
    lcd.setCursor(player.pos.x, player.pos.y);
    lcd.write(byte(0));
  }

  if (ball.renderPos.canDraw()) {
    lcd.setCursor(ball.renderPos.x, ball.renderPos.y);
    lcd.write(byte(1));
  }

  if (enemy.renderPos.canDraw()) {
    lcd.setCursor(enemy.renderPos.x, enemy.renderPos.y);
    lcd.write(byte(2));
  }
}