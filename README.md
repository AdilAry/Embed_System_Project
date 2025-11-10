### Commit and Merge rules;
Every team member, before commiting and pushing into the main branch, must first commit to the "dev" branch and make a pull request for a merge with the main branch.

---
### Project specification;
---
#### Battery details:
**4 cell Lithium Iron Phosphate battery.**

Cell voltage: 3.2 V

Cell capacity: 105 Ah

Charge voltage maximum: 3.65 V

Discharge cutoff voltage: 2.5 V

---
Battery full specifications can be found here: https://www.18650batterystore.com/products/eve-lf105

---
#### Requirements;
- Temperature monitoring (via two sensors).
- Individual battery cell voltages measurement.
- MOSFET driven switch (both charging and discharging).
- Current measurement, including over current protection (10A).
- STM32 controller with over/under voltage per cell (2.5V min, 3.65V max. per cell), over/under temperature (charge: 0C to 40C and discharge: -20C to 40C).
- Plugin design to connect a NUCLEO-L152RE board to the BMS circuit.

---
### Platform/software choices;
Version Control - Git, GitHub.

CAD - KiCAD.

Circuit Simulation - LTspice.