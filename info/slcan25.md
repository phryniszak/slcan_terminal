# Slcan Manual

## 2.) Application Developer Manual - Slcan

This manual is for software developers who want to communicate with the CANable 2.5 from a computer.

In Slcan all commands and responses are sent as **ASCII** strings which is slow. See [Comparison](#Speed_Comparison).

For professional applications use Candlelight and not Slcan.

The **first character** is the command to execute. The **last character** is always a Carriage Return.

### Slcan Commands

| Command | Condition | Version | Meaning | Comment |
|---------|-----------|---------|---------|---------|
| `A1\r` | Closed | legacy | Enable Auto Retransmission | Retransmit packets until they are acknowledged |
| `A0\r` | Closed | legacy | Disable Auto Retransmission | Enable one shot mode |
| `C\r` | Open/Closed | legacy | Close the adapter | Shut down CAN connection, reset to default settings |
| `L7\r` | Open/Closed | 100 | Enable Bus Load reports every 700 ms | L1: Report interval= 100 ms, L50: Interval= 5 seconds<br>Valid range of interval: 0 ... 100 (max 10 seconds) |
| `L0\r` | Open/Closed | 100 | Disable Bus Load report | |
| `O\r` | Closed | legacy | Open adapter | Connect to CAN bus with the mode set by M0 / M1 |
| `ON\r` | Closed | 100 | Open in normal mode | Ignore settings with M0 / M1 |
| `OS\r` | Closed | 100 | Open in silent mode | Ignore settings with M0 / M1 |
| `OI\r` | Closed | 100 | Open in internal loopback mode | Ignore settings with M0 / M1 |
| `OE\r` | Closed | 100 | Open in external loopback mode | Ignore settings with M0 / M1 |
| `V\r` | Open/Closed | legacy | Return detailed info: Version/Board/MCU/Limits | Returns seven key/value pairs. See [Version Info](#Slcan_Version) |

**Set Modes**

| Command | Condition | Version | Meaning | Comment |
|---------|-----------|---------|---------|---------|
| `M1\r` | Closed | legacy | Enable silent mode | The next command `O\r` will open in bus monitoring mode |
| `M0\r` | Closed | legacy | Disable silent mode | The next command `O\r` will open in normal mode |
| `MA\r` | Closed | 100 | Enable Auto Retransmission mode (same as A1) | Retransmit packets until they are acknowledged |
| `Ma\r` | Closed | 100 | Disable Auto Retransmission mode (same as A0) | Enable one-shot mode |
| `MD\r` | Open/Closed | 100 | Enable Debug Messages | Firmware sends string messages to the host |
| `Md\r` | Open/Closed | 100 | Disable Debug Messages | Do not send debug messages |
| `ME\r` | Open/Closed | 100 | Enable CAN Error reports | CAN errors are sent to the host |
| `Me\r` | Open/Closed | 100 | Disable CAN Error reports | CAN error reporting is off (deprecated!) |
| `MF\r` | Open/Closed | 100 | Enable Feedback mode | For each command a success/error feedback is sent |
| `Mf\r` | Open/Closed | 100 | Disable Feedback mode | Command Feedback mode is off (deprecated!) |
| `MI\r` | Open/Closed | 100 | Enable Identify mode | The green/blue LED's start blinking |
| `Mi\r` | Open/Closed | 100 | Disable Identify mode | The LED's stop blinking |
| `MM\r` | Open/Closed | 100 | Enable Tx echo report marker | Report all successfully sent packets with a marker |
| `Mm\r` | Open/Closed | 100 | Disable Tx echo report marker | No Tx Echo report |
| `MR\r` | Open/Closed | 100 | Enable 120 Ω Termination Resistor | Supported only by few boards |
| `Mr\r` | Open/Closed | 100 | Disable 120 Ω Termination Resistor | Supported only by few boards |
| `MS\r` | Open/Closed | 100 | Enable ESI report | Report the ESI flag of received CAN FD messages |
| `Ms\r` | Open/Closed | 100 | Disable ESI report | No ESI report |
| `MDEFMS\r` | Open/Closed | 100 | Enable Debug, Error, Feedback, Echo, ESI reports | You can set all modes at once in one command |

**Set Baudrates**

| Command | Condition | Version | Meaning | Comment |
|---------|-----------|---------|---------|---------|
| `S0\r` | Closed | legacy | Set nominal baudrate 10 kbaud | Samplepoint 87.5% |
| `S1\r` | Closed | legacy | Set nominal baudrate 20 kbaud | Samplepoint 87.5% |
| `S2\r` | Closed | legacy | Set nominal baudrate 50 kbaud | Samplepoint 87.5% |
| `S3\r` | Closed | legacy | Set nominal baudrate 100 kbaud | Samplepoint 87.5% |
| `S4\r` | Closed | legacy | Set nominal baudrate 125 kbaud | Samplepoint 87.5% |
| `S5\r` | Closed | legacy | Set nominal baudrate 250 kbaud | Samplepoint 87.5% |
| `S6\r` | Closed | legacy | Set nominal baudrate 500 kbaud | Samplepoint 87.5% |
| `S7\r` | Closed | legacy | Set nominal baudrate 800 kbaud | Samplepoint 87.5% |
| `S8\r` | Closed | legacy | Set nominal baudrate 1 Mbaud | Samplepoint 87.5% |
| `S9\r` | Closed | legacy | Set nominal baudrate 83.333 kbaud | Samplepoint 87.5% |
| `Y0\r` | Closed | 100 | Set CAN FD data baudrate 500 kbaud | Samplepoint 75% |
| `Y1\r` | Closed | 100 | Set CAN FD data baudrate 1 Mbaud | Samplepoint 75% |
| `Y2\r` | Closed | legacy | Set CAN FD data baudrate 2 Mbaud | Samplepoint 75% |
| `Y4\r` | Closed | 100 | Set CAN FD data baudrate 4 Mbaud | Samplepoint 75% |
| `Y5\r` | Closed | legacy | Set CAN FD data baudrate 5 Mbaud | Samplepoint 75% |
| `Y8\r` | Closed | 100 | Set CAN FD data baudrate 8 Mbaud | Samplepoint 50% (75% does not work on STM32G431) |
| `s4,69,10,7\r` | Closed | 100 | Set nominal bitrate: Prescaler=4, Seg1=69, Seg2=10, Synchr. Jump Width=7 | Set 500 kbaud, Samplepoint 87.5%<br>See [Samplepoint](#Samplepoint) |
| `y4,9,10,7\r` | Closed | 100 | Set data bitrate: Prescaler=4, Seg1=9, Seg2=10, Synchr. Jump Width=7 | Set 2 Mbaud, Samplepoint 50%<br>See [Samplepoint](#Samplepoint) |

**Set Filters**

| Command | Condition | Version | Meaning | Comment |
|---------|-----------|---------|---------|---------|
| `F7E8,7FF\r` | Closed | 100 | Set a mask filter for only one ID: 7E8 (11 bit) | You can set up to **8 mask filters** separated by semicolons.<br>11 bit and 29 bit filters can be mixed.<br>See [CAN Filters](#Filter). |
| `F18DA00F1,1FFF00FF\r` | Closed | 100 | Set a mask filter for 256 IDs: 18DAXXF1 (29 bit) | |
| `F7E0,7F0;720,7F0;730,7F0\r` | Closed | 100 | Set 3 filters for 16 ID's each: 7EX, 72X and 73X | |
| `f\r` | Closed | 100 | Clear all filters | Remove all filters |

**Boot Mode**

| Command | Condition | Version | Meaning | Comment |
|---------|-----------|---------|---------|---------|
| `*Boot0:Off\r` | Closed | 100 | Disable pin BOOT0 | See [Hardware Misdesign](#Hardware) |
| `*Boot0:?\r` | Open/Closed | 100 | Request status of pin BOOT0 | returns `+1\r` if enabled or `+0\r` if disabled |
| `*DFU\r` | Open/Closed | 100 | Enable pin BOOT0 and enter DFU mode | If feedback = `FBK_ResetRequired`: Reconnect USB cable! |

**Transmit Packets**

| Command | Condition | Version | Meaning | Comment |
|---------|-----------|---------|---------|---------|
| `Txxxxxxxxx\r` | Open | legacy | Send classic packet with 29 bit ID | Bits: IDE &nbsp; See [Slcan Packets](#Slcan_Packets) |
| `txxxxxxxxx\r` | Open | legacy | Send classic packet with 11 bit ID | Bits: None |
| `Rxxxxxxxxx\r` | Open | legacy | Send Remote Transmission Request with 29 bit ID | Bits: RTR + IDE |
| `rxxxxxxxxx\r` | Open | legacy | Send Remote Transmission Request with 11 bit ID | Bits: RTR |
| `Dxxxxxxxxx\r` | Open | legacy | Send CAN FD packet, 29 bit, no baudrate switch | Bits: FDF + IDE |
| `dxxxxxxxxx\r` | Open | legacy | Send CAN FD packet, 11 bit, no baudrate switch | Bits: FDF |
| `Bxxxxxxxxx\r` | Open | legacy | Send CAN FD packet, 29 bit with baudrate switch | Bits: FDF + BRS + IDE |
| `bxxxxxxxxx\r` | Open | legacy | Send CAN FD packet, 11 bit with baudrate switch | Bits: FDF + BRS |

### Slcan Responses and Events

**Feedback**

| Feedback | Version | Meaning | Comment |
|----------|---------|---------|---------|
| `#\r` | 100 | The command has executed successfully | Requires Feedback mode to be enabled |
| `#1\r` | 100 | The command is invalid | Requires Feedback mode to be enabled |
| `#2\r` | 100 | A parameter of the command is invalid | Requires Feedback mode to be enabled |
| `#3\r` | 100 | The command reqires the adapter to be open | Requires Feedback mode to be enabled |
| `#4\r` | 100 | The command reqires the adapter to be closed | Requires Feedback mode to be enabled |
| `#5\r` | 100 | The HAL from ST Microelectronics reported an error | Requires Feedback mode to be enabled |
| `#6\r` | 100 | The feature is not supported by the board / not implemented | Requires Feedback mode to be enabled |
| `#7\r` | 100 | The CAN Tx Buffer is full (no ACK received, 67 packets waiting) | Requires Feedback mode to be enabled |
| `#8\r` | 100 | CAN bus is off (a severe CAN error has occurred) | Requires Feedback mode to be enabled |
| `#9\r` | 100 | Sending packets is not possible in silent mode | Requires Feedback mode to be enabled |
| `#:\r` | 100 | The required baudrate has not been set | Requires Feedback mode to be enabled |
| `#;\r` | 100 | Programming the Option Bytes in flash memory failed | Requires Feedback mode to be enabled |
| `#<\r` | 100 | Please reconnect the USB cable, a hardware reset is required | Requires Feedback mode to be enabled |

**Response**

| Response | Version | Meaning | Comment |
|----------|---------|---------|---------|
| `+Text\r` | 100 | The firmware sends a text response to a command | Used by commands `*Boot0:?\r` and `V\r` |

**Event**

| Event | Version | Meaning | Comment |
|-------|---------|---------|---------|
| `>Message\r` | 100 | The firmware sends a debug message (plain text) | Requires Debug Messages to be enabled |
| `Exxxxxxxx\r` | 100 | The firmware reports the CAN Error Status (See [Slcan Errors](#Slcan_Errors)) | Requires CAN Error Reports to be enabled |
| `L27\r` | 100 | The firmware has calculated a bus load of 27%.<br>If the bus load is zero, no report is sent. | Requires Bus Load Reports to be enabled |
| `M3C\r` | 100 | The firmware reports the Tx echo marker 0x3C (See [Slcan Packets](#Slcan_Packets)) | Requires Tx Echo Report markers to be enabled |

**Rx Packets**

| Event | Version | Meaning | Comment |
|-------|---------|---------|---------|
| `Txxxxxxxxx\r` | legacy | Received classic packet with 29 bit ID | Bits: IDE (See [Slcan Packets](#Slcan_Packets)) |
| `txxxxxxxxx\r` | legacy | Received classic packet with 11 bit ID | Bits: None |
| `Rxxxxxxxxx\r` | legacy | Received Remote Transmission Request with 29 bit ID | Bits: RTR + IDE |
| `rxxxxxxxxx\r` | legacy | Received Remote Transmission Request with 11 bit ID | Bits: RTR |
| `Dxxxxxxxxx\r` | legacy | Received CAN FD packet, 29 bit, no baudrate switch | Bits: FDF + IDE |
| `dxxxxxxxxx\r` | legacy | Received CAN FD packet, 11 bit, no baudrate switch | Bits: FDF |
| `Bxxxxxxxxx\r` | legacy | Received CAN FD packet, 29 bit with baudrate switch | Bits: FDF + BRS + IDE |
| `bxxxxxxxxx\r` | legacy | Received CAN FD packet, 11 bit with baudrate switch | Bits: FDF + BRS |

### Slcan Version Info

In the new firmware the command `V\r` returns one string with **seven key/value pairs** separatad by **tab characters**.

It starts with a **plus character** that indicates a command response with text content, rather than a feedback response (#).

```
+Board: MksMakerbase
MCU: STM32G431
DevID: 1128
Firmware: 2427156
Slcan: 100
Clock: 160
Limits: 512,256,128,128,32,32,16,16\r
```

Split this string at each tab character ('\t') and then each part at the colon. You get 7 key/value pairs:

| Key | Value | Comment |
|-----|-------|---------|
| Board | MksMakerbase | This defines for which board the firmware was compiled.<br>It may not match the real hardware if the user has uploaded the wrong firmware.<br>See [Firmware Versions](#FirmwareVersions) |
| MCU | STM32G431 | This defines for which processor the firmware was compiled.<br>It may not match the real processor if the user has uploaded the wrong firmware.<br>See [Firmware Versions](#FirmwareVersions) |
| DevID | 1128 | This is the "Device ID" of the real processor. Convert 1128 to hex and you get 0x468.<br>Each **processor serie** is identified by a unique "Device ID".<br>In the [Reference Manual](https://www.st.com/resource/en/reference_manual/dm00355726.pdf) from ST Microelectronics you find these identifiers:<br>0x468 = Category 2 devices (STM32G431, STM32G441)<br>0x469 = Category 3 devices (STM32G471, STM32G473, STM32G474, STM32G483, STM32G484)<br>0x479 = Category 4 devices (STM32G491, STM32G4A1) |
| Firmware | 2427156 | The legacy firmware has returned completely meaningless version numbers like "ba6b1dd".<br>Here you get a **version** number that indicates **the date** when the firmware was created.<br>Convert to hex and you get the BCD version 0x250914, which means 14th september of 2025. |
| Slcan | 100 | This is the **version** of the Slcan module that you see in the tables above in column 'Version'.<br>The first version created by ElmüSoft is version 100 which contains all the features described here.<br>Future versions will be 101, 102, etc and contain new commands which will be documented here.<br>Use this version number to detect which features are supported by the firmware. |
| Clock | 160 | The CAN clock is 160 MHz |
| Limits | 512,256,128,128,<br>32,32,16,16 | These 8 values are the **upper limits** for the bitrate commands 's' and 'y'.<br>Each processor has different limits for Prescaler, Segment1, Segment2 and Synchr. Jump Width.<br>Nominal baudrate: max Prescaler= 512, max Seg1 = 256, max Seg2= 128, max SJW = 128<br>FD Data baudrate: max Prescaler= 32, max Seg1 = 32, max Seg2= 16, max SJW = 16<br>See [Samplepoint](#Samplepoint) |

### Slcan Packets

All packets are transmitted using this pattern:

- **Packet type**: T, t, R, r, D, d, B or b (see tables above)
- **CAN ID**: 3 digits for 11 bit or 8 digits for 29 bit CAN ID
- **DLC**: 1 digit (count of data bytes)
  - Classic packets: DLC = '0' to '8'
  - CAN FD: '8'= 8 byte, '9'= 12 bytes, 'A'= 16 bytes, 'B'= 20 byte, 'C'= 24 bytes, 'D'= 32 bytes, 'E'= 48 byte, 'F'= 64 byte.
- **Data bytes**: Classic packets: 0 to 8, CAN FD: 8 to 64 bytes.
- only for Tx packets and only if Tx **Echo Markers** are enabled: a 2 digit marker.
- only for Rx packets and only if **ESI** reporting is enabled: a character 'S' if the sending node is error passive.

| Direction | Packet | Slcan | Comment |
|-----------|--------|-------|---------|
| Both | 18AABBCC: 11 22 33 44 55 | `T18AABBCC51122334455\r` | Classic, 29 bit ID |
| Both | 7E0: 11 22 33 44 55 66 77 88 99 AA BB CC | `d7E09112233445566778899AABBCC\r` | CAN FD, 11 bit, FDF |
| Receive | 7E0: 11 22 33 44 55 66 77 88 | `b7E081122334455667788S\r` | FDF, BRS, ESI |
| Transmit | 7E0: 11 22 33 44 55 66 | `t7E061122334455663F\r` | Marker = 3F |

**Example:**

```
Tx: "t7E061122334455663F\r" <—— send packet with marker 3F
Rx: "#\r"                   <—— receive command feedback (Success)
Rx: "M3F\r"                 <—— receive echo marker 3F
```

### Slcan Error Reports

When CAN Error Reports are enabled you get an error report only if at least one error is present.

Errors are sent immediately, after 100 ms or after 3 seconds as explained [above](#CAN_Errors).

An error report is always of the form `Exxxxxxxx\r` and consists of 5 parts:

| Digit | Meaning | Value |
|-------|---------|-------|
| 1 | Bus Status | '0' = Bus Active<br>'1' = Warning Level<br>'2' = Bus Passive<br>'3' = Bus Off |
| 2 | Last Protocol Error | '0' = None<br>'1' = Bit stuffing error<br>'2' = Frame format error<br>'3' = No ACK received<br>'4' = Recessive bit error<br>'5' = Dominant bit error<br>'6' = CRC error |
| 3 + 4 | Firmware Error Flags | 0x00 = None<br>0x01 = Rx Failed<br>0x02 = Tx Failed<br>0x04 = CAN Tx buffer overflow<br>0x08 = USB IN buffer overflow<br>0x10 = Tx Timeout<br>Multiple flags may be combined |
| 5 + 6 | Tx Error Count | 0 ... 255 errors |
| 7 + 8 | Rx Error Count | 0 ... 255 errors |

**Example:** `E2310800C\r` means:

Bus is passive, No ACK received, Tx Timeout, 128 Tx Errors, 12 Rx Errors

### Slcan Initialization

When connecting to a CANable 2.5 adapter you should follow this sequence:

1. Send command `C\r` to **close the device** if it is till open.

   But this command has also another purpose: It **resets all internal variables** in the firmware to their default.

   Baudrates and all modes are reset (back to legacy), reports turned off, filters removed, Rx/Tx buffers are emptied.

   **ATTENTION:** The command 'Close' is the only command that does **not send a feedback**, although feedbacks are enabled.

   The purpose is that you can always send this as the first command and don't have to wait for a feedback.

   When you connect to a CANable you don't know if the user has connected a legacy device or a CANable 2.5

   Additionally the firmware cannot know at this moment if you will enable feedback mode later or not.

   Therefore the command 'Close' behaves exactly as the legacy command: It never sends a feedback.

2. Send command `V\r` to get the [Version String](#Slcan_Version).

   Split the response string at the **tab characters**.

   When you get **7 parts** or more you are communicating with a CANable 2.5, otherwise it is a legacy firmware.

   If you don't support legacy devices show an error message, otherwise switch to legacy mode.

   If it is a CANable 2.5 use the **Slcan Version Number** (100 or higher) to check which features are available.

3. Send command `MF\r` to enable **feedback mode** (or `MADEFMS\r` with all the other modes that you need)

   This will be the first command that sends a feedback.

   From now on you must wait for the feedback `#\r` and check for errors before sending the next command.

4. Send command 'S' or 's' to set the **nominal** [baudrate](#Baudrates).

5. If you also send command 'Y' or 'y' to set the **data baudrate**, the firmware will switch automatically into CAN FD mode.

6. Now you can optionally set [filters](#Filter)

7. Finally execute command `ON\r` to **open** the device in normal mode.

---

Here you see the initialization sequence sent manually to the adapter.

Use the **RS232 Terminal** in HUD ECU Hacker.

The **baudrate** that you enter to open the COM port is irrelevant.

The port runs always with maximum USB speed (12 MBit).
