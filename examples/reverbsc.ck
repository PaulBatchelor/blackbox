//Gain g => LPF l => ReverbSC r => ADSR fade => dac;
Gain g => LPF l => ReverbSC r => dac;

0.3 => r.mix;
0.98 => r.size;
15000 => r.cutoff;
1000 => l.freq;
2 => l.Q;
0.8 => r.gain;
0.1 => g.gain;

120 => float totaltime;
now + totaltime::second => time later;
//fade.set(10::ms, 0::ms, 1, 10::second);
fun void note(int n, float t, float del)
{
	SawOsc s => Envelope e => g;
	del::second => now;
	//while(now < later) 
	while(1) 
	{
		1 => e.time;
		Std.mtof(n) => s.freq;
		e.keyOn();
		4::second => now;
		e.keyOff();
		t::second => now;
	}
}

spork ~ note(58, 4, 0);
spork ~ note(65, 5, 3);
spork ~ note(60, 4, 6);
spork ~ note(69, 6, 24);
spork ~ note(72, 6.5, 30);
spork ~ note(74, 7, 70);
spork ~ note(81, 9, 71);
spork ~ note(46, 4, 50);
spork ~ note(34, 9.5, 53);

while(1) 1::second => now;
