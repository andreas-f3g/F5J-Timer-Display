/* Shiftregister 74164 set
 * Character to bit sequence mapping
 *
 * 4 full digit display with separate colon driver
*/

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
#define SR_DIGIT_4_PIN 5 // front digit left
#define SR_COLON_PIN 10 //  ':'

unsigned char cmap[15]; // character map
#define HYPHEN_IN_cmap 10
#define COLON_IN_cmap 11
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
cmap[11]= 0xc0; // :  | * | * |   |   |   |   |   |   |    
cmap[12]= 0xc0; // 1: | * | * |   |   |   |   |   |   |
cmap[13]= 0x80; //  : | * |   |   |   |   |   |   |   |
cmap[14]= 0x40; //  : |   | * |   |   |   |   |   |   |

 
  // output pins
  pinMode(SR_RESET_PIN, OUTPUT);
  pinMode(SR_CLK_PIN, OUTPUT);
  pinMode(SR_DIGIT_1_PIN, OUTPUT);
  pinMode(SR_DIGIT_2_PIN, OUTPUT);
  pinMode(SR_DIGIT_3_PIN, OUTPUT);
  pinMode(SR_DIGIT_4_PIN, OUTPUT);
  pinMode(SR_COLON_PIN,   OUTPUT);

  reset(); // ready 

  play_intro(2);

  set_digits( cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[COLON_IN_cmap], SEG_OFF);

  Serial.println("start ...");

} // end setup

void reset(){
   // reset outputs
  digitalWrite(SR_DIGIT_1_PIN, LOW);
  digitalWrite(SR_DIGIT_2_PIN, LOW);
  digitalWrite(SR_DIGIT_3_PIN, LOW);
  digitalWrite(SR_DIGIT_4_PIN, LOW);
  digitalWrite(SR_COLON_PIN,   LOW);

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
  return ret;
}

void set_digits (unsigned char digit1, unsigned char digit2, unsigned char digit3, unsigned char digit4, unsigned char digit5) {
  reset(); // reset all
  for (int i=0; i<7; i++) { // set 8 segments in all 4 digits
    unsigned char mask=0x80>>i;
    digitalWrite(SR_DIGIT_1_PIN, ((digit1 & mask) > 0) ? HIGH : LOW);
    digitalWrite(SR_DIGIT_2_PIN, ((digit2 & mask) > 0) ? HIGH : LOW);
    digitalWrite(SR_DIGIT_3_PIN, ((digit3 & mask) > 0) ? HIGH : LOW);
    digitalWrite(SR_DIGIT_4_PIN, ((digit4 & mask) > 0) ? HIGH : LOW);
    digitalWrite(SR_COLON_PIN,   ((digit4 & mask) > 0) ? HIGH : LOW);
    clk();  // shift segment data
  }
}

void play_intro(int repeat)
{
  while ( repeat > 0) {
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_E + SEG_G, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_D + SEG_F, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_E + SEG_G, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_D + SEG_F, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_D + SEG_F, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_E + SEG_G, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_D + SEG_F, SEG_OFF, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_E + SEG_G, SEG_OFF, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_C, SEG_C, SEG_C, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_B, SEG_B, SEG_B, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_A, SEG_A, SEG_A, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_B, SEG_B, SEG_B, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_C, SEG_C, SEG_C, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_E + SEG_G, SEG_OFF, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_D + SEG_F, SEG_OFF, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_E + SEG_G, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_D + SEG_F, SEG_OFF, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_D + SEG_F, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_E + SEG_G, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_D + SEG_F, SEG_OFF, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_E + SEG_G, SEG_OFF);
    delay(INTRO_DELAY);
    set_digits ( SEG_OFF, SEG_OFF, SEG_OFF, SEG_OFF, SEG_OFF);
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
  switch (phase) {
    case 'P':  // prepare time
      set_digits (  char2digit(rec_char[10]), 
                    char2digit(rec_char[9]), 
                    char2digit(rec_char[8]),
                    char2digit(rec_char[7]), 
                    (rec_char[10] & 0x1) ?
                    cmap[LOWER_COLON_IN_cmap] :
                    cmap[UPPER_COLON_IN_cmap] );
      break;
    case 'W':  // working time
      set_digits (  char2digit(rec_char[10]), 
                    char2digit(rec_char[9]), 
                    char2digit(rec_char[8]), 
                    char2digit(rec_char[7]),
                    cmap[COLON_IN_cmap]);
      break;
    case 'S':  // timer stoped with round/group display
      set_digits (  char2digit(rec_char[5]), 
                    SEG_B, 
                    char2digit(rec_char[2]), 
                    SEG_OFF,
                    SEG_OFF);
      break;
    default:
      set_digits( cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[HYPHEN_IN_cmap], cmap[COLON_IN_cmap]);
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
 
      switch (rec_char[0]) {

        case 'Y':
        case 'y':
          // Y<number>
          // set all digits to the same character (i.e. "Y2")
          if (rec_index>=2) {
            set_digits ( char2digit(rec_char[1]), char2digit(rec_char[1]), char2digit(rec_char[1]), char2digit(rec_char[1]), cmap[COLON_IN_cmap]);
          }
          break;

        case 'R':
        case 'r':
          setDisplay(rec_char);
      }
      NL = false; 
      rec_index=0;
    }
  }

  // Bluetooth serial
  if (BTserial.available())
  {
    c = BTserial.read();
    if (c=='\r') { NL = true; rec_char[rec_index]=0x00; }
    else if ((rec_index<15) && (c!='\n')) { rec_char[rec_index++] = c; }
    if (NL) { // extended data gliderscore
      setDisplay(rec_char);
      NL = false; 
      rec_index=0;
    }    
  }
  
} // end loop
