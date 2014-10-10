/*
    reverbsc.c:

    8 delay line FDN reverb, with feedback matrix based upon
    physical modeling scattering junction of 8 lossless waveguides
    of equal characteristic impedance. Based on Julius O. Smith III,
    "A New Approach to Digital Reverberation using Closed Waveguide
    Networks," Proceedings of the International Computer Music
    Conference 1985, p. 47-53 (also available as a seperate
    publication from CCRMA), as well as some more recent papers by
    Smith and others.

    Csound orchestra version coded by Sean Costello, October 1999

    C implementation (C) 2005 Istvan Varga
	
	Chuck Implementation by Paul Batchelor, March 2014
*/
#include "ugen_reverbsc.h"
#include "chuck_type.h"
#include "chuck_ugen.h"
#include "chuck_vm.h"
#include "chuck_globals.h"

#ifndef TWOPI
#define TWOPI   (6.28318530717958647692)
#endif

static t_CKUINT g_srate = 0;

#define BUFFER_SIZE 1024
typedef struct {
int sr;
long c; 
} soundpipe_data;

typedef struct {
int size;
double *arr;
}sp_array;

typedef struct auxch {
	size_t  size;
	void    *auxp;
} AUXCH;

typedef struct{
	double (*func)(void *, double, sp_array);
	void *data;
}sp_fx;

/* Begin port of reverbsc opcode */
//int reverbsc_init(soundpipe_data *data, sp_fx *info, 
//	double kFeedBack, double kLPFreq);
//double reverbsc(void *data, double s, sp_array params);




#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SRATE   44100.0
#define MIN_SRATE       5000.0
#define MAX_SRATE       1000000.0
#define MAX_PITCHMOD    20.0
#define DELAYPOS_SHIFT  28
#define DELAYPOS_SCALE  0x10000000
#define DELAYPOS_MASK   0x0FFFFFFF

///* reverbParams[n][0] = delay time (in seconds)                     */
///* reverbParams[n][1] = random variation in delay time (in seconds) */
///* reverbParams[n][2] = random variation frequency (in 1/sec)       */
///* reverbParams[n][3] = random seed (0 - 32767)                     */
//
static const double reverbParams[8][4] = {
    { (2473.0 / DEFAULT_SRATE), 0.0010, 3.100,  1966.0 },
    { (2767.0 / DEFAULT_SRATE), 0.0011, 3.500, 29491.0 },
    { (3217.0 / DEFAULT_SRATE), 0.0017, 1.110, 22937.0 },
    { (3557.0 / DEFAULT_SRATE), 0.0006, 3.973,  9830.0 },
    { (3907.0 / DEFAULT_SRATE), 0.0010, 2.341, 20643.0 },
    { (4127.0 / DEFAULT_SRATE), 0.0011, 1.897, 22937.0 },
    { (2143.0 / DEFAULT_SRATE), 0.0017, 0.891, 29491.0 },
    { (1933.0 / DEFAULT_SRATE), 0.0006, 3.221, 14417.0 }
};

static const double outputGain  = 0.35;
static const double jpScale     = 0.25;

typedef struct {
    int         writePos;
    int         bufferSize;
    int         readPos;
    int         readPosFrac;
    int         readPosFrac_inc;
    int         dummy;
    int         seedVal;
    int         randLine_cnt;
    double      filterState;
    MYFLT       buf[1];
} delayLine;

typedef struct {
    MYFLT       kFeedBack, kLPFreq;
    MYFLT       iSampleRate, iPitchMod, iSkipInit;
    double      sampleRate;
    double      dampFact;
    MYFLT       prv_LPFreq;
    int         initDone;
    delayLine   *delayLines[8];
    AUXCH       auxData;
} SC_REVERB;

static int delay_line_max_samples(SC_REVERB *p, int n)
{
    double  maxDel;

    maxDel = reverbParams[n][0];
    maxDel += (reverbParams[n][1] * (double) p->iPitchMod * 1.125);
    return (int) (maxDel * p->sampleRate + 16.5);
}

static int delay_line_bytes_alloc(SC_REVERB *p, int n)
{
    int nBytes;

    nBytes = (int) sizeof(delayLine) - (int) sizeof(MYFLT);
    nBytes += (delay_line_max_samples(p, n) * (int) sizeof(MYFLT));
    nBytes = (nBytes + 15) & (~15);
    return nBytes;
}

static void next_random_lineseg(SC_REVERB *p, delayLine *lp, int n)
{
    double  prvDel, nxtDel, phs_incVal;

    /* update random seed */
    if (lp->seedVal < 0)
      lp->seedVal += 0x10000;
    lp->seedVal = (lp->seedVal * 15625 + 1) & 0xFFFF;
    if (lp->seedVal >= 0x8000)
      lp->seedVal -= 0x10000;
    /* length of next segment in samples */
    lp->randLine_cnt = (int) ((p->sampleRate / reverbParams[n][2]) + 0.5);
    prvDel = (double) lp->writePos;
    prvDel -= ((double) lp->readPos
               + ((double) lp->readPosFrac / (double) DELAYPOS_SCALE));
    while (prvDel < 0.0)
      prvDel += (double) lp->bufferSize;
    prvDel = prvDel / p->sampleRate;    /* previous delay time in seconds */
    nxtDel = (double) lp->seedVal * reverbParams[n][1] / 32768.0;
    /* next delay time in seconds */
    nxtDel = reverbParams[n][0] + (nxtDel * (double) p->iPitchMod);
    /* calculate phase increment per sample */
    phs_incVal = (prvDel - nxtDel) / (double) lp->randLine_cnt;
    phs_incVal = phs_incVal * p->sampleRate + 1.0;
    lp->readPosFrac_inc = (int) (phs_incVal * DELAYPOS_SCALE + 0.5);
}

static void init_del_line(SC_REVERB *p, delayLine *lp, int n)
{
    double  readPos;
    /* int     i; */

    /* calculate length of delay line */
    lp->bufferSize = delay_line_max_samples(p, n);
    lp->dummy = 0;
    lp->writePos = 0;
    /* set random seed */
    lp->seedVal = (int) (reverbParams[n][3] + 0.5);
    /* set initial delay time */
    readPos = (double) lp->seedVal * reverbParams[n][1] / 32768;
    readPos = reverbParams[n][0] + (readPos * (double) p->iPitchMod);
    readPos = (double) lp->bufferSize - (readPos * p->sampleRate);
    lp->readPos = (int) readPos;
    readPos = (readPos - (double) lp->readPos) * (double) DELAYPOS_SCALE;
    lp->readPosFrac = (int) (readPos + 0.5);
    /* initialise first random line segment */
    next_random_lineseg(p, lp, n);
    /* clear delay line to zero */
    lp->filterState = 0.0;
   	memset(lp->buf, 0, sizeof(MYFLT)*lp->bufferSize);
    //lp->buf = malloc(sizeof(MYFLT) * lp->bufferSize);
    /* for (i = 0; i < lp->bufferSize; i++) */
    /*   lp->buf[i] = FL(0.0); */
}

//static int sc_reverb_init(CSOUND *csound, SC_REVERB *p)
int reverbsc_init(SC_REVERB *p, 
	double kFeedBack, double kLPFreq)
{
	p->kFeedBack = kFeedBack;
	p->kLPFreq = kLPFreq;
	p->iPitchMod  = 1;
	p->iSampleRate = g_srate;
	p->sampleRate = g_srate;
	p->iSkipInit = 0;
	int i;
	int nBytes;
	nBytes = 0;
	for (i = 0; i < 8; i++)
	 nBytes += delay_line_bytes_alloc(p, i);

	p->auxData.auxp = malloc(nBytes*sizeof(MYFLT));
	memset(p->auxData.auxp, 0, nBytes*sizeof(MYFLT));

	/* set up delay lines */
	nBytes = 0;
	for (i = 0; i < 8; i++) {
	 p->delayLines[i] = (delayLine*) ((unsigned char*) (p->auxData.auxp)
							    + (int) nBytes);
	 init_del_line(p, p->delayLines[i], i);
	 nBytes += delay_line_bytes_alloc(p, i);
	}
	p->dampFact = 1.0;
	p->prv_LPFreq = 0.0;
	p->initDone = 1.0;
	return 1;
}

//static int sc_reverb_perf(CSOUND *csound, SC_REVERB *p)
double reverbsc(SC_REVERB *p, double s)
{
	double 	s1, s2;
	double    ainL, ainR, aoutL, aoutR;
	double    vm1, v0, v1, v2, am1, a0, a1, a2, frac;
	delayLine *lp;
	int       n, readPos;
	int       bufferSize; /* Local copy */
	double    dampFact = p->dampFact;

	/* calculate tone filter coefficient if frequency changed */
	if (p->kLPFreq != p->prv_LPFreq) {
	 p->prv_LPFreq = p->kLPFreq;
	 dampFact = 2.0 - cos(p->prv_LPFreq * TWOPI / p->sampleRate);
	 dampFact = p->dampFact = dampFact - sqrt(dampFact * dampFact - 1.0);
	}
	 /* calculate "resultant junction pressure" and mix to input signals */
	 ainL = aoutL = aoutR = 0.0;
	 s1 = s;
	 s2 = s1;
	 for (n = 0; n < 8; n++)
	 	ainL += p->delayLines[n]->filterState;
	 ainL *= jpScale;
	 ainR = ainL + s1; 
	 ainL = ainL + s2;
	 /* loop through all delay lines */
	 for (n = 0; n < 8; n++) {
	   lp = p->delayLines[n];
	   bufferSize = lp->bufferSize;
	   /* send input signal and feedback to delay line */
	   lp->buf[lp->writePos] = (MYFLT) ((n & 1 ? ainR : ainL)
								 - lp->filterState);
	   if (++lp->writePos >= bufferSize)
		lp->writePos -= bufferSize;
	   /* read from delay line with cubic interpolation */
	   if (lp->readPosFrac >= DELAYPOS_SCALE) {
		lp->readPos += (lp->readPosFrac >> DELAYPOS_SHIFT);
		lp->readPosFrac &= DELAYPOS_MASK;
	   }
	   if (lp->readPos >= bufferSize)
		lp->readPos -= bufferSize;
	   readPos = lp->readPos;
	   frac = (double) lp->readPosFrac * (1.0 / (double) DELAYPOS_SCALE);
	   /* calculate interpolation coefficients */
	   a2 = frac * frac; a2 -= 1.0; a2 *= (1.0 / 6.0);
	   a1 = frac; a1 += 1.0; a1 *= 0.5; am1 = a1 - 1.0;
	   a0 = 3.0 * a2; a1 -= a0; am1 -= a2; a0 -= frac;
	   /* read four samples for interpolation */
	   if (readPos > 0 && readPos < (bufferSize - 2)) {
		vm1 = (double) (lp->buf[readPos - 1]);
		v0  = (double) (lp->buf[readPos]);
		v1  = (double) (lp->buf[readPos + 1]);
		v2  = (double) (lp->buf[readPos + 2]);
	   }
	   else {
		/* at buffer wrap-around, need to check index */
		if (--readPos < 0) readPos += bufferSize;
		vm1 = (double) lp->buf[readPos];
		if (++readPos >= bufferSize) readPos -= bufferSize;
		v0 = (double) lp->buf[readPos];
		if (++readPos >= bufferSize) readPos -= bufferSize;
		v1 = (double) lp->buf[readPos];
		if (++readPos >= bufferSize) readPos -= bufferSize;
		v2 = (double) lp->buf[readPos];
	   }
	   v0 = (am1 * vm1 + a0 * v0 + a1 * v1 + a2 * v2) * frac + v0;
	   /* update buffer read position */
	   lp->readPosFrac += lp->readPosFrac_inc;
	   /* apply feedback gain and lowpass filter */
	   v0 *= (double) p->kFeedBack;
	   v0 = (lp->filterState - v0) * dampFact + v0;
	   lp->filterState = v0;
	   /* mix to output */
	   if (n & 1)
		aoutR += v0;
	   else
		aoutL += v0;
	   /* start next random line segment if current one has reached endpoint */
	   if (--(lp->randLine_cnt) <= 0)
		next_random_lineseg(p, lp, n);
	 }
	 //p->aoutL[i] = (MYFLT) (aoutL * outputGain);
	 //p->aoutR[i] = (MYFLT) (aoutR * outputGain);
	return aoutL * outputGain; 		
	//return ainL;
}



struct Rev_Data
{
	SAMPLE max;
	SAMPLE min;
	SAMPLE mix;
	SAMPLE size;
	SAMPLE cutoff;
	SC_REVERB p;
	Rev_Data( ){
				max = 1.0f; 
				min = -1.0f; 
				mix = 0.5;
				size = 0.98f; 
				cutoff = 10000;}
	};

static t_CKUINT revsc_offset_data = 0;

DLL_QUERY revsc_query( Chuck_DL_Query * QUERY)
{
	Chuck_Env *env = Chuck_Env::instance();
	Chuck_DL_Func * func = NULL;

	g_srate = QUERY->srate;

	if(!type_engine_import_ugen_begin( 	env, "ReverbSC", "UGen", env->global(), 
										revsc_ctor, revsc_dtor, revsc_tick, NULL) )
	return FALSE;


	//add member function
	revsc_offset_data = type_engine_import_mvar(env, "int", "@revsc_data", FALSE);
	if( revsc_offset_data == CK_INVALID_OFFSET) goto error;	

	//add ctrl max
	func = make_new_mfun( "float", "max", revsc_ctrl_max );
	func->add_arg( "float", "max" );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	//add cget max
	func = make_new_mfun( "float", "max", revsc_cget_max );
	if( !type_engine_import_mfun( env, func ) ) goto error;
	
	//add ctrl min
	func = make_new_mfun( "float", "min", revsc_ctrl_min );
	func->add_arg( "float", "min" );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	//add cget min
	func = make_new_mfun( "float", "min", revsc_cget_min );
	if( !type_engine_import_mfun( env, func ) ) goto error;
	
	//add ctrl mix
	func = make_new_mfun( "float", "mix", revsc_ctrl_mix );
	func->add_arg( "float", "mix" );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	//add cget mix
	func = make_new_mfun( "float", "mix", revsc_cget_mix );
	if( !type_engine_import_mfun( env, func ) ) goto error;
	
	//add ctrl size
	func = make_new_mfun( "float", "size", revsc_ctrl_size );
	func->add_arg( "float", "size" );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	//add cget size
	func = make_new_mfun( "float", "size", revsc_cget_size );
	if( !type_engine_import_mfun( env, func ) ) goto error;
	
	//add ctrl cutoff
	func = make_new_mfun( "float", "cutoff", revsc_ctrl_cutoff );
	func->add_arg( "float", "cutoff" );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	//add cget cutoff
	func = make_new_mfun( "float", "cutoff", revsc_cget_cutoff );
	if( !type_engine_import_mfun( env, func ) ) goto error;

error:	
	if(!type_engine_import_class_end( env ))
		return FALSE;

	return TRUE;
}

CK_DLL_CTOR ( revsc_ctor )
{
	OBJ_MEMBER_UINT(SELF, revsc_offset_data) = (t_CKUINT)new Rev_Data;
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	reverbsc_init(&d->p, d->size, d->cutoff);
}
CK_DLL_DTOR ( revsc_dtor )
{
	delete (Rev_Data *) OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	OBJ_MEMBER_UINT(SELF, revsc_offset_data) = 0;
}

CK_DLL_TICK( revsc_tick )
{
	Rev_Data *d = (Rev_Data *)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	//*out = in > d->max ? d->max : (in < d->min ? d->min : in);
	*out = in * (1 - d->mix) + reverbsc(&d->p, in) * d->mix;
	return TRUE;
}

CK_DLL_CTRL( revsc_ctrl_max )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	d->max = (SAMPLE)GET_CK_FLOAT(ARGS);
	RETURN->v_float = (t_CKFLOAT)(d->max); } CK_DLL_CGET( revsc_cget_max ) {
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	RETURN->v_float = (t_CKFLOAT)(d->max);
}

CK_DLL_CTRL( revsc_ctrl_min )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	d->min = (SAMPLE)GET_CK_FLOAT(ARGS);
	RETURN->v_float = (t_CKFLOAT)(d->min);
}
CK_DLL_CGET( revsc_cget_min )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	RETURN->v_float = (t_CKFLOAT)(d->min);
}

CK_DLL_CTRL( revsc_ctrl_mix )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	d->mix = (SAMPLE)GET_CK_FLOAT(ARGS);
	RETURN->v_float = (t_CKFLOAT)(d->mix);
}
CK_DLL_CGET( revsc_cget_mix )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	RETURN->v_float = (t_CKFLOAT)(d->mix);
}
CK_DLL_CTRL( revsc_ctrl_size )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	d->size = (SAMPLE)GET_CK_FLOAT(ARGS);
	d->p.kFeedBack= d->size;
	RETURN->v_float = (t_CKFLOAT)(d->size);
}
CK_DLL_CGET( revsc_cget_size )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	RETURN->v_float = (t_CKFLOAT)(d->size);
}
CK_DLL_CTRL( revsc_ctrl_cutoff )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	d->cutoff = (SAMPLE)GET_CK_FLOAT(ARGS);
	d->p.kLPFreq = d->cutoff;
	RETURN->v_float = (t_CKFLOAT)(d->cutoff);
}
CK_DLL_CGET( revsc_cget_cutoff )
{
	Rev_Data *d = (Rev_Data*)OBJ_MEMBER_UINT(SELF, revsc_offset_data);
	RETURN->v_float = (t_CKFLOAT)(d->cutoff);
}
