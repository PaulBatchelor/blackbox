Csound c => dac;

c.compile("pluck.csd");

string message;
float freq;

while(1) {
"i1 0 3 " => message;
Std.rand2(80, 800) => freq;
freq +=> message;
c.inputMessage(message);
0.5::second => now;
}
