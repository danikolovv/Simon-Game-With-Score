const uint8_t ledPins[] = {9, 10, 11, 12}; /*Номерата на пиновете за LEDs, използвани в играта*/
const uint8_t buttonPins[] = {2, 3, 4, 5}; /*Номерата на пиновете за бутоните, използвани в играта*/
#define SPEAKER_PIN 8 /*Номера на пина за говорителя*/


const int LATCH_PIN = A1; 
const int DATA_PIN = A0;  
const int CLOCK_PIN = A2; /*LATCH_PIN, DATA_PIN, CLOCK_PIN са обозначенията на пиновете за "Shift register"(използва се за контролиране на 7-сегментовия дисплей, показващ резултата) */

#define MAX_GAME_LENGTH 100 /*Максимална дължина на играта*/

const int gameTones[] = { NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5}; /*Звуците при натискането на отделните бутони*/


uint8_t gameSequence[MAX_GAME_LENGTH] = {0}; /*Масив за съхраняване на произволно генерираната последователност от игри*/
uint8_t gameIndex = 0;


void setup() { /*Задава режимите на пиновете за светодиоди, бутони, високоговорител и регистър за смяна и зарежда генератора на произволни числа с аналогово четене.*/
  Serial.begin(9600);
  for (byte i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  randomSeed(analogRead(A3));
}


const uint8_t digitTable[] = { /*Съхранява двоични стойности за показване на цифри 0-9 на 7-сегментния дисплей.*/
  0b11000000,
  0b11111001,
  0b10100100,
  0b10110000,
  0b10011001,
  0b10010010,
  0b10000010,
  0b11111000,
  0b10000000,
  0b10010000,
};
const uint8_t DASH = 0b10111111; /*Съхранява двоичната стойност за показване на символ "-" на 7-сегментния дисплей.*/

void sendScore(uint8_t high, uint8_t low) { /*Актуализира резултата на 7-сегментния дисплей с помощта на регистъра за смяна.*/
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, low);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, high);
  digitalWrite(LATCH_PIN, HIGH);
}

void displayScore() { /*Изчислява цифрите на резултата от индекса на играта и извиква sendScore(), за да ги покаже.*/
  int high = gameIndex % 100 / 10;
  int low = gameIndex % 10;
  sendScore(high ? digitTable[high] : 0xff, digitTable[low]);
}


void lightLedAndPlayTone(byte ledIndex) { /*Включва определения светодиод, възпроизвежда съответния тон на високоговорителя и след това изключва светодиода.*/
  digitalWrite(ledPins[ledIndex], HIGH);
  tone(SPEAKER_PIN, gameTones[ledIndex]);
  delay(300);
  digitalWrite(ledPins[ledIndex], LOW);
  noTone(SPEAKER_PIN);
}


void playSequence() { /*Преминава през последователността на играта и извиква lightLedAndPlayTone() за всеки елемент.*/
  for (int i = 0; i < gameIndex; i++) {
    byte currentLed = gameSequence[i];
    lightLedAndPlayTone(currentLed);
    delay(50);
  }
}


byte readButtons() { /*Проверява състоянието на бутоните и връща индекса на натиснатия (или освободен) бутон.*/
  while (true) {
    for (byte i = 0; i < 4; i++) {
      byte buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        return i;
      }
    }
    delay(1);
  }
}


void gameOver() { /*Отпечатва крайния резултат. Възпроизвежда мелодия с помощта на високоговорителя, за да покаже края на играта.
Настройва показването на резултата да показва символ "-". Добавя забавяне за визуален ефект.*/
  Serial.print("Game over! your score: ");
  Serial.println(gameIndex - 1);
  gameIndex = 0;
  delay(200);

  
  tone(SPEAKER_PIN, NOTE_DS5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_CS5);
  delay(300);
  for (byte i = 0; i < 10; i++) {
    for (int pitch = -10; pitch <= 10; pitch++) {
      tone(SPEAKER_PIN, NOTE_C5 + pitch);
      delay(5);
    }
  }
  noTone(SPEAKER_PIN);

  sendScore(DASH, DASH);
  delay(500);
}


bool checkUserSequence() { /*Сравнява натисканията на бутоните на играча с очакваната последователност от играта. Извиква lightLedAndPlayTone() за всяко натискане на бутон.
Връща true, ако последователностите съвпадат, false в противен случай.*/
  for (int i = 0; i < gameIndex; i++) {
    byte expectedButton = gameSequence[i];
    byte actualButton = readButtons();
    lightLedAndPlayTone(actualButton);
    if (expectedButton != actualButton) {
      return false;
    }
  }

  return true;
}


void playLevelUpSound() { /*Възпроизвежда мелодия с помощта на високоговорителя, за да покаже успешното завършване на ниво.*/
  tone(SPEAKER_PIN, NOTE_E4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_E5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_C5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G5);
  delay(150);
  noTone(SPEAKER_PIN);
}


void loop() { /*Позволява на потребителя да играе играта многократно.*/
  displayScore();
  
  
  gameSequence[gameIndex] = random(0, 4);
  gameIndex++;
  if (gameIndex >= MAX_GAME_LENGTH) {
    gameIndex = MAX_GAME_LENGTH - 1;
  }

  playSequence();
  if (!checkUserSequence()) {
    gameOver();
  }

  delay(300);

  if (gameIndex > 0) {
    playLevelUpSound();
    delay(300);
  }
}
