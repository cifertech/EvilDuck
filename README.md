<div align="center">

  <img src="https://user-images.githubusercontent.com/62047147/195847997-97553030-3b79-4643-9f2c-1f04bba6b989.png" alt="logo" width="100" height="auto" />
  
  <h1> EvilDuck </h1>
  <p> DIY USB Rubber Ducky based on Arduino Micro </p>


<!-- Badges -->
<a href="https://github.com/cifertech/evilduck" title="Go to GitHub repo"><img src="https://img.shields.io/static/v1?label=cifertech&message=evilduck&color=purple&logo=github" alt="cifertech - evilduck"></a>
<a href="https://github.com/cifertech/evilduck"><img src="https://img.shields.io/github/stars/cifertech/evilduck?style=social" alt="stars - evilduck"></a>
<a href="https://github.com/cifertech/evilduck"><img src="https://img.shields.io/github/forks/cifertech/evilduck?style=social" alt="forks - evilduck"></a>

   
<h4>
    <a href="https://twitter.com/techcifer">TWITTER</a>
  <span> · </span>
    <a href="https://www.instagram.com/cifertech/">INSTAGRAM</a>
  <span> · </span>
    <a href="https://www.youtube.com/@techcifer">YOUTUBE</a>
  <span> · </span>
    <a href="https://cifertech.net/">WEBSITE</a>
  </h4>
</div>

<br />



## 📖 Explore the Full Documentation

Ready to dive deeper into EvilDuck's details? Discover the full story, in-depth tutorials, and all the exciting features in our comprehensive [documentation](https://cifertech.net/nrfbox-your-all-in-one-gadget-for-ble-and-2-4ghz-networks/). Click the link and explore further!
  

## 🌟 About the Project
EvilDuck enables anyone to build a powerful USB keystroke injection tool with simple hardware. It’s inspired by the original USB Rubber Ducky but allows for more flexibility by using a customizable microcontroller (Arduino Micro) and an SD card for payload storage. This makes it ideal for both educational purposes and professional use in security assessments.


## 🎯 Features, Software and Hardware
### Features
- **SD Card Hot-Swap**: Detects SD card insertion even when powered.
- **Keystroke Injection**: Execute commands stored in a `script.txt` file.
- **LED Indicators**:
  - Blinks every second when no SD card is present.
  - Blinks every 2 seconds when no script is found.
  - Blinks on each keystroke during script execution.
- **Supports Rubber Ducky-style Scripts**: Commands like `STRING`, `DELAY`, and special keypresses.


### Software Requirements
- **Arduino IDE**: Version 1.8 or higher
- **Libraries**:
  - SD
  - SPI
  - Keyboard

### Hardware Requirements
- **Microcontroller**: Arduino Micro or compatible
- **MicroSD card**: Formatted as FAT16/32
- **LED**: Connected to indicate status
- **SD Card Module**: For reading scripts


## 🔌 Schematic
The detailed schematic of the EvilDuck setup can be found [here](hardware/schematic.pdf). This will guide you in wiring the SD card module and LED to your Arduino Micro.

1. Connect the SD card module to the Arduino Micro using these pins:
   - **CS** -> Pin 4
   - **MOSI** -> Pin 11
   - **MISO** -> Pin 12
   - **SCK** -> Pin 13
2. Connect the status LED:
   - **Anode** -> Pin 9
   - **Cathode** -> Ground


## ⚠ Future Changes
Future updates may include:
- **Improved Error Handling**: More detailed feedback on SD card or file issues.
- **Multiple Script Files**: Ability to load and execute multiple scripts.
- **Additional Commands**: Expanding the supported command set for more advanced payloads.


<!-- License -->
## :warning: License

Distributed under the MIT License. See LICENSE.txt for more information.


<!-- Contact -->
## :handshake: Contact

▶ Support me on Patreon [patreon.com/cifertech](https://www.patreon.com/cifertech)

CiferTech - [@twitter](https://twitter.com/techcifer) - CiferTech@gmali.com

Project Link: [https://github.com/cifertech/EvilDuck](https://github.com/cifertech/EvilDuck)


