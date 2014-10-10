<CsoundSynthesizer>
<CsOptions>
-d -onull -+rtaudio=null 
-b 1
</CsOptions>
<CsInstruments>

sr	=	44100
ksmps	=	1
nchnls	=	1
0dbfs	=	1

alwayson 2
instr 2
a1 chnget "Chuck_Out"
out waveset(a1, 5) * 0.5
endin

</CsInstruments>
<CsScore>
f 0 $INF
</CsScore>
</CsoundSynthesizer>

