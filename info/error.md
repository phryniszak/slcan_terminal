Exxxxxxxx\r" | 100 | The firmware reports the CAN Error Status (See Slcan Errors) | Requires CAN Error Reports to be enabled

### Slcan Error Reports {#Slcan_Errors}

When CAN Error Reports are enabled you get an error report only if at least one error is present.

Errors are sent immediately, after 100 ms or after 3 seconds as explained [above](#CAN_Errors).

An error report is always of the form "Exxxxxxxx\r" and consists of 5 parts:

| Digit | Meaning | Value |
|-------|---------|-------|
| 1 | **Bus Status** | '0' = Bus Active<br>'1' = Warning Level<br>'2' = Bus Passive<br>'3' = Bus Off |
| 2 | **Last Protocol Error** | '0' = None<br>'1' = Bit stuffing error<br>'2' = Frame format error<br>'3' = No ACK received<br>'4' = Recessive bit error<br>'5' = Dominant bit error<br>'6' = CRC error |
| 3 + 4 | **Firmware Error Flags** | 0x00 = None<br>0x01 = Rx Failed<br>0x02 = Tx Failed<br>0x04 = CAN Tx buffer overflow<br>0x08 = USB IN buffer overflow<br>0x10 = Tx Timeout<br>Multiple flags may be combined |
| 5 + 6 | **Tx Error Count** | 0 ... 255 errors |
| 7 + 8 | **Rx Error Count** | 0 ... 255 errors |

**Example:** "E**2****3****10****80****0C**\r" means:
Bus is passive, No ACK received, Tx Timeout, 128 Tx Errors, 12 Rx Errors

