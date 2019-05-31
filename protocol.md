Binary Protocol between sensor and gateway
==========================================

When the sensor wakes up, it sends one packet to the gateway (with
`ADDR="000001"`). The packet has 21 bytes, as follows:

Byte  | Item        | Description                               |
------|-------------|-------------------------------------------|
0     | Device Type | Value=`0x01`                              |
1-6   | Unique ID   | 6-bytes unique ID read from SHA204        |
7- 8  | Battery     | Battery level, in ADC counts, as a short  |
9-12  | Temperature | Temperature, in deg C, as a float         |
13-16 | Humidity    | Relative humidity, in %, as a float       |
17-20 | Pressure    | Pressure, in hPa, as a float              |

