<CsoundSynthesizer>
<CsOptions>
;disable audio output and let ChucK handle it all
-d -onull -+rtaudio=null 
</CsOptions>
<CsInstruments>

sr	=	44100
ksmps	=	1
nchnls	=	1
0dbfs	=	1

instr 1	
aout = \
pluck(0.1, p4, p4, 0, 1) * linseg(1, p3, 0)
out aout
endin

</CsInstruments>
<CsScore>
f 0 $INF
</CsScore>
</CsoundSynthesizer>

