#include <MD_MAX72xx.h>

#define BUF_SIZE 20
#define MAX_DEVICES 6
#define CHAR_SIZE 6
#define MAX_CHARS_ON_SCREEN 6

#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

uint8_t charachters[37][5] = 
{  {0,0,0,0,0}              // ETC
  ,{126, 9, 9, 9, 126}      // A
  ,{127, 73, 73, 73, 54}    // B
  ,{62, 65, 65, 65, 65}     // C
  ,{127, 65, 65, 65, 62}    // D
  ,{ 127, 73, 73, 73, 65}   // E
  ,{127, 9, 9, 9, 1}        // F
  ,{62, 65, 65, 73, 121}    // G
  ,{127, 8, 8, 8, 127}      // H
  ,{0, 65, 127, 65, 0}      // I 
  ,{48, 65, 65, 65, 63}     // J
  ,{127, 8, 20, 34, 65}     // K
  ,{127, 64, 64, 64, 64}    // L
  ,{127, 2, 12, 2, 127}     // M
  ,{127, 4, 8, 16, 127}     // N
  ,{62, 65, 65, 65, 62}     // O
  ,{127, 9, 9, 9, 6}        // P
  ,{62, 65, 65, 97, 126}    // Q
  ,{127, 9, 25, 41, 70}     // R
  ,{70, 73, 73, 73, 49}     // S
  ,{1, 1, 127, 1, 1}        // T
  ,{63, 64, 64, 64, 63}     // U
  ,{31, 32, 64, 32, 31}     // V
  ,{63, 64, 56, 64, 63}     // W
  ,{99, 20, 8, 20, 99}      // X
  ,{3, 4, 120, 4, 3}        // Y
  ,{97, 81, 73, 69, 67}     // Z
  ,{62, 81, 73, 69, 62,}    // 0
  ,{0, 4, 2, 127, 0}        // 1
  ,{113, 73, 73, 73, 70}    // 2
  ,{65, 73, 73, 73, 54}     // 3
  ,{15, 8, 8, 8, 127}       // 4
  ,{79, 73, 73, 73, 49}     // 5
  ,{62, 73, 73, 73, 48}     // 6
  ,{3, 1, 1, 1, 127}        // 7
  ,{54, 73, 73, 73, 54}     // 8
  ,{6, 73, 73, 73, 62}      // 9
};

class Effects {
public:
  Effects(MD_MAX72XX *mx): display(mx) {}


void open() {

  for (int i = 8; i > 0; i--) {
    display->setRow(i, 0xff);
    delay(300);
  }

  for (int i = 1; i < 7; i++) {
    display->setRow(i, 0);
    delay(300);
  }
}

void close() {
  for (int i = 3; i >= 0; i--) {
    delay(300);
    
    display->setRow(i, 0x00);
    display->setRow(7 - i, 0x00);
  }
  
  delay(300);
}

 void reverseData() {
    uint8_t buffer;
    for(int row = 0 ; row < 4 ; row++) {
        for(int col = 0 ; col < 8 ; col++) {
            buffer = display->getColumn(row, col);
            display->setColumn(row * 8 + col, ~buffer);
        }
    }
}

private:
  MD_MAX72XX *display = 0 ;
};

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::PAROLA_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
Effects effect = Effects(&mx);

void printCharacter(uint8_t column, uint8_t character)
{
  uint8_t mappedCharacter;
  
  if(character >= 65 && character <= 90)
    mappedCharacter = character - 64;
  else if(character >= 97 && character <= 122)
    mappedCharacter = character - 96;
  else if(character >= 48 && character <= 57 )
    mappedCharacter = character - 21;
  else
    mappedCharacter = 0;

  mx.setBuffer(column, 5, charachters[mappedCharacter]);
}
void blinkText(const char *string, uint8_t length) {
  static char curr_msg[BUF_SIZE];
  int mid_col = (length % 2) ? 18 : 15;
  int mid_char_indx = length / 2;
  int i, right_char_col, left_char_col;

  if (length <= 0)
    return;

  strcpy(curr_msg, string);


  for (i = 0; i < (length / 2) + 1; i++) {
    right_char_col = mid_col - i * CHAR_SIZE;
    left_char_col = mid_col + i * CHAR_SIZE;

    if (mid_char_indx - i >= 0 && left_char_col <= 32 + CHAR_SIZE) {
     
      printCharacter(left_char_col, curr_msg[mid_char_indx - i]);
      delay(500);
     
      printCharacter(left_char_col, ' ');
    }

    if (mid_char_indx + i < length && i != 0 && right_char_col >= 0) {
      // Blink ON
      printCharacter(right_char_col, curr_msg[mid_char_indx + i]);
      delay(500);
      // Blink OFF
      printCharacter(right_char_col, ' ');
    }
  }
  for(i = 0 ; i < (length/2)+1 ; i++){
    right_char_col = mid_col - i*CHAR_SIZE;
    left_char_col = mid_col + i*CHAR_SIZE;
    if(mid_char_indx-i >= 0 && left_char_col <= 32 + CHAR_SIZE) // change to macro
      printCharacter(left_char_col, curr_msg[mid_char_indx-i]);
    if(mid_char_indx-+i < length && i!=0 && right_char_col >= 0)
      printCharacter(right_char_col,curr_msg[mid_char_indx+i]);
  }
}

void displayMovingText(const char *text, uint8_t length) {
  int index, charIndex;
  int startColumn = 0, currentColumn = 0;
  static char currentMessage[BUF_SIZE];
  uint32_t animationLength;

  if(length <= 0)
    return;

  strcpy(currentMessage, text);

  animationLength = (length + MAX_CHARS_ON_SCREEN) * CHAR_SIZE;

  for(index = 0 ; index < animationLength ; index++) {
    int numOfChars = (int)(index / CHAR_SIZE) + 1 ;
    numOfChars = (numOfChars > length) ? length : numOfChars ;
    startColumn = currentColumn;
    int minCharIndex = ((numOfChars < MAX_CHARS_ON_SCREEN) ? 0 : numOfChars - MAX_CHARS_ON_SCREEN);
    mx.clear();
    for(charIndex = numOfChars - 1; charIndex >= minCharIndex ; charIndex--) {
      printCharacter(startColumn, (uint8_t)currentMessage[charIndex]);
      startColumn += 6;
    }
    currentColumn += 1;
    if(currentColumn >= CHAR_SIZE && numOfChars != length)
      currentColumn = 0;
    delay(50);
  }

}

void show() {
  effect.open();
  displayMovingText("MoeinAhmadieh", 13);
  blinkText("Micro",5);
  delay(1000);
  effect.close();
}

void setup() {
  mx.begin();
}

void loop() {
  show();
}
