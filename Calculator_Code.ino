// The Special Calculator
// Dunn (Qiyuan) Zhang
// This code will calculate the selected operations of the two inputs, and based on the circumstances, produce either a correct, or false result.
// For pin mapping, refer to the schematic provided in the project documentation site.

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>

/// https://www.allaboutcircuits.com/projects/use-a-keypad-with-your-arduino/

enum { INPUT1,                  // first row
       OPERATOR,                // second row
       INPUT2,                  // third row
       ENTER,                   // perform calcuations (do not alter result here)
       SOLUTION,                // alter the result, if prompted, and then display on screen
       DUNN } mode = INPUT1;    // "DONE" mark the operation as complete and clear all inputs

LiquidCrystal_I2C screen(0x27, 16, 4);

const byte rows = 4;
const byte cols = 3;

const int operatorPins[] = { 9, 10, 11, 12 };
int operatorState = 0;
int currOperator = 0;  // 0: None, 1: Add, 2: Subtract, 3: Multiply, 4: Division

// define key map, with assistance from Prof. Robert Zacharias

char keys[rows][cols] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

byte rowPins[rows] = { 4, 2, 8, 6 };
byte colPins[cols] = { 3, 7, 5 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

unsigned long timer;

long inputNum1;
long inputNum2;
long solution;

bool clearState = false;
bool overflow = false;
bool falseSolution = true;

int calculations = 0;

// Count Function - provided by ChatGPT

int countDigits(unsigned long num) {
  if (num == 0) return 1;  // Special case for 0, as log10(0) is undefined

  int count = 0;
  while (num > 0) {
    count++;
    num /= 10;
  }
  return count;
}

// switch digits in a number - provided by ChatGPT

long switchRandomDigits(long num) {
  String numStr = String(num);  // Convert the number to a string
  int len = numStr.length();
  if (len < 2) return num;    // No digits to switch if the number is a single digit
  int pos1 = random(0, len);  // Random position between 0 and len-1
  int pos2;
  do {
    pos2 = random(0, len);
  } while (pos1 == pos2);  // Ensure we get two distinct positions
  // Swap the digits
  char temp = numStr[pos1];
  numStr[pos1] = numStr[pos2];
  numStr[pos2] = temp;
  return numStr.toInt();  // Convert back to integer
}

void setup() {
  Serial.begin(9600);
  screen.init();
  screen.backlight();
  screen.home();

  for (int x = 0; x < 4; x++) {
    pinMode(operatorPins[x], INPUT_PULLUP);
  }
}

void loop() {

  bool floatSolution = false;

  switch (mode) {
    case INPUT1:
      {
        // Metal Jeypad
        char key1 = keypad.getKey();
        if (key1 and isdigit(key1)) {
          long digit = key1 - '0';
          if (inputNum1 <= (2147483647 - digit) / 10) {  // max of long
            inputNum1 = inputNum1 * 10 + digit;
            screen.setCursor(0, 0);
            screen.print(inputNum1);
            // Serial.println(inputNum1);
          }
        }
        // 1x4 Key strip (operators)
        for (int x = 0; x < 4; x++) {
          operatorState = digitalRead(operatorPins[x]);
          if (operatorState == LOW and operatorPins[x] == 10) {
            currOperator = 1;
          }
          if (operatorState == LOW and operatorPins[x] == 9) {
            currOperator = 2;
          }
          if (operatorState == LOW and operatorPins[x] == 11) {
            currOperator = 3;
          }
          if (operatorState == LOW and operatorPins[x] == 12) {
            currOperator = 4;
          }
        }
        if (currOperator != 0 and inputNum1 != 0) {                       
          mode = OPERATOR;
        }
        if (key1 == '#') {
          inputNum1 = 0;
          screen.clear();
        }
        break;
      }
    case OPERATOR:
      {
        for (int x = 0; x < 4; x++) {
          operatorState = digitalRead(operatorPins[x]);
          screen.setCursor(0, 1);
          if (operatorState == LOW and operatorPins[x] == 10) {
            currOperator = 1;
            // Serial.println("+");
            screen.print("+");
          }
          if (operatorState == LOW and operatorPins[x] == 9) {
            currOperator = 2;
            // Serial.println("-");
            screen.print("-");
          }
          if (operatorState == LOW and operatorPins[x] == 11) {
            currOperator = 3;
            // Serial.println("x");
            screen.print("x");
          }
          if (operatorState == LOW and operatorPins[x] == 12) {
            currOperator = 4;
            // Serial.println("÷");
            screen.print(char(0xfd));
          }
        }
        char key2 = keypad.getKey();
        if (key2 and isDigit(key2)) {
          if (currOperator != 0) {
            mode = INPUT2;
            inputNum2 = key2 - '0';
            screen.setCursor(0, 2);
            screen.print(inputNum2);
          }
        }

        break;
      }
    case INPUT2:
      {
        // Metal Jeypad
        char key3 = keypad.getKey();
        if (key3 and isdigit(key3)) {
          clearState = false;
          long digit = key3 - '0';
          if (inputNum2 <= (2147483647 - digit) / 10) {  // max of long
            inputNum2 = inputNum2 * 10 + digit;
            screen.setCursor(0, 2);
            screen.print(inputNum2);
          }
        }
        if (key3 == '*') {
          mode = ENTER;
        }
        // clear funtion
        if (key3 == '#' and clearState == false) {
          screen.clear();
          inputNum2 = 0;
          screen.setCursor(0, 0);
          screen.print(inputNum1);

          screen.setCursor(0, 1);
          if (currOperator == 1) {
            screen.print("+");
          }
          if (currOperator == 2) {
            screen.print("-");
          }
          if (currOperator == 3) {
            screen.print("x");
          }
          if (currOperator == 4) {
            screen.print(char(0xfd));
          }
          screen.setCursor(0, 2);
          screen.print(inputNum2);

          clearState = true;
        }
        key3 = keypad.getKey();
        if (key3 == '#' and clearState == true) {
          inputNum1 = 0;
          currOperator = 0;
          inputNum2 = 0;
          mode = INPUT1;
          screen.clear();
          clearState = false;
        }
        break;
      }
    case ENTER:
      {
        bool solutionGenerated = false;
        if (currOperator == 1) {
          solution = inputNum1 + inputNum2;
          solutionGenerated = true;
        }
        if (currOperator == 2) {
          solution = inputNum1 - inputNum2;
          solutionGenerated = true;
        }
        if (currOperator == 3) {
          solution = inputNum1 * inputNum2;
          if (inputNum1 != 0 and solution / inputNum1 != inputNum2) {
            overflow = true;
          }
          solutionGenerated = true;
        }
        if (currOperator == 4) {
          solution = inputNum1 / inputNum2;
          solutionGenerated = true;
        }
        if (solutionGenerated) {
          mode = SOLUTION;
        }
        break;
      }
    case SOLUTION:  // alter solution
      {
        char key4 = keypad.getKey();
        calculations += 1;
        int count = countDigits(solution);
        if ((calculations % 10 == 0) or ((millis() / 1000) % 5 == 0)) {  // alter answer
          Serial.println("False Answer");
          // setup manipulation cases
          int alterCase = random(1, 4);
          Serial.println((String) "altered case:" + alterCase);
          switch (alterCase) {
            case 1:
              Serial.println((String) "1 unaltered solution:" + solution);
              solution += random(1, 11);
              break;
            case 2:
              Serial.println((String) "2 unaltered solution:" + solution);
              solution -= random(1, 11);
              break;
            case 3:
              Serial.println((String) "3 unaltered solution:" + solution);
              solution = switchRandomDigits(solution);
              break;
          }
        }
        if (overflow == false) {
          int startLocation = 20 - count;
          screen.setCursor(startLocation, 3);
          screen.print(solution);
          screen.setCursor(0, 3);
          screen.print("=");
        } else {
          screen.setCursor(0, 3);
          screen.print("Output Overflew.....");
        }
        // Serial.println((String) "calculations:" + calculations);
        Serial.println((String) "Current Calculation: " + calculations);
        Serial.println((String) "Current Time: " + millis());
        mode = DUNN;
        break;
      }
    case DUNN:
      {
        char key4 = keypad.getKey();
        if (key4 == '#') {
          mode = INPUT1;
          inputNum1 = 0;
          inputNum2 = 0;
          currOperator = 0;
          overflow = false;
          screen.clear();
        }
        break;
      }
  }
}