Binary Protocol between sensor and gateway
==========================================

When the sensor wakes up, it sends one packet to the gateway (with
`ADDR="000001"`). The packet has 23 bytes, as follows:

Byte  | Description | Value                                |
------|-------------|--------------------------------------|
0     | Device Type | 0x01                                 |
1-6   | Unique ID   | 6-bytes unique ID read from SHA204   |
7-10  | Battery     | Battery level, in volts, as a float  |
11-14 | Temperature | Temperature, in deg C, as a float    |
15-18 | Humidity    | Relative humidity, in %, as a float  |
19-22 | Pressure    | Pressure, in hPa, as a float         |

