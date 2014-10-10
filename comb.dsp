import("filter.lib");
b1 = hslider("b1", 0, -1, 1, 0.01) : smooth(0.999);
delayLength = hslider("delLength", 1, 1, 10, 0.01) * 0.001 * SR;
feedbk = hslider("feedbk", 0, 0, 1, 0.001); 
filter = _ <: _ + (_@delayLength : *(b1)) : *(0.5);
gate = checkbox("gate");
process = (noise * gate) : filter;
