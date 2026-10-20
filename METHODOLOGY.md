# How we worked out the Switch 2 controller protocol

This document describes the reverse engineering behind NS2-LIB-HID: what we measured, the
tools we used, and how each conclusion was reached and then checked on hardware. The work was
done by Hand Held Legend in October 2026, on our own hardware and consoles, against retail
Nintendo controllers we own.

Our own work is kept apart from what came from other people. Public research we relied on is
listed in [Prior work we used](#prior-work-we-used), with what each source contributed. Everything
else here was found by us, using the methods below.

Captures and logs from this work are not in this repository. They hold console and controller
Bluetooth addresses, serial numbers and pairing keys. The numbers quoted here are taken from them.

## Contents

- [Tools](#tools)
- [How we worked](#how-we-worked)
- [USB: the NSO GameCube controller on a Switch 2](#usb-the-nso-gamecube-controller-on-a-switch-2)
- [Taking no pairing slot](#taking-no-pairing-slot)
- [Telling the host apart](#telling-the-host-apart)
- [Rumble](#rumble)
- [Colours](#colours)
- [Bluetooth Classic](#bluetooth-classic)
- [Bluetooth LE](#bluetooth-le)
- [Prior work we used](#prior-work-we-used)

## Tools

| Tool | What it is | What we used it for |
|---|---|---|
| Hardware USB sniffer | Alex Taradov's open-source full-speed/high-speed USB analyzer, placed inline between a host and a device | Every USB capture below. It records the bus itself, so the host's timing, NAKs and STALLs are visible too. Software sniffers on a PC can't see that, and can't see a console at all. |
| `usb_dump.py` | Our decoder for the sniffer's captures | Turns raw packets into transactions: control transfers with readable request names, bulk commands and replies in order, and interrupt reports with their rates and changes. |
| nRF52840 dongle with Nordic's nRF Sniffer for Bluetooth LE 4.1 | An LE sniffer, driven from Python through Nordic's SnifferAPI | Following a controller's advertising and its connection to the console. |
| `ble_follow.py` | Our capture script for the nRF Sniffer | Finds the first advertiser carrying Nintendo's manufacturer data and follows its connection. It can be limited to one advertising channel, so the console's connect request isn't missed. |
| `ble_decrypt.py` | Our LE link-layer decryptor | Decrypts a captured, encrypted LE connection (see [Bluetooth LE](#bluetooth-le)). |
| Wireshark and tshark | Packet analysis | Dissecting both kinds of capture, and as the parser our scripts read through. |
| HOJA gamepads (GCU 2, GCU 1) and the HOJA dongle | Our own controllers, RP2040 based | The test devices. Firmware built for the bench recorded what the console sent into a flash log, read back afterwards over USB. |
| `hoja_bench.py` | Our bench control script | Flashing and rebooting the test controllers and the dongle without touching them, so a test could run start to finish the same way each time. |
| Retail controllers | An NSO GameCube controller and a Joy-Con 2 | The reference for every comparison. |
| Consoles and hosts | A Switch 2, a Switch 1, and a Windows PC with and without Steam running | The hosts each behaviour was measured on. |

## How we worked

Each item followed the same loop:

1. **Capture a retail controller** doing the thing in question, on the real console.
2. **Script the human side of the test.** When a capture had to show buttons, a fixed order was
   agreed before recording. For example: A, B, X, Y, Z, ZL, L analog then click, R analog then
   click, Start, D-pad up/down/left/right, both sticks, Capture, C, Home. Each bit and byte in the
   report could then be matched to one action.
3. **Form a hypothesis**: what a field means, or what the console is waiting for.
4. **Build it into our firmware** and run our controller on the same console.
5. **Capture our controller** and compare it with the retail one, transfer by transfer.
6. **Change one thing at a time** until the two matched, or until the difference explained what
   the console did.

Several conclusions below came from experiments where we answered the console differently on
purpose and watched how it reacted.

## USB: the NSO GameCube controller on a Switch 2

**Enumeration.** We captured the retail controller on the Switch 2. It enumerates as:
- 057E:2073, bcdDevice 0x0101, bcdUSB 0x0200;
- a composite device (IAD, class EF/02/01);
- a HID interface with interrupt endpoints 0x81 and 0x01, bInterval 4, so one report every 4 ms;
- a vendor interface with bulk endpoints 0x02 and 0x82.

The console never reads the HID report descriptor. That told us the console identifies the
controller by its IDs and its replies, not by HID parsing.

**Vendor requests.** Straight after SET_CONFIGURATION, the console makes three vendor control
requests:

| Request | Direction | What it is |
|---|---|---|
| 0x03 | IN, 64 bytes | The device-info block, the same data as memory address 0x13000: serial, VID/PID and colours |
| 0x02 | IN, 16 bytes | Firmware version, then the controller's Bluetooth address, byte-reversed |
| 0x04 | OUT, value 0x0276 | No data |

We decoded the 0x03 and 0x02 replies by comparing them with the controller's known identity.

**Bulk commands.**
- Commands on the bulk endpoint use the same command set as Bluetooth: `[command][0x91][transport][subcommand]...`.
  The transport byte is 0x00 on USB and 0x01 on Bluetooth.
- Replies use `00 F8` in the header where Bluetooth replies use `10 78`.
- We logged the full sequence from the captures: init, pairing, memory reads of 0x13080, 0x130C0,
  0x1FC040, 0x13040, 0x13100, 0x13140 and 0x13060, feature selection, and the player LED.
- Two replies carry a different direction byte on the retail controller: 0x01/0x0C and 0x0A/0x08.
  We copy them as captured.

**The input report.** Report 0x0A, 64 bytes, decoded with the button test:

| Bytes | Content |
|---|---|
| 1 | Counter |
| 2 | Power state |
| 3–5 | Buttons, in the Pro Controller 2's bit layout |
| 6–8 | Left stick, two 12-bit values |
| 9–11 | C-stick, two 12-bit values |
| 12 | 0x30, or 0x38 once rumble is enabled |
| 13 | Analog L (idle about 35, about 233 at the click) |
| 14 | Analog R |

The button bits we mapped this way:

| Bit | Button | Bit | Button |
|---|---|---|---|
| 0 | B | 10 | Left |
| 1 | A | 11 | Up |
| 2 | Y | 12 | L |
| 3 | X | 13 | ZL |
| 4 | R | 16 | Home |
| 5 | Z | 17 | Capture |
| 6 | Start | 20 | C |
| 8 | Down | | |
| 9 | Right | | |

Minus and the stick clicks follow the Pro Controller 2's positions for them.

**Calibration.** Our firmware applies its own stick and trigger calibration, then reports the
full range. The calibration blocks we serve (sticks at 0x130A8 and 0x130E8, triggers at 0x13140)
describe the full range to match.

## Taking no pairing slot

On USB the console pairs the controller for later wireless use, through command 0x15:
- 0x01 exchanges addresses;
- 0x04 exchanges key material;
- 0x02 confirms the key;
- 0x03 finalises.

A wired adapter shouldn't use up one of the console's pairing slots, so we tested what the
console does with each kind of answer.

1. **Fresh against already paired.** We captured the retail controller twice: once freshly paired,
   then again after it was already paired. The second time the console sends 0x01, then goes
   straight to 0x02 and 0x03, and skips 0x04. So the console decides what to do from the address
   the controller returns, and it remembers controllers.
2. **Answer variants.** Our firmware answered pairing in four ways:
   - normally;
   - with a bare reply header and no data ("decline");
   - not at all;
   - with a wrong key confirmation.
3. **Result.** With the decline, the console skips 0x04, 0x02 and 0x03. It asks 0x01 once more
   after setting the player LED, then carries on and uses the controller.
4. **Check.** We cleared the console's pairings and plugged our controller in twice. Both sessions
   were identical: the second still asked 0x01 as if for the first time. Nothing was stored.

Decline is the library's default answer for USB (`NS2_USB_PAIRING_DECLINE`).

## Telling the host apart

One cable start has to give the right device on three kinds of host:
- the NSO controller on a Switch 2;
- Nintendo's GameCube adapter on a Switch 1;
- Slippi on a PC.

We captured the retail controller on each host, and the PC both with and without Steam running.
These differences appear within milliseconds of plug-in:

| Host | What it does first |
|---|---|
| PC (Windows) | Asks for 64 bytes of the device descriptor first. Then reads strings and the device qualifier before SET_CONFIGURATION, then requests the Microsoft OS descriptors. With Steam running, Steam then reads controller memory, but never makes vendor request 0x03 and never pairs. |
| Switch 1 | Asks for 8 bytes first. Reads string 0 (the language list) before SET_CONFIGURATION, then the HID report descriptor. Never makes a vendor request or sends a command. |
| Switch 2 | Asks for 8 bytes first. Reads no strings. Makes vendor request 0x03 about 2 ms after SET_CONFIGURATION. |

**The rule we built from that.**

| What the controller sees | What it decides |
|---|---|
| A first device request longer than 8 bytes | A PC |
| A string read before configuration | A Switch 1 |
| Configured, but no vendor request 0x03 within 250 ms | A Switch 1 |
| Vendor request 0x03 after configuration | A Switch 2 |

We then ran our controller on all four hosts, capturing each run. Two problems came out of those
runs:

- **Restarting too early.** Leaving a Switch 1 in the middle of its setup made it wait about 7.7 s
  before it looked at the port again. Restarting only after it finished configuring the device
  (about 200 ms, at most 1 s) brought the adapter up about 0.7 s after plug-in.
- **The stick's rest position.** A Switch 1 takes a GameCube controller's stick rest position from
  the first report it sees, and keeps it until the console reboots. Early, unsettled reports made
  the stick read as held down. The adapter now keeps port 1 empty for its first 150 ms.

## Rumble

**Game rumble.** Games drive rumble with HID output report 0x03.
- We captured it in play on our controller.
- Its fields are 2-bit states: rumble in the low bit, brake in the high bit, the same scheme as a
  GameCube console's own rumble command.
- Samples from the captures: `61 00 01 00` is rumble on, `51 02 00 00` is brake (games send it to
  stop), and `50 00 00 00` is stop.
- Games pulse rumble and brake about every 50 ms. The motor follows the rumble bits, masked with
  0x55 across bytes 2 to 4.

**The console's own sounds.** Command 0x0A/0x02 is not motor on/off. Our first build treated it
that way, and the motor ran without stopping. ndeadly's command documentation lists it as playing a
built-in vibration sample by ID: 0 stops, 1 is a low buzz, 2 the "search for controllers" alarm, 3
the click-click of a connection, 4 to 7 short beeps. The GameCube controller has a single on/off
motor, not the Pro Controller 2's HD rumble. So we wrote our own on/off timing for each sample, to
approximate how it sounds and feels on one motor.

## Colours

We put distinct colours into every field of the 0x13000 block: body, buttons, highlight and grip.
The Switch 2 still drew its standard GameCube controller icon, so the console ignores colours for
this controller type.

## Bluetooth Classic

We tested whether a Switch 2 would accept a Pro Controller 2 over Bluetooth Classic. Our firmware
claimed the Pro Controller 2's IDs (057E:2069) and its HID report descriptor, and recorded
everything the console sent to flash. The console answered with Switch 1 subcommands, and then
Switch 1 rumble. So over Classic a Switch 2 always uses the Switch 1 protocol, and the Switch 2
protocol needs LE.

## Bluetooth LE

**Decrypting a retail session.** We captured the retail NSO GameCube controller pairing with the
Switch 2 over LE. The pairing commands are sent in plaintext before encryption starts, so the key
can be rebuilt from the capture: the long-term key is the XOR of the two halves exchanged in
command 0x15/0x04. `ble_decrypt.py` then:
1. computes the session key the standard way, from the key and the session-key diversifiers in
   LL_ENC_REQ / LL_ENC_RSP;
2. decrypts each packet with AES-CCM, using the packet counter and IV from the link layer;
3. resynchronises its counter when the sniffer has missed packets.

All but two packets of the encrypted part decrypted with a valid MIC; the sniffer had
caught those two damaged.

**What the encrypted part showed us about the GameCube controller over LE:**
- The console sends commands as write commands to handle 0x0016, after a 5-byte prefix.
- The controller replies with notifications on 0x001A, without a prefix.
- The same two odd reply headers appear as on USB.
- Input arrives on 0x000E once the console enables its CCC (0x000F) and writes 85 00 to the rate
  descriptor 0x0010. Each report is 63 bytes: the USB report 0x0A without its ID byte.

**Link-layer order.** The capture also showed the order on the link:
1. The controller starts the data-length update.
2. The console runs a feature exchange, then a PHY update to 2M.
3. The console starts GATT about 100 ms after the PHY update.

**Our radio, which has no 2M PHY.** We then ran our own controller over LE with recording
firmware, sniffing each run.
- The console connected at 1M and held the link for minutes.
- It never sent a PHY request, never started encryption or GATT, and allowed us only 27-byte
  packets, where the retail controller gets 251.
- We paired our controller over USB first, keeping the key, then advertised as that paired
  controller. The console connected within about a second, so it knew the controller, but it
  stopped at the same point.

Our conclusion is that the console needs the 2M PHY from a controller before it will talk to it
over LE. We have stopped pursuing LE on a radio without 2M.

## Prior work we used

| Source | What we took from it |
|---|---|
| [zhantss/ESP32-BLE5-NSController-Emulator](https://github.com/zhantss/ESP32-BLE5-NSController-Emulator) (MIT) | The original Pro Controller 2 BLE emulation behind this library's first version: GATT layout, advertising format, pairing key exchange, simulated memory blocks, fixed command replies. Credited in [LICENSING.md](LICENSING.md). |
| [ndeadly/switch2_controller_research](https://github.com/ndeadly/switch2_controller_research) | Protocol documentation for the Bluetooth interface, its commands and reports, including the list of vibration samples for command 0x0A/0x02. Credited in [LICENSING.md](LICENSING.md). |
| A published console capture by ndeadly, as cited by the Switch2Connect project | Pointed us to HID output report 0x03 as the game rumble report. We confirmed it, and worked out its meaning, from our own captures (see [Rumble](#rumble)). |
| SoulCalDan's FPGA NSO GameCube adapter | Showed that a Switch 2 accepts a non-Nintendo NSO GameCube controller over USB, and gave an outline of the command flow. It was used as a reference only. No code was taken from it, and every value in the USB support comes from our own captures. |

All of the following is our own work, based on the captures and experiments described above:
- USB enumeration and the vendor requests;
- the bulk command flow and its quirks;
- the report 0x0A layout and button map;
- calibration;
- declining pairing without taking a slot;
- host detection;
- the rumble report's meaning, and the on/off timing of each vibration sample;
- the colour, Classic and LE experiments;
- decrypting and decoding the retail LE session.
