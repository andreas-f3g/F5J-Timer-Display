/* Shiftregister 74164 set
 * Character to bit sequence mapping
 *
 * Utils to test hardware
 *
*/

// debug utils
#define DBG_ON
#ifdef DBG_ON
#define DBG_PRINT_NL  Serial.println();
#define DBG_PRINT(text)  Serial.print(text);
#define DBG_PRINT_HEX(text)  Serial.print(text,HEX);
#else
#define DBG_PRINT_NL  ;
#define DBG_PRINT(text) ;
#define DBG_PRINT_HEX(text) ;
#endif

#include <SoftwareSerial.h>

// serial protocol
boolean NL = false;
char rec_char[15];
int rec_index=0;
SoftwareSerial BTserial(8, 9); // RX, TX 4 BT

// arduino output definition
#define SR_CLK_PIN 7
#define SR_RESET_PIN 6
#define SR_DIGIT_1_PIN 2 // right most digit
#define SR_DIGIT_2_PIN 3
#define SR_DIGIT_3_PIN 4
#define SR_DIGIT_4_PIN 5 // front digit left, only '1' and ':'

unsigned char cmap[15]; // character map
#define HYPHEN_IN_cmap 10
#define COLLON_IN_cmap 11
#define ONE_AND_COLON_IN_cmap 12
#define LOWER_COLON_IN_cmap 13
#define UPPER_COLON_IN_cmap 14

// times
#define INTRO_DELAY 250

 // segment bits
#define SEG_A 0x80
#define SEG_B 0x40
#define SEG_C 0x20
#define SEG_D 0x10
#define SEG_E 0x08
#define SEG_F 0x04
#define SEG_G 0x02
#define SEG_OFF 0

// globals
int   plop_step=0;
char  phase='S';

void setup() {

  // USB serial
  Serial.begin(9600);

  // BT serial
  BTserial.begin(9600);

  // character to bit sequence mapping of 74164 output pins
/* segment numbering
         --A-- 
       D|     |E
         --B-- 
       F|     |G
         --C-- 
*/
              // char | A | B | C | D | E | F | G | : |
cmap[0] = 0xbf; // 0  | * |   | * | * | * | * | * | * |
cmap[1] = 0x0b; // 1  |   |   |   |   | * |   | * | * |
cmap[2] = 0xed; // 2  | * | * | * |   | * | * |   | * |
cmap[3] = 0xeb; // 3  | * | * | * |   | * |   | * | * |
cmap[4] = 0x5b; // 4  |   | * |   | * | * |   | * | * |
cmap[5] = 0xf3; // 5  | * | * | * | * |   |   | * | * |
cmap[6] = 0x77; // 6  |   | * | * | * |   | * | * | * |
cmap[7] = 0x8b; // 7  | * |   |   |   | * |   | * | * |
cmap[8] = 0xff; // 8  | * | * | * | * | * | * | * | * |
cmap[9] = 0xdb; // 9  | * | * |   | * | * |   | * | * |
cmap[10]= 0x40; // -  |   | * |   |   |   |   |   |   |
cmap[11]= 0x14; // :  |   |   |   | * |   | * |   |   |    
cmap[12]= 0x1e; // 1: |   |   |   | * | * | * | * |   |
cmap[13]= 0x10; //  : |   |   |   | * |   |   |   |   |
cmap[14]= 0x04; //  : |   |   |   |   |   | * |   |   |

 
  // output pins
  pinMode(SR_RESET_PIN, OUTPUT);
  pinMode(SR_CLK_PIN, OUTPUT);
  pinMode(SR_DIGIT_1_PIN, OUTPUT);
  pinMode(SR_DIGIT_2_PIN, OUTPUT);
  pinMode(SR_DIGIT_3_PIN, OUTPUT);
  pinMode(SR_DIGIT_4_PIN, OUTPUT);

  reset(); // ready 

  play_intro(2);

  set_digits( cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[COLLON_IN_cmap]);

  Serial.println("start ...");

} // end setup

void reset(){
   // reset outputs
  digitalWrite(SR_DIGIT_1_PIN, LOW);
  digitalWrite(SR_DIGIT_2_PIN, LOW);
  digitalWrite(SR_DIGIT_3_PIN, LOW);
  digitalWrite(SR_DIGIT_4_PIN, LOW);

  digitalWrite(SR_RESET_PIN, LOW); // first reset all
  digitalWrite(SR_RESET_PIN, HIGH); // ready 
}

void clk() {
  digitalWrite(SR_CLK_PIN, LOW);
  digitalWrite(SR_CLK_PIN, HIGH);  // shift segment data
  digitalWrite(SR_CLK_PIN, LOW);
}

unsigned char char2digit ( char c) {
  unsigned char ret=cmap[HYPHEN_IN_cmap];
  if ((c >= '0')&&(c<='9')) ret = cmap[c-'0'];
  DBG_PRINT("char2digit (")
  DBG_PRINT(c)
  DBG_PRINT(") -> 0x")
  DBG_PRINT_HEX(ret)
  DBG_PRINT_NL
  return ret;
}

void set_digits (unsigned char digit1, unsigned char digit2, unsigned char digit3, unsigned char digit4) {
  reset(); // reset all
  for (int i=0; i<7; i++) { // set 8 segments in all 4 digits
    unsigned char mask=0x80>>i;
    digitalWrite(SR_DIGIT_1_PIN, ((digit1 & mask) > 0) ? HIGH : LOW);
    digitalWrite(SR_DIGIT_2_PIN, ((digit2 & mask) > 0) ? HIGH : LOW);
    digitalWrite(SR_DIGIT_3_PIN, ((digit3 & mask) > 0) ? HIGH : LOW);
    digitalWrite(SR_DIGIT_4_PIN, ((digit4 & mask) > 0) ? HIGH : LOW);
    clk();  // shift segment data
  }
}

void set_segment (unsigned char digit, unsigned char segment) {
  reset(); // reset all

  int digitPin; // map digit number
  switch (digit) {
    case '1': digitPin=SR_DIGIT_1_PIN; break;
    case '2': digitPin=SR_DIGIT_2_PIN; break;
    case '3': digitPin=SR_DIGIT_3_PIN; break;
    case '4': digitPin=SR_DIGIT_4_PIN; break;
    default: digitPin=SR_DIGIT_1_PIN;
  }

  unsigned char seg; // map segment id
  switch (segment) {
    case 'A':
    case 'a': seg=0x80; break;
    case 'B':
    case 'b': seg=0x40; break;
    case 'C':
    case 'c': seg=0x20; break;
    case 'D':
    case 'd': seg=0x10; break;
    case 'E':
    case 'e': seg=0x08; break;
    case 'F':
    case 'f': seg=0x04; break;
    case 'G':
    case 'g': seg=0x02; break;
    case ':': seg=0x01; break;
    default:  seg=0x00;
  }

 
  for (int i=0; i<7; i++) { // set 8 segments in digit
    unsigned char mask=0x80>>i;
    digitalWrite(digitPin, ((seg & mask) > 0) ? HIGH : LOW);
    DBG_PRINT("digit: ")
    DBG_PRINT(digitPin - 1)
    DBG_PRINT(" seg mask: -> ")
    DBG_PRINT_HEX(mask)
    DBG_PRINT(" ")
    DBG_PRINT(((seg & mask) > 0) ? HIGH : LOW)
    DBG_PRINT_NL
    clk();  // shift segment data
  }
} // end set_segment

void play_plop(int repeat)
{
  while ( repeat > 0) {
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_D + SEG_F);
    delay(120);
    set_digits ( SEG_OFF, SEG_D + SEG_F + SEG_B, SEG_E + SEG_G + SEG_B, SEG_OFF);
    delay(120);
     set_digits ( SEG_OFF, SEG_A + SEG_C + SEG_E + SEG_G, SEG_A + SEG_C + SEG_D + SEG_F, SEG_OFF);
    delay(120);
     set_digits ( SEG_D + SEG_F, SEG_OFF, SEG_OFF, SEG_E + SEG_G);
    delay(120);
     set_digits ( SEG_OFF, SEG_A + SEG_C + SEG_E + SEG_G, SEG_A + SEG_C + SEG_D + SEG_F, SEG_OFF);
    delay(120);
    set_digits ( SEG_OFF, SEG_D + SEG_F + SEG_B, SEG_E + SEG_G + SEG_B, SEG_OFF);
     delay(120);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_D + SEG_F);
     repeat--;
  }
}

void step_plop()
{
  switch (plop_step) {
    case 1:
      set_digits ( SEG_OFF, SEG_D + SEG_F + SEG_B, SEG_E + SEG_G + SEG_B, SEG_OFF);
      break;
    case 2:
      set_digits ( SEG_OFF, SEG_A + SEG_C + SEG_E + SEG_G, SEG_A + SEG_C + SEG_D + SEG_F, SEG_OFF);
      break;
    case 3:
      set_digits ( SEG_D + SEG_F, SEG_OFF, SEG_OFF, SEG_E + SEG_G);
      break;
    case 4:
      set_digits ( SEG_OFF, SEG_A + SEG_C + SEG_E + SEG_G, SEG_A + SEG_C + SEG_D + SEG_F, SEG_OFF);
      break;
    case 5:
      set_digits ( SEG_OFF, SEG_D + SEG_F + SEG_B, SEG_E + SEG_G + SEG_B, SEG_OFF);
    default:
      set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_D + SEG_F);
      plop_step = 0;
  }
  plop_step++;
}

void play_intro(int repeat)
{
  while ( repeat > 0) {
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_E + SEG_G);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_D + SEG_F, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_E + SEG_G, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_D + SEG_F);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_D + SEG_F, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_E + SEG_G, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_D + SEG_F, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_E + SEG_G, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_C, SEG_C, SEG_C, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_B, SEG_B, SEG_B, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_A, SEG_A, SEG_A, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_B, SEG_B, SEG_B, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_C, SEG_C, SEG_C, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_E + SEG_G, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_D + SEG_F, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_E + SEG_G, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_D + SEG_F, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_D + SEG_F);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_E + SEG_G, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_D + SEG_F, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_E + SEG_G);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
     repeat--;
  }
}

void setDisplay(unsigned char * rec_char)
{
  // parse time and phase 'R??G??T?????T
  if (  (rec_index >= 13)
      && (rec_char[0]=='R')
      && (rec_char[3]=='G')
      && (rec_char[6]=='T')
      && (rec_char[12]=='T')
  ) {
  phase = rec_char[11];
  DBG_PRINT(" phase ")
  DBG_PRINT(phase)
  DBG_PRINT(" - ")
  switch (phase) {
    case 'P':  // prepare time
      set_digits (  char2digit(rec_char[10]), 
                    char2digit(rec_char[9]), 
                    char2digit(rec_char[8]), 
                    ((rec_char[7]=='1')?char2digit('1'):0) + ((rec_char[10] & 0x1) ?
                    cmap[LOWER_COLON_IN_cmap] :
                    cmap[UPPER_COLON_IN_cmap] ) );
      break;
    case 'W':  // working time
      set_digits (  char2digit(rec_char[10]), 
                    char2digit(rec_char[9]), 
                    char2digit(rec_char[8]), 
                    ((rec_char[7]=='1')?char2digit('1'):0) + cmap[COLLON_IN_cmap]);
      break;
    case 'S':
      set_digits (  char2digit(rec_char[5]), 
                    SEG_B, 
                    char2digit(rec_char[2]), 
                    0);
      break;
    default:
      set_digits( cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[COLLON_IN_cmap]);
  } }
}

void loop() {
  unsigned char c=' ';
 
  // standard USB serial
  if (Serial.available())
  {  
    c = Serial.read();
    if (c=='\r') { NL = true; rec_char[rec_index]=0x00; }
    else if ((rec_index<15) && (c!='\n')) { rec_char[rec_index++] = c; }
    if (NL) { 
      DBG_PRINT(">");
      DBG_PRINT(rec_char); 
      DBG_PRINT(": ");
      
      switch (rec_char[0]) {

        case 'C':
        case 'c':
          // test clock signal
          // C<digit number>
          // (i.e. "C3")
            int digitPin; // map digit number
            switch (rec_char[1]) {
              case '1': digitPin=SR_DIGIT_1_PIN; break;
              case '2': digitPin=SR_DIGIT_2_PIN; break;
              case '3': digitPin=SR_DIGIT_3_PIN; break;
              case '4': digitPin=SR_DIGIT_4_PIN; break;
              default: digitPin=SR_DIGIT_1_PIN;
            }
            DBG_PRINT(" test clock (arduino PIN=")
            DBG_PRINT(digitPin)
            DBG_PRINT(")")
            while (true) {
              digitalWrite(digitPin, HIGH);
              clk();
              clk();
              digitalWrite(digitPin, LOW);
              clk();
            }
          break;

        case 'P':
        case 'p':
          // P<count>
          // play animation
          if (rec_index>1) {
            play_plop(rec_char[1]-'0');
          }
          break;

        case 'I':
        case 'i':
          // P<count>
          // play intro
          if (rec_index>1) {
            play_intro(rec_char[1]-'0');
          }
          break;

        case 'X':
        case 'x':
          // X<digit number><segment number>
          // set single segment (i.e. "X3E")
          if (rec_index>=3) {
            DBG_PRINT(" set digit ")
            DBG_PRINT(rec_char[1])
            DBG_PRINT(" segment ")
            DBG_PRINT(rec_char[2])
            DBG_PRINT_NL
            set_segment(rec_char[1], rec_char[2]); 
          }
          break;

        case 'Y':
        case 'y':
          // Y<number/character>
          // set all digits to the same character (i.e. "Y2")
          if (rec_index>=2) {
            DBG_PRINT(" set digits to ")
            DBG_PRINT(rec_char[1])
            DBG_PRINT_NL
            set_digits ( char2digit(rec_char[1]), char2digit(rec_char[1]), char2digit(rec_char[1]), cmap[ONE_AND_COLON_IN_cmap]);
          }
          break;

        case 'R':
        case 'r':
          setDisplay(rec_char);
      }
      DBG_PRINT_NL
      NL = false; 
      rec_index=0;
    }
  }

  // Bluetooth serial
  if (BTserial.available())
  {
    c = BTserial.read();
    DBG_PRINT(c)
    if (c=='\r') { NL = true; rec_char[rec_index]=0x00; }
    else if ((rec_index<15) && (c!='\n')) { rec_char[rec_index++] = c; }
    if (NL) { // extended data gliderscore
      DBG_PRINT_NL
      DBG_PRINT(">")
      DBG_PRINT(rec_char)
      DBG_PRINT(": ")
      setDisplay(rec_char);
      DBG_PRINT_NL
      NL = false; 
      rec_index=0;
    }    
  }
  
} // end loop
