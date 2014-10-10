<CsoundSynthesizer>
<CsOptions>
;-d -odac:system:playback_ -+rtaudio="jack" 
-d -onull -+rtaudio=null 
-B 1
-b 1
</CsOptions>
<CsInstruments>

sr	=	44100
ksmps	=	1
nchnls	=	1
0dbfs	=	1

instr 1	
aout = \
moogvcf(vco2(0.1, p4, 0), 1000, 0.1)
outs aout, aout
endin

alwayson 2
instr 2
a1 chnget "Chuck_Out"
a1 = \
waveset(moogvcf(a1, 1000, 0.1), 5)
out a1
endin

</CsInstruments>
<CsScore>
f 0 $INF
;i1 0 100 440
</CsScore>
</CsoundSynthesizer>

