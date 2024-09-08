#include <SPI.h>
#include <SD.h>
#include <string.h>
#include "Keyboard.h"

File scriptFile;
const int chipSelectPin = 4;
const int ledPin = 9;  
const String scriptFileName = "script.txt";
bool sdCardPresent = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  Keyboard.begin();
}

void loop() {
  if (!SD.begin(chipSelectPin)) {
    // SD card missing, blink LED every 1 second
    blinkLED(1000);
    sdCardPresent = false;
    return;
  }

  if (!sdCardPresent) {
    sdCardPresent = true;
    executeScript();
  }
}

void executeScript() {
  scriptFile = SD.open(scriptFileName);
  
  if (!scriptFile) {
    // No script file found, blink LED every 2 seconds
    while (true) {
      blinkLED(2000);
    }
  }

  String currentLine = "";
  while (scriptFile.available()) {
    char character = scriptFile.read();
    if (character == '\n') {
      executeCommand(currentLine);
      currentLine = "";
    } else if ((int)character != 13) {
      currentLine += character;
    }
  }
  executeCommand(currentLine);
  scriptFile.close();
  Keyboard.end();
}

void executeCommand(String line) {
  int firstSpace = line.indexOf(' ');
  String command = (firstSpace == -1) ? line : line.substring(0, firstSpace);
  String argument = (firstSpace == -1) ? "" : line.substring(firstSpace + 1);

  if (command == "STRING") {
    Keyboard.print(argument);
  } else if (command == "DELAY") {
    delay(argument.toInt());
  } else if (command == "REM") {
    // This is a comment, do nothing
  } else {
    typeKeys(line);
  }

  Keyboard.releaseAll();
  blinkLED(100);  
}

void typeKeys(String keyString) {
  while (keyString.length() > 0) {
    int nextSpace = keyString.indexOf(' ');
    String key = (nextSpace == -1) ? keyString : keyString.substring(0, nextSpace);
    
    pressKey(key);
    keyString = (nextSpace == -1) ? "" : keyString.substring(nextSpace + 1);

    delay(5);  
  }
}

void pressKey(String key) {
  if (key.length() == 1) {
    Keyboard.press(key[0]);
  } else if (key == "ENTER") {
    Keyboard.press(KEY_RETURN);
  } else if (key == "CTRL") {
    Keyboard.press(KEY_LEFT_CTRL);
  } else if (key == "SHIFT") {
    Keyboard.press(KEY_LEFT_SHIFT);
  } else if (key == "ALT") {
    Keyboard.press(KEY_LEFT_ALT);
  } else if (key == "GUI") {
    Keyboard.press(KEY_LEFT_GUI);
  } else if (key == "UP" || key == "UPARROW") {
    Keyboard.press(KEY_UP_ARROW);
  } else if (key == "DOWN" || key == "DOWNARROW") {
    Keyboard.press(KEY_DOWN_ARROW);
  } else if (key == "LEFT" || key == "LEFTARROW") {
    Keyboard.press(KEY_LEFT_ARROW);
  } else if (key == "RIGHT" || key == "RIGHTARROW") {
    Keyboard.press(KEY_RIGHT_ARROW);
  } else if (key == "DELETE") {
    Keyboard.press(KEY_DELETE);
  } else if (key == "PAGEUP") {
    Keyboard.press(KEY_PAGE_UP);
  } else if (key == "PAGEDOWN") {
    Keyboard.press(KEY_PAGE_DOWN);
  } else if (key == "HOME") {
    Keyboard.press(KEY_HOME);
  } else if (key == "ESC") {
    Keyboard.press(KEY_ESC);
  } else if (key == "INSERT") {
    Keyboard.press(KEY_INSERT);
  } else if (key == "TAB") {
    Keyboard.press(KEY_TAB);
  } else if (key == "END") {
    Keyboard.press(KEY_END);
  } else if (key == "CAPSLOCK") {
    Keyboard.press(KEY_CAPS_LOCK);
  } else if (key.startsWith("F") && key.length() == 2) {
    char functionKey = key.charAt(1);
    if (functionKey >= '1' && functionKey <= '9') {
      Keyboard.press(KEY_F1 + (functionKey - '1'));
    }
  } else if (key.startsWith("F") && key.length() == 3) {
    char functionKey = key.substring(1).toInt();
    if (functionKey >= 10 && functionKey <= 12) {
      Keyboard.press(KEY_F10 + (functionKey - 10));
    }
  } else if (key == "SPACE") {
    Keyboard.press(' ');
  }
}

void blinkLED(int delayTime) {
  digitalWrite(ledPin, HIGH);
  delay(delayTime / 2);  
  digitalWrite(ledPin, LOW);
  delay(delayTime / 2);  
}
