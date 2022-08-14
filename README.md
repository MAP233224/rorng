# rorng

***This repo is currently a work in progress.***  

## What is this?
A program that generates numbers that contain all the values for a specified number of bits when you rotate it (usually with ROR) and AND its lowest bits.  
Eg: ``0xACDC1F48`` (or ``0b10101100110111000001111101001000``), a 32-bit number, contains every 5-bit value when you just look at the 5 least significant bits and keep rotating it.

A side effect of this idea is that it work as a very fast PRNG (if you make sure that it can go through the whole period).

```c

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

int rorng(int state) {
  /* 5-bit prng, this should just compile to a ROR and AND instruction */
  return ROR(0xACDC1F48, state) & 31;
}
```

**When you want an N-bit period, you will need a 2^N-bit number.**

Of course you cannot use the ROR instruction as your values get larger, but that's still the general idea (I'm trying to find a method that would achieve the same result optimally).  

...
