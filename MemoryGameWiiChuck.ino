#include <Wire.h>

#define NB_MOVEMENTS 5

const int redLed = 13;
const int yellowLedLeft = 12;
const int yellowLedRight = 11;
const int greenLed = 10;

const int ledOk = 9;
const int ledFail = 8;

volatile int movements[NB_MOVEMENTS];
volatile int selected_movements[NB_MOVEMENTS];
volatile unsigned index_movement = 0;

byte buffer[6]; // nunchuck data buffer
byte cnt = 0;

enum movementType {
  LEFT,
  RIGHT,
  UP,
  DOWN
};

void shutdownLed()
{
  digitalWrite(redLed, LOW);
  digitalWrite(yellowLedLeft, LOW);
  digitalWrite(yellowLedRight, LOW);
  digitalWrite(greenLed, LOW);

  digitalWrite(ledOk, LOW);
  digitalWrite(ledFail, LOW);
}

boolean checkInputMovements()
{
  for(int i = 0;i < NB_MOVEMENTS;i++){
    if(selected_movements[i] != movements[i])
      return false;
  }

  return true;
}

void fillDisplaySelectedMovements(){
  Serial.println("fillDisplaySelectedMovements begin");
  shutdownLed();
  for(int i = 0; i < NB_MOVEMENTS; i++){
    movementType movement = (movementType) random(0,4);

    switch(movement){
      case RIGHT:
        Serial.print("RIGHT ");
        digitalWrite(yellowLedRight, HIGH);
        break;
      case LEFT:
        Serial.print("LEFT ");
        digitalWrite(yellowLedLeft, HIGH);
        break;
      case UP:
        Serial.print("UP");
        digitalWrite(redLed, HIGH);
        break;
      case DOWN:
        Serial.print("DOWN");
        digitalWrite(greenLed, HIGH);
        break;
    }

    selected_movements[i] = movement;
    Serial.println("");
    delay(1000);
    shutdownLed();
    delay(1000);
  }
  Serial.println("fillDisplaySelectedMovements end");
}

void displayLedResult(bool result)
{
  digitalWrite((result ? ledOk : ledFail), HIGH);
  delay(3000);
}

void handshake()
{
  Wire.beginTransmission (0x52);
  Wire.write (0x00);
  Wire.endTransmission ();
}

void getStickPosition()
{
    Wire.requestFrom (0x52, 6);
    while (Wire.available())
    {
      buffer[cnt] = Wire.read();
      cnt++;
    }

    if (cnt < 5) { // com fail
      cnt = 0;
      getStickPosition();
      handshake();
    }

    cnt = 0;
    handshake();
}

void waitUntilStickIdle()
{
  byte joy_x_axis, joy_y_axis;

  Serial.println("waitUntilStickIdle begin");
  do {
    getStickPosition();
    joy_x_axis = buffer[0]; // joystick axe x (0-255)
    joy_y_axis = buffer[1]; // joystick axe y (0-255)

    Serial.println("Stick not idle : ");
    Serial.print(joy_x_axis);
    Serial.print('\t');
    Serial.println(joy_y_axis);

   } while(joy_x_axis > 150 || joy_x_axis < 50 ||
           joy_y_axis > 150 || joy_y_axis < 50);

  Serial.println("waitUntilStickIdle end");
}

void start_game()
{
  boolean movementDone = false;

  do {
    getStickPosition();
    byte joy_x_axis = buffer[0]; // joystick axe x (0-255)
    byte joy_y_axis = buffer[1]; // joystick axe y (0-255)

    Serial.print (joy_x_axis, DEC);
    Serial.print ("\t");
    Serial.println (joy_y_axis, DEC);

    if (joy_y_axis > 150) {
      digitalWrite(redLed, HIGH);
      movements[index_movement++] = UP;
      movementDone = true;
    }

    else if(joy_y_axis < 50) {
      digitalWrite(greenLed, HIGH);
      movements[index_movement++] = DOWN;
      movementDone = true;
    }

    else if (joy_x_axis > 150) {
      digitalWrite(yellowLedRight, HIGH);
      movements[index_movement++] = RIGHT;
      movementDone = true;
    }

    else if (joy_x_axis < 50) {
      digitalWrite(yellowLedLeft, HIGH);
      movements[index_movement++] = LEFT;
      movementDone = true;
    }

    delay(250);
  } while(!movementDone);

  waitUntilStickIdle();

  Serial.print("index_movement = ");
  Serial.println(index_movement);

  shutdownLed();

  if (index_movement == NB_MOVEMENTS) {
    Serial.println("index movement max reached");

    boolean result = checkInputMovements();
    Serial.print("checkInputMovements : ");
    Serial.println(result);

    shutdownLed();
    displayLedResult(result);
    shutdownLed();

    index_movement = 0;
    fillDisplaySelectedMovements();
  }
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  // init nunchuck
  Wire.beginTransmission (0x52);
  Wire.write (0x40);
  Wire.write (0x00);
  Wire.endTransmission ();

  pinMode(redLed, OUTPUT);
  pinMode(yellowLedLeft, OUTPUT);
  pinMode(yellowLedRight, OUTPUT);
  pinMode(greenLed, OUTPUT);

  pinMode(ledOk, OUTPUT);
  pinMode(ledFail, OUTPUT);

  randomSeed(analogRead(0));

  shutdownLed();
  getStickPosition();
  fillDisplaySelectedMovements();
}

void loop()
{
  start_game();
  delay (250);
}
