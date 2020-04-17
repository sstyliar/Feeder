#include <LiquidCrystal.h>
#include <Time.h>
#include <TimeLib.h>
#include <Servo.h>

//Servo
const int servoPin = 6;
int servo_pos = 0;
int initialPos = 0;
int finalPos = 120;
bool is_feeding = false;
int delay_val = 5;
Servo myServo;

time_t t;

//Feeding sequence
int buttonState = 0;
int error_margin = 20; //Production: 20 
int default_interval = 4; //Defined in hours
int feedingInterval;
int staticInterval; //The static value of the feding interval, used for checking if its time to feed the beast
int feedingCount = 0;
const int feedButtonPin = 8;

//Lcd screen
LiquidCrystal lcd(2, 3, 10, 11, 12, 13);

//Setting sequence
const int setButtonPin = 4;
int setButtonState = 0;
const int upButtonPin = 5;
int upButtonState = 0;
const int downButtonPin = 7;
int downButtonState = 0;
bool is_setting = false;
typedef struct _s_mode {
  int hours;
  int minutes;
  bool is_setting;
  bool is_setting_hours;
  bool is_setting_minutes;
} setting_mode;

setting_mode *s_mode = malloc(sizeof(setting_mode));

void setup() {
  //Init setting values
  s_mode->hours = 1;
  s_mode->minutes = 0;
  s_mode->is_setting = false;
  s_mode->is_setting_hours = false;
  s_mode->is_setting_minutes = false;
  //Init lcd and say hi
  lcd.begin(16,2);
  delay(1000);
  lcd.print("Welcome...");
  delay(3000);
  //Setup pins and init servo
  pinMode(feedButtonPin, INPUT);
  pinMode(setButtonPin, INPUT);
  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);
  myServo.attach(servoPin);
  myServo.write(initialPos);
  //Setup the initial values for the feeding
  feedingInterval = default_interval * 60 * 60;
  staticInterval = feedingInterval;
  s_mode->hours = default_interval;
  s_mode->minutes = s_mode->hours / 60;
}

void loop() {
  t = now();
  setButtonState = digitalRead(setButtonPin);
  if (setButtonState == HIGH && !is_feeding) {
    s_mode->is_setting = true;
    delay(500); //give it time for the user to stop pressing the button
    if(!s_mode->is_setting_hours && !s_mode->is_setting_minutes){
      s_mode->is_setting_hours = true;
    } else if(s_mode->is_setting_hours){
      s_mode->is_setting_hours = false;
      s_mode->is_setting_minutes = true;
    }else{
      s_mode->is_setting = false;
      s_mode->is_setting_hours = false;
      s_mode->is_setting_minutes = false;
      reset();
    }
  }
  if (s_mode->is_setting) { //we are in the setting territory
    displaySettingScreen();
    delay(300);
  } else {//we are in the feeding territory
    checkOverrideButton(); //override if the beast is hella hungry
    displayTimeToNextFeed(feedingInterval);
    checkForFeed(t);
    delay(1000);
    feedingInterval--;
  }
}

void displaySettingScreen(){
  lcd.setCursor(0,0);
  lcd.print("Feed every:  h/m");
  int temp;
  char tempString[50];
  lcd.setCursor(0,1);
  temp = sprintf(tempString, "%02d:%02d          ", s_mode->hours, s_mode->minutes);
  lcd.print(tempString);
  upButtonState = digitalRead(upButtonPin);
  downButtonState = digitalRead(downButtonPin);
  if(s_mode->is_setting_hours && !s_mode->is_setting_minutes){
     lcd.setCursor(1,1);
     setHours(upButtonState, downButtonState);
  }else if(s_mode->is_setting_minutes){
    lcd.setCursor(4,1);     
    setMinutes(upButtonState, downButtonState);
  }
  lcd.blink();
}

void setHours(int upBtn, int downBtn){
  if (upBtn == HIGH && s_mode->hours < 12){
    s_mode->hours++;
  }
  if (downBtn == HIGH && s_mode->hours > 0){
     s_mode->hours--;
  }
}

void setMinutes(int upBtn, int downBtn){
  if (upBtn == HIGH && s_mode->minutes < 60){
    s_mode->minutes++;
  }
  if (downBtn == HIGH && s_mode->minutes > 0){
     s_mode->minutes--;
  }
}

void displayTimeToNextFeed(int interval) {
  displayLcdTime(interval);
}

void displayLcdTime(int sec){
  int temp, h, m, s;
  char dumpString[20];
  lcd.setCursor(0,0);
  lcd.print("TTNF: ");
  h = (sec / 3600);
  m = (sec - (3600 * h)) / 60;
  s = (sec - (3600 * h) - (m * 60));
  temp = sprintf(dumpString, "%02d:%02d:%02d", h, m, s);
  lcd.setCursor(6,0);
  lcd.print(dumpString);
  lcd.setCursor(0,1);
  temp = sprintf(dumpString, "%s: %d", "Fed count", feedingCount);
  lcd.print(dumpString);
}

void reset() {
  setTime(0);
  lcd.clear();
  lcd.noBlink();
  if(s_mode->hours == 0 && s_mode->minutes == 0){
    s_mode->hours = 4;
  }
  feedingInterval = (s_mode->hours * 60 * 60) + (s_mode->minutes * 60);
  staticInterval = feedingInterval;
}

void checkForFeed(time_t t) {
  if (t <= (staticInterval + error_margin) && t >= (staticInterval - error_margin)) {
    feed();
  }
}

void checkOverrideButton() {
  buttonState = digitalRead(feedButtonPin);
  if (buttonState == HIGH && !is_feeding) {
    feed();
  }
}

void feed() {
  is_feeding = true;
  roll_servo(1);
  delay(10);
  roll_servo(0);
  is_feeding = false;
  feedingCount++;
  reset();
}

void roll_servo(int direction_to_turn) { //0 -> clock wise, 1 -> counter clock wise
  if (!myServo.attached()) {
    myServo.attach(servoPin);
  }
  for (int i = 0; i <= finalPos; i++) {
    if (direction_to_turn) {
      myServo.write(i);
    } else {
      servo_pos = finalPos - i;
      myServo.write(servo_pos);
    }
    delay(delay_val);
  }
  myServo.detach();
}



//The "beast" is a bunny and its name is Fluffy Bunny the 1st,
//destroyer of furniture and defender of the holly pellet.
