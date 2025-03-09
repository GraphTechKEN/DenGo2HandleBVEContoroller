- 簡単な説明  
PS用電車でGOコントローラーをBVEおよびJRETSで使用するスケッチです
Arduino Leonard および Micro 用 です  
TS185用に、13番ピンをモード切替用に使用、適宜スイッチやジャンパを設置すること(GNDとショートで通常、オープンで185系モード)
>- 通常モード:P5/B8/EB
>- JRETS185モード:P5/B8/自動帯/EB
  
- キーアサイン  
A:↑  
B:Return  
C:↓  
Start:Return (185モード:BackSpace)  
Select:+  
  
- PADピンアサイン(本体側に向かって右から)  
1:MISO ※5ピンの+5Vと2kΩ程度の抵抗で接続してプルアップする  
2:MOSI  
3:(NC)  
4:GND  
5:+5V  
6:SS  
7:SCK  
8:(NC)  
9:(NC)

- 注意事項
適宜スケッチの内容を改変し用途に合わせてご使用ください  
動作保証はありません
いかなる損害が発生しても当方は一切の責任を負いかねます
  
//Licences  
Copyright (c) 2025 GraphTechKEN // Released under the MIT license  // http://opensource.org/licenses/mit-license.php  
SPI test Arduino: Master, DUALSHOCK: Slave // // Copyright (c) 2015 Kazumasa ISE // Released under the MIT license // http://opensource.org/licenses/mit-license.php
