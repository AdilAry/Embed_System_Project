3 mistakes total.
At least the PCB plugs into the nucleo board perfectly.

---
### Temperature measurement / Design mistakes;
While the component itself works, the circuit accompanying it doesn't.
And it doesn't work for a pretty nonsensical reason as well.

#### Future Fix;
I have simply fixed that part of a circuit, which can be found on the presentation.

---
### Current measurement / Shunt soldering issues;
The shunt resistor couldn't be consistently soldered due to a footprint of a wrong size, so it was soldered on two wires near, it was flimsy and broke off, so it was re-soldered again and again, eventually the shunt got burnt.

#### Future Fix;
I made a custom footprint for the shunt that actually makes it fit, which also can be found on the presentation.

---
### Voltage measurement / MUX soldering issues;
This one is of a simmiliar nature as the shunt mistake, although to a worse degree. As the problem wasn't on the pcb design per se, but more like with the component being ordered. The chosen multiplexer was extremely small, smaller than some of the resistor and more like a small capacitor in size, it was too small to solder by hand, even by printing another PCB as another did is just not doable as the printer and etching was problematic even for their case scenario(of a typically normal MOSFET), for a component case like ours this will not be feasable.

#### Future Fix;
The only fix possible is to find a new multiplexer, which I did and it can be seen in the presentation.