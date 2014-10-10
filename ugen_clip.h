#ifndef __UGEN_CLIP_H__
#define __UGEN_CLIP_H__

#include "chuck_dl.h"

// query
DLL_QUERY clip_query( Chuck_DL_Query * query );

// clip
CK_DLL_CTOR( clip_ctor );
CK_DLL_DTOR( clip_dtor );
CK_DLL_TICK( clip_tick );
CK_DLL_CTRL( clip_ctrl_max );
CK_DLL_CGET( clip_cget_max );
CK_DLL_CTRL( clip_ctrl_min );
CK_DLL_CGET( clip_cget_min );

#endif // __UGEN_CLIP_H__
