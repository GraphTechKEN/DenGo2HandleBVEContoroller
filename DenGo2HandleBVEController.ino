// SPI test Arduino: Master, DUALSHOCK: Slave // // Copyright (c) 2015 Kazumasa ISE // Released under the MIT license // http://opensource.org/licenses/mit-license.php

//簡単な説明
//PS用電車でGOコントローラーをBVEおよびJRETSで使用するスケッチです
//Arduino Leonard および Micro 用 です
//TS185用に、13番ピンをモード切替用に使用、適宜スイッチやジャンパを設置すること(GNDとショートで通常、オープンで185系モード)
// Copyright (c) 2025 GraphTechKEN // Released under the MIT license  // http://opensource.org/licenses/mit-license.php 

//キーアサイン
//A:↑
//B:Return
//C:↓
//Start:Return (185モード:BackSpace)
//Select:+

//PADピンアサイン(本体側に向かって右から)
//1:MISO ※5ピンの+5Vと2kΩ程度の抵抗で接続してプルアップする
//2:MOSI
//3:(NC)
//4:GND
//5:+5V
//6:SS
//7:SCK
//8:(NC)
//9:(NC)

#include <Keyboard.h>
#include <SPI.h>

#define TRANSFER_WAIT 16
#define FRAME_WAIT 16
#define COUNT 3

int notch_mc = 0;
int notch_brk = 0;
int model = 0;
int count = 0;
int ispeed = 0;
int notch_mc_latch = 0;
int notch_brk_latch = 0;
int EBcount = 0;
int Ncount = 0;
bool flgconnect = false;

//ボタン設定
bool btnSelect = false;
bool btnStart = false;
bool btnA = false;
bool btnB = false;
bool btnC = false;
bool btnD = false;

//TS185モード
bool mode185 = false;

//自動帯制御
uint8_t notch_AAB = 0;
bool flgEB = false;
bool flgBRK = false;

//SPI設定
SPISettings spiSettings = SPISettings(250000, LSBFIRST, SPI_MODE3);

/////以下DualShock用/////
//コンフィグレーションモード用コマンド
const byte CMD_config[] = { 0x01, 0x43, 0x00, 0x01, 0x00 };
const byte CMD_config_BYTES = sizeof CMD_config;
byte DAT_config[CMD_config_BYTES] = { 0 };

//コンフィグレーションイグジット用コマンド
const byte CMD_config_exit[] = { 0x01, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const byte CMD_config_exit_BYTES = sizeof CMD_config_exit;
byte DAT_config_exit[CMD_config_exit_BYTES] = { 0 };

//セットアンドロック用コマンド
const byte CMD_set_lock[] = { 0x01, 0x44, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00 };
const byte CMD_set_lock_BYTES = sizeof CMD_set_lock;
byte DAT_set_lock[CMD_set_lock_BYTES] = { 0 };

//コントローラーデータ読込用コマンド
const byte CMD_read[] = { 0x01, 0x42, 0x00, 0x00, 0x00 };
const byte CMD_read_BYTES = sizeof CMD_read;
byte DAT_SPI_read[CMD_read_BYTES] = { 0 };

//コントローラーモデル読込用コマンド
const byte CMD_model[] = { 0x01, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const byte CMD_model_BYTES = sizeof CMD_model;
byte DAT_model[CMD_model_BYTES] = { 0 };
/////以上DualShock用/////

void setup() {
  pinMode(13, INPUT_PULLUP);  //185系モード:true
  SPI.begin();
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);
  pinMode(SS, OUTPUT);
  Keyboard.begin();
}

void loop() {
  data_read();
  if (DAT_SPI_read[1] != 0xFF) {
    config_mode();
    model = model_check();
    set_lock();
    config_mode_exit();
    flgconnect = true;
  }
  while (flgconnect) {
    mode185 = digitalRead(13);
    data_read();
    if (DAT_SPI_read[2] == 0xFF) {  //コントローラーが抜かれたとき、モデルと接続フラグをリセット
      model = 0;
      flgconnect = false;
    }
    key_analysys();
    keyboard_control();
    button_Control();
    notch_mc_latch = notch_mc;
    notch_brk_latch = notch_brk;
  }
}

void key_analysys() {
  if (model == 0x41 && DAT_SPI_read[3] != 0x00) {
    switch (DAT_SPI_read[4] & 0x0F) {
      case 0x00:
        //非常に入るバグがあるのでカウントで回避
        if (EBcount > 3) {
          notch_brk = 0;
          notch_AAB = 3;
          EBcount = 0;
          flgEB = true;
        } else {
          EBcount++;
        }
        Ncount = 0;
        break;
      case 0x09:
        notch_brk = 1;
        notch_AAB = 0;
        EBcount = 0;
        Ncount = 0;
        break;
      case 0x0D:
        notch_brk = 2;
        EBcount = 0;
        Ncount = 0;
        break;
      case 0x02:
        notch_brk = 3;
        EBcount = 0;
        Ncount = 0;
        break;
      case 0x06:
        notch_brk = 4;
        EBcount = 0;
        Ncount = 0;
        break;
      case 0x03:
        notch_brk = 5;
        EBcount = 0;
        Ncount = 0;
        break;
      case 0x07:
        notch_brk = 6;
        EBcount = 0;
        Ncount = 0;
        break;
      case 0x0A:
        notch_brk = 7;
        EBcount = 0;
        Ncount = 0;
        break;
      case 0x0E:
        notch_brk = 8;
        EBcount = 0;
        Ncount = 0;
        break;
      case 0x0B:
        //Nに入るバグがあるのでカウントで回避
        if (Ncount > 3) {
          notch_brk = 9;
          Ncount = 0;
        } else {
          Ncount++;
        }
        EBcount = 0;
        break;

      case 0x0C:
      case 0x08:
        notch_AAB = 1;
        break;

      case 0x05:
      case 0x01:
      case 0x04:
        notch_AAB = 2;
        break;
    }

    if ((DAT_SPI_read[4] & 0x10) == 0x10) {
      switch (DAT_SPI_read[3] & 0xF0) {
        case 0x70:
          notch_mc = 10;
          break;
        case 0xD0:
          notch_mc = 12;
          break;
        case 0x50:
          notch_mc = 14;
          break;
      }
    } else {
      switch (DAT_SPI_read[3] & 0xF0) {
        case 0xF0:
          notch_mc = 9;
          break;
        case 0x70:
          notch_mc = 11;
          break;
        case 0xD0:
          notch_mc = 13;
          break;
      }
    }
    //  }
  }
}

int config_mode() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(SS, LOW);
  delayMicroseconds(TRANSFER_WAIT);
  for (byte i = 0; i < CMD_config_BYTES; i++) {
    DAT_config[i] = SPI.transfer(CMD_config[i]);  //SPI送受信，受信データビット反転
    delayMicroseconds(TRANSFER_WAIT);
  }
  digitalWrite(SS, HIGH);
  delay(FRAME_WAIT);
  SPI.endTransaction();
  return DAT_config[2];
}

int config_mode_exit() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(SS, LOW);
  delayMicroseconds(TRANSFER_WAIT);
  for (byte i = 0; i < CMD_config_exit_BYTES; i++) {
    DAT_config_exit[i] = SPI.transfer(CMD_config_exit[i]);  //SPI送受信，受信データビット反転
    delayMicroseconds(TRANSFER_WAIT);
  }
  digitalWrite(SS, HIGH);
  delay(FRAME_WAIT);
  SPI.endTransaction();
  return DAT_config_exit[1];
}

int model_check() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(SS, LOW);
  delayMicroseconds(TRANSFER_WAIT);
  for (byte i = 0; i < CMD_model_BYTES; i++) {
    DAT_model[i] = SPI.transfer(CMD_model[i]);  //SPI送受信，受信データビット反転
    delayMicroseconds(TRANSFER_WAIT);
  }
  digitalWrite(SS, HIGH);
  delay(FRAME_WAIT);
  SPI.endTransaction();
  return DAT_model[1];
}

void set_lock() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(SS, LOW);
  delayMicroseconds(TRANSFER_WAIT);
  for (byte i = 0; i < CMD_set_lock_BYTES; i++) {
    DAT_set_lock[i] = SPI.transfer(CMD_set_lock[i]);  //SPI送受信，受信データビット反転
    delayMicroseconds(TRANSFER_WAIT);
  }
  digitalWrite(SS, HIGH);
  delay(FRAME_WAIT);
  SPI.endTransaction();
}

void data_read(void) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(SS, LOW);
  delayMicroseconds(TRANSFER_WAIT);
  for (byte i = 0; i < CMD_read_BYTES; i++) {
    if (i == 3 || i == 4) {
      DAT_SPI_read[i] = SPI.transfer(CMD_read[i]) ^ 0xFF;  //SPI送受信，受信データビット反転
    } else {
      DAT_SPI_read[i] = SPI.transfer(CMD_read[i]);  //SPI送受信，受信データビット反転
    }
    delayMicroseconds(TRANSFER_WAIT);
  }
  digitalWrite(SS, HIGH);
  delay(FRAME_WAIT);
  SPI.endTransaction();
}

void keyboard_control() {
  //ノッチが前回と異なるとき
  if (notch_mc != notch_mc_latch) {
    int d = abs(notch_mc - notch_mc_latch);
    //力行ノッチ
    if (notch_mc >= 9 && notch_mc_latch >= 9) {
      //進段
      if ((notch_mc - notch_mc_latch) > 0) {
        for (int i = 0; i < d; i++) {
          Keyboard.write('Z');
        }
      }
      //戻し
      if ((notch_mc - notch_mc_latch) < 0) {
        for (int i = 0; i < d; i++) {
          Keyboard.write('A');
        }
      }
    }
  }

  //TS185用
  static bool notch_AAB_latch = notch_AAB;
  if (mode185 && notch_AAB_latch == 1 && notch_AAB == 0 && !flgEB) {
    Keyboard.write('K');
  }
  if (notch_AAB_latch == 1 && notch_AAB == 0 && flgEB) {
    flgEB = false;
  }
  if (mode185 && notch_AAB_latch == 0 && notch_AAB == 1) {
    Keyboard.write('.');
  }
  if (mode185 && notch_AAB == 2) {
    flgBRK = true;
    Keyboard.press('.');
  }
  if (mode185 && notch_AAB == 1 && flgBRK && !flgEB) {
    Keyboard.release('.');
    flgBRK = false;
  }
  notch_AAB_latch = notch_AAB;

  //ノッチが前回と異なるとき
  if (notch_brk != notch_brk_latch) {
    int d = abs(notch_brk - notch_brk_latch);
    //ブレーキノッチ
    if (notch_brk <= 9 && notch_brk_latch <= 9 && notch_brk > 0) {
      //戻し
      if ((notch_brk - notch_brk_latch) > 0) {
        for (int i = 0; i < d; i++) {
          if (mode185) {
            Keyboard.write('K');
          } else {
            Keyboard.write(',');
          }
        }
      }
      //ブレーキ
      if ((notch_brk - notch_brk_latch) < 0) {
        for (int i = 0; i < d; i++) {
          if (mode185) {
            Keyboard.write('L');
          } else {
            Keyboard.write('.');
          }
        }
      }
    }
    if (notch_brk == 0) {
      Keyboard.write('/');
    }
  }

  btnA = (DAT_SPI_read[4] >> 7 & 1 );
  btnB = (DAT_SPI_read[4] >> 6 & 1 );
  btnC = (DAT_SPI_read[4] >> 5 & 1 );
  btnD = (DAT_SPI_read[4] >> 4 & 1 );
  btnStart = (DAT_SPI_read[3] >> 3 & 1);
  btnSelect = (DAT_SPI_read[3] & 1);
}

void button_Control(void) {
  static bool btnA_latch = btnA;
  static bool btnB_latch = btnB;
  static bool btnC_latch = btnC;
  static bool btnD_latch = btnD;
  static bool btnStart_latch = btnStart;
  static bool btnSelect_latch = btnSelect;
  if (btnA != btnA_latch) {
    if (btnA) {
      Keyboard.write(KEY_UP_ARROW);
    }
  }
  if (btnB != btnB_latch) {
    if (btnB) {
      Keyboard.write(KEY_RETURN);
    }
  }
  if (btnC != btnC_latch) {
    if (btnC) {
      Keyboard.write(KEY_DOWN_ARROW);
    }
  }
  if (btnStart != btnStart_latch) {
    if (btnStart) {
      if (mode185) {
        Keyboard.press(KEY_BACKSPACE);
      } else {
        Keyboard.press(KEY_RETURN);
      }
    } else {
      if (mode185) {
        Keyboard.release(KEY_BACKSPACE);
      } else {
        Keyboard.release(KEY_RETURN);
      }
    }
  }

  if (btnSelect != btnSelect_latch) {
    if (btnSelect) {
      Keyboard.write(KEY_KP_PLUS);
    }
  }

  btnA_latch = btnA;
  btnB_latch = btnB;
  btnC_latch = btnC;
  btnStart_latch = btnStart;
  btnSelect_latch = btnSelect;
}
