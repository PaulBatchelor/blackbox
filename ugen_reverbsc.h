#ifndef __UGEN_REVERBSC_H__
#define __UGEN_REVERBSC_H__

#define OK 1
#define MYFLT double 

#include "chuck_dl.h"

#endif

DLL_QUERY revsc_query( Chuck_DL_Query * query );

CK_DLL_TICK ( revsc_tick );

CK_DLL_CTOR ( revsc_ctor );
CK_DLL_DTOR ( revsc_dtor );

CK_DLL_CTRL( revsc_ctrl_max );
CK_DLL_CGET( revsc_cget_max );

CK_DLL_CTRL( revsc_ctrl_min );
CK_DLL_CGET( revsc_cget_min );

CK_DLL_CTRL( revsc_ctrl_size );
CK_DLL_CGET( revsc_cget_size );

CK_DLL_CTRL( revsc_ctrl_cutoff );
CK_DLL_CGET( revsc_cget_cutoff );

CK_DLL_CTRL( revsc_ctrl_mix );
CK_DLL_CGET( revsc_cget_mix );
