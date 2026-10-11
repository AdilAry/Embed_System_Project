Some stuff might be wrong, specifically everything that relates to accurate number, be that:
- Margin measurements for footprints(Downloading CAD files from Mouser didn't work).
- Morpho headers.

---
### Some notes;
Two voltage deviders near the Intrumentation Amplifier are to tackle the problem of us needing to measure current bi-directionally, one of them we alreaddy discussed, it's the virtual ground, the other however is for the reference voltage. Since we are measuring bi-directionally and there isn't such a thing we can pass to our ADC as negative voltage our reference point must be higher than 0, so that the -10A will be in the spot 0(in this case the reference is 1.65V since our max is 3.3V).

Many of the component symbols look strange in the schematic, it's because I wasn't able to get them from Mouser so I just re-used the default ones by simply changing their pin assigment.

I was reading through the forums and found a Bus pre-charger circuit, I remember there was a task to like figure out a circuit that protects the BMS from overcurrent before the fuse and I wonder if this bus precharge circuit is related to that.

I spent some time picking cheaper components, and I was wrong. It's not the same price as the PCB, quite decently cheaper so.

There were quite some IC's I found https://www.ti.com/product-category/battery-management-ics/overview.html , but at the end I decided to go with the normal by component implentation for learning sake and since most of our work/study has been on these.

One mistake that I can already see is the MOSFET gate drive voltage.

---
No 90 degree angles,
1 mm for high power tracks, 0.5/0.4(in-case of clearence issues) for low-side.

DRC is ok. Remaining DRC warnings come from the connector footprint and don’t affect the design