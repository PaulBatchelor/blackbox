SawOsc s => LPF l => ReverbSC r => Csound c => dac;

0.93 => r.size;
c.compile("waveset.csd");
l.set(1000, 0.1);

string message;
float freq;

while(1) {
    Std.rand2(50, 1000) => s.freq;
    500::ms => now;
}
