# SLCAN Terminal

An interactive terminal application for sending and receiving raw SLCAN commands over a serial port. This is a C++ version based on `slcan_attach.c` that provides a direct command-line interface instead of creating a CAN network interface.

I created this to work with CANable 2.0 adapters with new firmware found here: https://github.com/Elmue/CANable-2.5-firmware-Slcan-and-Candlelight

## Features

- Interactive terminal interface for SLCAN communication
- Automatic device detection (searches `/dev/serial/by-id/` for devices with "slcan" in the name)
- Initialization commands for automatic device configuration
- Real-time display of received data
- Separate transmit (TX) and receive (RX) indicators
- **Automatic feedback code interpretation** - displays human-readable descriptions for SLCAN feedback codes (#, #1-#9, #:, #;, #<)
- **Error code interpretation** - automatically decodes detailed error reports (Exxxxxxxx format) with bus status, protocol errors, and error counts
- **cansend-like syntax** - use `<can_id>#<data>` format, automatically converted to SLCAN (like can-utils cansend)
- Support for all standard SLCAN commands
- Line editing with backspace support
- Graceful exit with 'quit', 'exit', or Ctrl+C

## Building

### Requirements

- CMake 3.10 or higher
- C++11 compatible compiler (g++, clang++)
- pthread library

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

For a release build with optimizations:

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Installation

```bash
sudo make install
```

This will install the binary to `/usr/local/bin/slcan_terminal`.

## Usage

```bash
./slcan_terminal [options] [tty_device]
```

### Options

- `-h, --help` - Show help message
- `-i, --init <commands>` - Send initialization commands (comma-separated)

**Note:** If no `tty_device` is specified, the tool will automatically search for the first device in `/dev` containing "slcan" in its name.

### Examples

Auto-detect SLCAN device:
```bash
./slcan_terminal
```

Auto-detect with initialization commands:
```bash
./slcan_terminal -i "C,S6,O"
```

Specify device explicitly:
```bash
./slcan_terminal /dev/ttyUSB0
```

With initialization commands (close, set speed to 500k, open):
```bash
./slcan_terminal -i "C,S6,O" /dev/ttyUSB0
```

With version check and configuration:
```bash
./slcan_terminal --init "C,V,S6,ON" /dev/ttyS1
```

### Terminal Commands

Once in the terminal, you can enter SLCAN commands directly. Common commands include:

#### Configuration Commands

- `V` - Get version and serial number
- `S0` to `S8` - Set CAN speed:
  - `S0` = 10 Kbit/s
  - `S1` = 20 Kbit/s
  - `S2` = 50 Kbit/s
  - `S3` = 100 Kbit/s
  - `S4` = 125 Kbit/s
  - `S5` = 250 Kbit/s
  - `S6` = 500 Kbit/s
  - `S7` = 800 Kbit/s
  - `S8` = 1000 Kbit/s

#### Channel Control

- `O` - Open channel (normal mode)
- `ON` - Open channel (normal mode, SLCAN 2.5)
- `OS` - Open channel (silent mode)
- `L` - Open channel (listen-only mode)
- `C` - Close channel
- `F` - Read status flags

#### CAN Frame Transmission

- `tiiildd...` - Transmit standard CAN frame
  - `iii` = 3-digit hex CAN ID
  - `l` = data length (0-8)
  - `dd` = data bytes in hex

  Example: `t12348AABBCCDDEE1122` (ID=0x123, 8 bytes of data)

- `Tiiiiiiiildd...` - Transmit extended CAN frame
  - `iiiiiiii` = 8-digit hex CAN ID
  - `l` = data length (0-8)
  - `dd` = data bytes in hex

- `riiil` - Transmit standard RTR frame
- `Riiiiiiiil` - Transmit extended RTR frame

#### Simplified CAN Frame Format

The terminal supports a simpler format with automatic DLC calculation:

**Format:** `<packet_type><can_id>#<data>`

**Packet Types:**
- `t` - Standard (11-bit) CAN frame
- `T` - Extended (29-bit) CAN frame
- `r` - Standard RTR frame
- `R` - Extended RTR frame
- `d` - Standard CAN FD frame (FDF, no BRS)
- `D` - Extended CAN FD frame (FDF, no BRS)
- `b` - Standard CAN FD frame (FDF + BRS)
- `B` - Extended CAN FD frame (FDF + BRS)

**Features:**
- Data can be hex bytes optionally separated by dots (`.`) or spaces
- DLC is automatically calculated from data length
- Standard IDs are zero-padded to 3 hex digits
- Extended IDs are zero-padded to 8 hex digits

**Examples:**

```bash
# Standard CAN frame
> t123#DEADBEEF
[TX] t12304DEADBEEF

# With dots separating bytes
> t7E0#11.22.33.44
[TX] t7E00411223344

# Extended CAN frame
> T18AABBCC#112233
[TX] T18AABBCC03112233

# RTR frame (no data)
> r123#
[TX] r1230

# CAN FD with BRS (12 bytes)
> b123#112233445566778899AABBCC
[TX] b1239112233445566778899AABBCC
```

### SLCAN Feedback Codes

When feedback mode is enabled (with `MF` command), the adapter sends feedback codes to indicate command status. The terminal automatically interprets these codes:

| Code | Description |
|------|-------------|
| `#` | Success - command executed successfully |
| `#1` | Invalid command |
| `#2` | Invalid parameter |
| `#3` | Adapter must be open |
| `#4` | Adapter must be closed |
| `#5` | HAL error from ST Microelectronics |
| `#6` | Feature not supported/implemented |
| `#7` | CAN Tx buffer full (no ACK, 67 packets waiting) |
| `#8` | CAN bus off (severe error occurred) |
| `#9` | Sending not possible in silent mode |
| `#:` | Baudrate not set |
| `#;` | Flash Option Bytes programming failed |
| `#<` | Hardware reset required - reconnect USB |

Example with feedback enabled:
```bash
> MF
[RX] # (Success)
> O
[RX] # (Success)
> InvalidCmd
[RX] #1 (Invalid command)
```

### Error Code Interpretation

The terminal automatically decodes detailed error reports sent by the adapter in response to the `F` (status flags) command or error conditions. Error reports use the format `Exxxxxxxx` (E + 8 hex digits):

**Format:** `Exxxxxxxx`

| Position | Description | Values |
|----------|-------------|--------|
| Digit 1 | Bus Status | 0=Active, 1=Warning Level, 2=Passive, 3=Bus Off |
| Digit 2 | Last Protocol Error | 0=None, 1=Bit stuffing, 2=Frame format, 3=No ACK, 4=Recessive bit, 5=Dominant bit, 6=CRC |
| Digits 3-4 | Firmware Error Flags (hex) | 0x01=Rx Failed, 0x02=Tx Failed, 0x04=CAN Tx overflow, 0x08=USB IN overflow, 0x10=Tx Timeout |
| Digits 5-6 | Tx Error Count (hex) | 0x00-0xFF |
| Digits 7-8 | Rx Error Count (hex) | 0x00-0xFF |

Example error report:
```bash
> F
[RX] E03000505 (Bus Active, No ACK received, Tx Errors: 5, Rx Errors: 5)
```

### Example Session

Basic session without initialization:
```
./slcan_terminal -i C,MF,MD,ME,MS,s\"1,239,80,80\",y\"4,10,9,9\",V,ON /dev/ttyACM0
command string: C,MF,MD,ME,MS,s"1,239,80,80",y"4,10,9,9",V,ON

=== Sending initialization commands ===
[INIT] C
[INIT] MF
[RESP] #
[INIT] MD
[RESP] #
[INIT] ME
[RESP] #
[INIT] MS
[RESP] #
[INIT] s1,239,80,80
[RESP] #
[INIT] y4,10,9,9
[RESP] #
[INIT] V
[RESP] +Board: MksMakerbase	MCU: STM32G431	DevID: 1128	Firmware: 2428963	Slcan: 100	Clock: 160	Limits: 512,256,128,128,32,32,16,16
[INIT] ON
[RESP] #
[RESP] >Nominal: 500k baud, 75.0%; Data: 2M baud, 55.0%; Perfect match: No
=== Initialization complete ===


=== SLCAN Terminal ===
Connected to: /dev/ttyACM0
Commands: Enter SLCAN commands (e.g., 'V' for version, 'O' to open)
Special: 'quit' or 'exit' to close, Ctrl+C to abort
======================

[RX] b01223456
[RX] b023A8877665544332211AABBCCDDEEFF0000
> C  
[TX] C
> quit

Terminal closed.
```

## Differences from slcan_attach.c

The original `slcan_attach.c` creates a network CAN interface using the Linux SLCAN line discipline. This C++ version instead:

- Provides an interactive terminal interface
- Allows manual entry of SLCAN commands
- Displays both transmitted and received data in real-time
- Does not create a network interface
- Useful for debugging, testing, and direct SLCAN device interaction


## License

Based on slcan_attach.c:
- SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
