Scraps of code I have obtained from [here](https://sites.google.com/site/patrickmaillot/arduino#h.p_DHBA4DLmHcQD)

```cpp
#define send_udp(message, len) \
  Udp.beginPacket(X32IP, 10023); \
  Udp.write(message, len); \
  Udp.endPacket();
```

Data information

`char s_remt[12] = "/xremote";             // remote buffer`

xremote operation

`send_udp(s_remt, 12);`

Hopefully this is enough for future me to work out getting data from the sound board
