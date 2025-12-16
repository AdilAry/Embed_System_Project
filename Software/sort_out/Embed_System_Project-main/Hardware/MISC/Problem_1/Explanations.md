Our Vin = 2.5 - 3.5V(from the microcontroller), Vcc = 40V, Vout = 25 - 35V.

At it's most basic what an op amp does is explained by a formula Vout = A(V+ - V-) where (V+ - V-) is the Vin, where A is the amplification gain which is ideally is assumed to be infinite, which it technically isn't, but is assumed to be as such(i don't know why).
Despite knowing why it is assumed that way, one thing I can say because of that assumption is that logically, if the output of the op amp is finite, then the input is logically 0, since the input is derived from (V+ - V-) then we can easily that V+ = V-, which we can easily see in my LTspice screenshot.

Another thing that is infinite is the resistance, so we can say that Vin = Vout * R1/(R1+R2) (voltage divider), and solving for Vout it will be Vout = Vin * (R1+R2)/R1, 
now solving for gain, A = Vout / Vin = (Vin * (R1+R2)/R1) / Vin = R1+R2/R1
And this is the logic behind the resistances. 
In short, how an op amp can be explained(at least once the feedback is in) is that the output of the op amp will be of whatever the value it needs to be(to the maximum of the supplied voltage obviously) to make the feedback equal to the other input. 

---
The NPN transistor acts as a pass through, the op amp connected to the base of it will decide what's the amount from the Vcc that can pass through the Emitter which our Output.
I read this from the datasheet, but not really sure on somedetails that I will probably clarify during the lesson.

---
Why no negative voltage supply, because we are not dealing with the Vout range that has a negative voltage.

---
Material used:
The datasheet for the LM317 precision voltage regulator, specifically chapter 8where there is a block diagram of the ic.
+some exercises that I did before, which is the "Component_3" file and from which I got to know precision voltage regulators.	