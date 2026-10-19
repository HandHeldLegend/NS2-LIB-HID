# Licensing — `NS2-LIB-HID`

`NS2-LIB-HID` is licensed under **CC BY-NC 4.0** (non-commercial):

- <https://creativecommons.org/licenses/by-nc/4.0/>
- <https://creativecommons.org/licenses/by-nc/4.0/legalcode>

For licensing inquiries (including commercial licensing), contact:
**support@handheldlegend.com**

Project intent: using this codebase as a reference, learning from it, and even borrowing
large portions is not something we will police or care about.
Our concern is people taking the entirety of the code as-is and selling it as a product.

This is a summary for convenience; file headers and the linked license texts control.

## Acknowledgements

The Switch 2 BLE protocol knowledge used by this library (GATT layout, advertisement format,
pairing key exchange, command set and simulated memory contents) comes from public research:

- [zhantss/ESP32-BLE5-NSController-Emulator](https://github.com/zhantss/ESP32-BLE5-NSController-Emulator)
  (MIT License) — reference ESP32/NimBLE implementation of a Pro Controller 2 emulator. The
  simulated memory blocks in `ns2_lib_memory.c` and several fixed command responses in
  `ns2_lib_protocol.c` are taken from observed controller data published there.
- [ndeadly/switch2_controller_research](https://github.com/ndeadly/switch2_controller_research) —
  protocol documentation for the Switch 2 controller Bluetooth interface, commands and reports.
