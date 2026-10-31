EvilDuckS3 script library (48+ payloads)
=======================================

Folders
  demos/      Feature tests (delay, repeat, locale, LED, KEYCODE)
  windows/    Windows shortcuts and apps
  linux/      Linux desktop launcher scripts
  macos/      macOS Spotlight / shortcuts
  advanced/   VAR, IF, WHILE, SAVE, READ, ATTACKMODE

Upload to the duck
------------------
1. Web UI: Upload -> pick SPIFFS or SD (use sd:name.txt for SD)
2. SPIFFS bulk flash:
     cd scripts
     .\sync_to_data.ps1
     Arduino IDE -> Tools -> ESP32 Sketch Data Upload
3. SD card: copy folders to /scripts/ on the card
4. Serial CLI: run demos/01_hello_windows.txt

Device paths
------------
  SPIFFS:  demos/01_hello_windows.txt
  SD:      sd:payload.txt  ->  /scripts/payload.txt

Built-in examples (firmware, first boot): examples/hello.txt etc.

Only test on systems you own or are authorized to use.
