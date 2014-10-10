#ifndef __UGEN_CSOUND_H__
#define __UGEN_CSOUND_H__

#include "chuck_dl.h"

// query
DLL_QUERY csound_query( Chuck_DL_Query * query );

// clip
CK_DLL_CTOR( csound_ctor );
CK_DLL_DTOR( csound_dtor );
CK_DLL_TICK( csound_tick );
CK_DLL_CTRL( csound_compile);
CK_DLL_CTRL( csound_compile_orc);
CK_DLL_CTRL( csound_inputMessage);
//CK_DLL_CGET( clip_cget_max );
//CK_DLL_CTRL( clip_ctrl_min );
//CK_DLL_CGET( clip_cget_min );

#endif 
