# Wireless Keypad using NodeMCU to use with Homey

### How to connect the pins

You need basic soldering skills (and a soldering iron ofcourse) to connect the keypad and buzzer to the NodeMCU.
Connect the keypad and buzzer as follows:
(coming soon) 

### How to upload to a NodeMCU board

1. Arduino IDE software [download here](https://www.arduino.cc/en/Main/Software)
2. In the Arduino IDE, open "Preferences" from the "File" menu. Add the following URL to the "Additional Boards Manager URL's": http://arduino.esp8266.com/stable/package_esp8266com_index.json
3. Connect the NodeMCU to your computer.
4. In the "Tools" menu, select "Node MCU 1.0" in the "Board" sub menu. Select the port in the "Port" sub menu.
5. Click the "Upload" button (round button with an arrow pointing to the right)
6. The upload should take less than a minute.

### How to configure

See the [repo for the Homey app](https://github.com/SergeRegoor/nl.regoor.wirelesskeypad) on how to pair the device with your Homey.