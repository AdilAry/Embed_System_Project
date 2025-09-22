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
Current cutoff: 10 A

#### Requirements;
- MOSFET driven switch (both charging and discharging).
- Temperature monitoring (via two sensors).
- Individual battery cell voltages measurement.
- Over current(10 A) and voltage(2.5 - 3.65V) protection.
- Current measurement.
- Plugin design to connect a NUCLEO-L15RE board to the BMS circuit.

"No need to worry about casing"
"No need to worry about cell overcharge balancing(this far at least)"

---
### Platform/software choices;
Version Control - Git, GitHub.

CAD - KiCAD.

Circuit Simulation - LTspice.