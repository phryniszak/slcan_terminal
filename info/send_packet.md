### Slcan Packets

All packets are transmitted using this pattern:

- **Packet type**: T, t, R, r, D, d, B or b (see tables above)
- **CAN ID**: 3 digits for 11 bit or 8 digits for 29 bit CAN ID
- **DLC**: 1 digit (count of data bytes)
  - Classic packets: DLC = '0' to '8'
  - CAN FD: '8'= 8 byte, '9'= 12 bytes, 'A'= 16 bytes, 'B'= 20 byte, 'C'= 24 bytes, 'D'= 32 bytes, 'E'= 48 byte, 'F'= 64 byte
- **Data bytes**: Classic packets: 0 to 8, CAN FD: 8 to 64 bytes
- only for Tx packets and only if Tx **Echo Markers** are enabled: a 2 digit marker
- only for Rx packets and only if **ESI** reporting is enabled: a character 'S' if the sending node is error passive

| Direction | Packet | Slcan | Comment |
|-----------|--------|-------|---------|
| Both | 18AABBCC: 11 22 33 44 55 | "**T**18AABBCC**5**1122334455\r" | Classic, 29 bit ID |
| Both | 7E0: 11 22 33 44 55 66 77 88 99 AA BB CC | "**d**7E0**9**112233445566778899AABBCC\r" | CAN FD, 11 bit, FDF |
| Receive | 7E0: 11 22 33 44 55 66 77 88 | "**b**7E0**8**1122334455667788**S**\r" | FDF, BRS, **ESI** |
| Transmit | 7E0: 11 22 33 44 55 66 | "**t**7E0**6**112233445566**3F**\r" | Marker = **3F** |

**Example:**

```
Tx: "t7E061122334455663F\r" <—— send packet with marker 3F
Rx: "#\r"                   <—— receive command feedback (Success)
Rx: "M3F\r"                 <—— receive echo marker 3F
```

| Transmit Packets | Condition | Version | Meaning | Comment |
|------------------|-----------|---------|---------|---------|
| "Txxxxxxxxx\r" | Open | legacy | Send classic packet with 29 bit ID | Bits: IDE  See [Slcan Packets](#slcan_packets) |
| "txxxxxxxxx\r" | Open | legacy | Send classic packet with 11 bit ID | Bits: None |
| "Rxxxxxxxxx\r" | Open | legacy | Send Remote Transmission Request with 29 bit ID | Bits: RTR + IDE |
| "rxxxxxxxxx\r" | Open | legacy | Send Remote Transmission Request with 11 bit ID | Bits: RTR |
| "Dxxxxxxxxx\r" | Open | legacy | Send CAN FD packet, 29 bit, no baudrate switch | Bits: FDF + IDE |
| "dxxxxxxxxx\r" | Open | legacy | Send CAN FD packet, 11 bit, no baudrate switch | Bits: FDF |
| "Bxxxxxxxxx\r" | Open | legacy | Send CAN FD packet, 29 bit with baudrate switch | Bits: FDF + BRS + IDE |
| "bxxxxxxxxx\r" | Open | legacy | Send CAN FD packet, 11 bit with baudrate switch | Bits: FDF + BRS |