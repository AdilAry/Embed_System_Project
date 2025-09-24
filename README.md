### Team members;
- Adil Arystanbek, e2202683

- Ganeshram Manisekaran Sujatha, e2203643

---
### Commit and Merge rules;
Every team member, before commiting and pushing into the main branch, must first commit to the "dev" branch and make a pull request for a merge with the main branch.

---
### Project specification;
---
#### Battery details:
4 cell Lithium Iron Phosphate battery.

Cell voltage: 3.2 V

Cell capacity: 105 Ah

Energy storage: 336 Wh

Charge voltage maximum: 3.65 V

Discharge cutoff voltage: 2.5 V

---
#### Requirements;
- Temperature monitoring (via two sensors).
- Individual battery cell voltages measurement.
- MOSFET driven switch (both charging and discharging).
- Current measurement, including over current protection (10A).
- STM32 controller with over/under voltage per cell (2.5V min, 3.65V max. per cell), over/under temperature (charge: 0C to 40C and discharge: -20C to 40C).
- Plugin design to connect a NUCLEO-L15RE board to the BMS circuit.

"No need to worry about casing"

"No need to worry about cell overcharge balancing(this far at least)"

---
### Platform/software choices;
Version Control - Git, GitHub.

CAD - KiCAD.

Circuit Simulation - LTspice.