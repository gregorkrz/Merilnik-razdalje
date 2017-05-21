#include <TM1637Display.h>
#include <QuickStats.h>
#include <EEPROM.h>
#include <NewPing.h>
QuickStats stats;
float meritve[10];
#define ZELEN 4
#define CRN 2
boolean addl = false;
boolean adva = false;
boolean expview = false;
double save[5] = {0, 0, 0, 0, 0}; //spremenljivka, ki hrani pomožne spremenljivke (vsote, produkte)
int airtemp = 20;
int enota = 0;
int slot = 0;
int eksponent = 0;
int stevec2 = 0;
#define CLK 8
#define DIO 10
#define TRIG 7
#define ECHO 6
NewPing sonar(TRIG, ECHO, 700);
TM1637Display display(CLK, DIO);
#define A 0b01110111
#define B 0b01111100
#define C 0b00111001
#define D 0b01011110
#define I 0b00000110
#define E 0b01111001
#define M 0b01010101
#define N 0b01010100
#define O 0b00111111
#define U 0b00111110
#define F 0b01110001
#define T 0b01111000
#define S 0b01101101
#define L 0b00111000
#define R 0b01010000
#define K 0b01110000
#define P 0b01110011
#define V 0b00011100
#define MINUS 0b01000000
uint8_t data[] = {0x0, 0x0, 0x0, 0x0}; //spremenljivka, ki skrbi za prikaz na zaslonu
int korak = 1;
float meritev = 0;
boolean sestevanje = true;
int stevec = 0;
float c = 0;
void setup() {
  Serial.begin(115200);
  display.setBrightness(0x0f);
  pinMode(ZELEN, INPUT_PULLUP);
  pinMode(CRN, INPUT_PULLUP);
  EEPROM.get(400, save); //pridobi meritve in nastavitve iz EEPROM-a
  if (EEPROM.read(1) == 255) {
    EEPROM.write(1, 20);
  }
  if (EEPROM.read(2) == 255) EEPROM.write(2, 0);
  if (EEPROM.read(2) == 0) addl = false;
  else addl = true;
  if (EEPROM.read(3) == 255) EEPROM.write(3, 0);
  if (EEPROM.read(3) == 0) adva = false;
  else adva = true;
  if (EEPROM.read(10) == 255) EEPROM.write(10, 0);
  enota =  EEPROM.read(10) - 10;
  airtemp = EEPROM.read(1);
}

void loop() {

  stevec++;
  delay(10);
  //posodabljanje zaslona (na 50 ms)
  if (stevec == 5) {
    stevec = 0;
    if (korak == 1) prikazi(D, I, S, T);
    else if (korak == 2) prikazi(S, E, T, T);
    else if (korak == 3) prikazi(U, S, B, 0x0);
    else if (korak == 12)prikazi(A, D, D, 0x0);
    else if (korak == 121 or korak == 131 or korak == 141) prikazi(S, L, T, display.encodeDigit(slot));
    else if (korak == 13) prikazi(M, U, L, T);
    else if (korak == 14) prikazi (L, O, A, D);
    else if (korak == 15) prikazi(M, E, M, 0x0);
    else if (korak == 16) prikazi(B, A, C, K);
    else if (korak == 21) prikazi(T, E, M, P);
    else if (korak == 22) prikazi(A, D, D, L);
    else if (korak == 23) prikazi(A, D, V, A);
    else if (korak == 11) {
      stevec2++;
      if (stevec2 >= 13) {
        stevec2 = 0;
        expview = !expview;
      }
      if (expview == true) { //prikaz vrednosti meritve
        display.showNumberDec(meritev);
      }
      //prikaz eksponenta
      else if (eksponent > 9) prikazi(E, 0x0, display.encodeDigit(String(eksponent).substring(0, 1).toInt()), display.encodeDigit(String(eksponent).substring(1).toInt()));
      else if (eksponent < 0) prikazi(E, 0x0, MINUS, display.encodeDigit(-1 * eksponent));
      else  prikazi(E, 0x0, 0x0, display.encodeDigit(eksponent));
    }
    else if (korak == 24) prikazi(U, N, I, T);
    else if (korak == 25) prikazi(B, A, C, K);
    else if (korak == 210) display.showNumberDec(airtemp);
    else if (korak == 241) prikazi(0x0, M, M, 0x0);
    else if (korak == 242) prikazi(0x0, C, M, 0x0);
    else if (korak == 243) prikazi(0x0, M, 0x0, 0x0);
    else if (korak == 31) {
      c = 0.02005 * sqrt(273 + airtemp); //bolj bi bilo učinkovito, da bi to izvedel brez preračunavanja, vendar je tako preprosteje
      while (digitalRead(ZELEN) == HIGH) {
        Serial.println(izmerek()*pow(10, enota));
        delay(1);
      }
      korak = 1;
      delay(100);
    }
  }
  // Pritisnjeni gumbi??
  if (digitalRead(ZELEN) == LOW) { //zelen gumb pritisnjen?
    while (digitalRead(ZELEN) == LOW) {
      delay(60);
    }

    if (korak == 1) {
      korak = 11;
      Serial.println("----------");
      for (byte x = 0; x < 5; x++) {
        Serial.print("slot ");
        Serial.print(x + 1);
        Serial.print(": ");
        Serial.println(save[x]);
      }
      Serial.println("----------");
    }
    else if (korak == 2) korak = 21;
    else if (korak == 16) korak = 1;
    else if (korak == 3)   korak = 31;
    else if (korak == 141) {
      float temporary = save[slot - 1];
      int temp2 = 0;
      if (temporary<10000 and temporary >= 1000) {       //štirimestna številka?
        eksponent = 0;
        meritev = temporary;
        korak = 11;
        expview = false;
      }
      else if (temporary > 10000) { //več kot 4-mestna?
        while (temporary > 10000) {
          temp2++;
          temporary = temporary / 10;
        }

        eksponent = temp2;
        meritev = temporary;
        korak = 11;
        expview = false;
      }
      else if (temporary < 1000) {
        while (temporary < 1000) {
          temp2++;
          temporary = temporary * 10;
        }
        eksponent = -1 * temp2;
        meritev = temporary;
        expview = false;
        korak = 11;
      }
    }
    else if (korak == 11) {
      eksponent = enota;
      meritev = razdalja();
      expview = true;
      stevec2 = 0;
    }
    else if (korak == 12) {
      korak = 121;
      slot = 1;
    }
    else if (korak == 15) {
      for (byte x = 0; x < 5; x++) {
        save[x] = 0;
      }
      EEPROM.put(400, save);
      delay(50);
      korak = 1;
    }
    else if (korak == 131) {
      save[slot - 1] = save[slot - 1] * (meritev * pow(10, eksponent));
      korak = 1;
      EEPROM.put(400, save);
    }
    else if (korak == 121) {
      save[slot - 1] = save[slot - 1] + (meritev * pow(10, eksponent));
      korak = 1;
      EEPROM.put(400, save);

    }
    else if (korak == 14) {
      korak = 141;
      slot = 1;
    }
    else if (korak == 25) korak = 2;
    else if (korak == 13) {
      korak = 131;
      slot = 1;
    }
    else if (korak == 21) {
      airtemp = EEPROM.read(1);
      korak = 210;
    }
    else if (korak == 22) {
      if (addl == true) {
        addl = false;
        EEPROM.write(2, 0);
        prikazi(0x0, O, F, F);
        delay(500);
      }
      else {
        addl = true;
        EEPROM.write(2, 1);
        prikazi(0x0, O, N, 0x0);
        delay(500);
      }
    }
    else if (korak == 23) {
      if (adva == true) {
        adva = false;
        EEPROM.write(3, 0);
        prikazi(0x0, O, F, F);
        delay(500);
      }
      else {
        adva = true;
        EEPROM.write(3, 1);
        prikazi(0x0, O, N, 0x0);
        delay(500);
      }
    }
    else if (korak == 24) {
      korak = 241;
      enota = 0;
    }
    else if (korak == 210) {
      EEPROM.write(1, airtemp);
      korak = 21;
    }
    else if (korak == 241) {
      enota = 0;
      EEPROM.write(10, 10 + enota);
      korak = 24;
    }
    else if (korak == 242) {
      enota = -1;
      EEPROM.write(10, 10 + enota);

      korak = 24;
    }
    else if (korak == 243) {
      enota = -3;
      EEPROM.write(10, 10 + enota);

      korak = 24;
    }
  }
  else if (digitalRead(CRN) == LOW) {   //črn gumb pritisnjen?
    while (digitalRead(CRN) == LOW) delay(60);
    if ( korak == 1) korak = 2;
    else if ( korak == 2) korak = 3;
    else if (korak == 3) korak = 1;
    else if (korak == 4) korak = 1;
    else if (korak == 11) korak = 12;
    else if (korak == 131 or korak == 141 or korak == 121) {
      if (slot == 5) slot = 1;
      else slot++;
    }
    else if (korak == 12) korak = 13;
    else if (korak == 13) korak = 14;
    else if (korak == 14) korak = 15;
    else if (korak == 15) korak = 16;
    else if (korak == 16) korak = 12;
    else if (korak == 21) korak = 22;
    else if (korak == 22) korak = 23;
    else if (korak == 23) korak = 24;
    else if (korak == 24) korak = 25;
    else if (korak == 25) korak = 21;
    else if (korak == 210) {
      if (airtemp == 30) {
        airtemp = 0;
      }
      else {
        airtemp = airtemp + 5;
      }
    }
    else if (korak == 241) korak = 242;
    else if (korak == 242) korak = 243;
    else if (korak == 243) korak = 241;
  }
}



/
void prikazi(uint8_t prvi, uint8_t drugi, uint8_t tretji, uint8_t cetrti) { //prikazi segmente na zaslonu
  data[0] = prvi;
  data[1] = drugi;
  data[2] = tretji;
  data[3] = cetrti;
  display.setSegments(data);

}
int izmerek() { //izvedi meritev in vrni čas v mikrosekundah
  float uS = sonar.ping();
  uS = (uS / 2) * c;
  if (addl == true and uS != 0) {
    uS = uS + 96;
  }
  delay(2);
  return round(uS);
}
int razdalja() {
  c = 0.02005 * sqrt(273 + airtemp);
  if (adva == true) {
    for (byte y = 0; y < 10; y++) {
      meritve[y] = izmerek();
      delay(5);
    }
    stats.bubbleSort(meritve, 10);
    if (stats.maximum(meritve, 10) < 140) {
      return (stats.average(meritve, 10));
    }
    else {
      if (((meritve[7] + meritve[8] + meritve[9]) / 3) / stats.maximum(meritve, 10) >= 0.97) {
        return (meritve[7] + meritve[8] + meritve[9]) / 3;
      }
      else if ((((meritve[8] + meritve[9]) / 2) / stats.maximum(meritve, 10)) >= 0.97) {
        return ((meritve[8] + meritve[9]) / 2);
      }
      else {
        return stats.maximum(meritve, 10);
      }
    }

  }
  else return izmerek();
}
