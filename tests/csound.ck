SawOsc s => ReverbSC r => Csound c => dac;
0.5 => s.gain;
0.93 => r.size;

c.compile("test.csd");

string message;
float freq;


while(1) {
"i1 0 1 " => message;
Std.rand2(100, 1000) => freq;
freq +=> message;
freq => s.freq;
c.inputMessage(message);
1::second => now;
}
