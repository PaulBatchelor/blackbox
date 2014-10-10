#include "ugen_csound.h"
#include "chuck_type.h"
#include "chuck_ugen.h"
#include "chuck_vm.h"
#include "chuck_globals.h"
#include <csound/csound.hpp>
#include <csound/csPerfThread.hpp>
#include <stdio.h>

static t_CKUINT csound_offset_data = 0;

void Stupid_Callback(void *what){
}


DLL_QUERY csound_query( Chuck_DL_Query * QUERY )
{
    Chuck_Env * env = Chuck_Env::instance();
    Chuck_DL_Func * func = NULL;

    //---------------------------------------------------------------------
    // init as base class: csound
    //---------------------------------------------------------------------
    if( !type_engine_import_ugen_begin( env, "Csound", "UGen", env->global(),
                                        csound_ctor, csound_dtor, csound_tick, NULL ) )
        return FALSE;

    // add member variable
    csound_offset_data = type_engine_import_mvar( env, "int", "@csound_data", FALSE );
    if( csound_offset_data == CK_INVALID_OFFSET ) goto error;

    // add ctrl: max
    func = make_new_mfun( "void", "compile", csound_compile );
    func->add_arg( "string", "filename" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    func = make_new_mfun( "void", "compileOrc", csound_compile_orc );
    func->add_arg( "string", "filename" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    
    func = make_new_mfun( "void", "inputMessage", csound_inputMessage);
    func->add_arg( "string", "score" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    //// add cget: max
    //func = make_new_mfun( "float", "max", csound_cget_max );
    //if( !type_engine_import_mfun( env, func ) ) goto error;

    //// add ctrl: min
    //func = make_new_mfun( "float", "min", csound_ctrl_min );
    //func->add_arg( "float", "min" );
    //if( !type_engine_import_mfun( env, func ) ) goto error;

    //// add cget: min
    //func = make_new_mfun( "float", "min", csound_cget_min );
    //if( !type_engine_import_mfun( env, func ) ) goto error;

    // end import
    if( !type_engine_import_class_end( env ) )
        return FALSE;

    return TRUE;

error:
    // end import
    if( !type_engine_import_class_end( env ) )
        return FALSE;

    return FALSE;
}

struct Csound_Data
{
    SAMPLE max;
    SAMPLE min;
    Csound *cs;
    MYFLT *output;
    MYFLT *input;
    long pos;
    long inBufSize; 
    long outBufSize; 
    bool isRunning;
    CsoundPerformanceThread *perfThread; 
    Csound_Data( ) { max = 1.0f; min = -1.0f; }
};

CK_DLL_CTOR( csound_ctor )
{
    // return data to be used later
    OBJ_MEMBER_UINT(SELF, csound_offset_data) = (t_CKUINT)new Csound_Data;
    Csound_Data *d = (Csound_Data*)OBJ_MEMBER_UINT(SELF, csound_offset_data);
    d->cs = new Csound();
}

CK_DLL_DTOR( csound_dtor )
{
    Csound_Data *d = (Csound_Data *)OBJ_MEMBER_UINT(SELF, csound_offset_data);
    d->cs->Cleanup();
    delete (Csound_Data*) OBJ_MEMBER_UINT(SELF, csound_offset_data);
    OBJ_MEMBER_UINT(SELF, csound_offset_data) = 0;
}

CK_DLL_TICK( csound_tick )
{
    Csound_Data *d = (Csound_Data *)OBJ_MEMBER_UINT(SELF, csound_offset_data);
    if(d->pos < (d->outBufSize- 1)) {
        d->pos++;
    }else{
        if(d->isRunning) {
            if(d->cs->PerformBuffer()){
                printf("we are done here...\n");
                d->cs->Cleanup();
                d->isRunning = false;
            }
            d->pos= 0;
        }
    }

    if(d->isRunning)
    {
    *out = (float)d->output[d->pos];
    //d->input[d->pos] = in;
    *d->input = in;
    }else{
    *out = 0;
    }
    return TRUE;
}

CK_DLL_CTRL( csound_compile )
{
    Csound_Data *d = (Csound_Data *)OBJ_MEMBER_UINT(SELF, csound_offset_data);
    Chuck_String * ckfilename = GET_CK_STRING(ARGS);
    const char * tmp = ckfilename->str.c_str();
    char * filename = (char *) tmp;
    d->cs->CompileCsd(filename); 
    d->outBufSize= d->cs->GetOutputBufferSize();
    d->output = d->cs->GetOutputBuffer();
    //d->input= d->cs->GetInputBuffer();
    d->cs->GetChannelPtr(d->input, "Chuck_Out", CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL); 
    d->pos = 0; 
    d->perfThread = new CsoundPerformanceThread(d->cs->GetCsound()); 
    d->isRunning = true;
    //d->perfThread->Play();
}

CK_DLL_CTRL( csound_compile_orc )
{
    Csound_Data *d = (Csound_Data *)OBJ_MEMBER_UINT(SELF, csound_offset_data);
    Chuck_String * ckorc= GET_CK_STRING(ARGS);
    const char * tmp = ckorc->str.c_str();
    char * orc= (char *) tmp;
    d->cs->CompileOrc(orc); 
    d->outBufSize= d->cs->GetOutputBufferSize();
    d->output = d->cs->GetOutputBuffer();
    //d->input= d->cs->GetInputBuffer();
    d->cs->GetChannelPtr(d->input, "Chuck_Out", CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL); 
    d->pos = 0; 
    d->perfThread = new CsoundPerformanceThread(d->cs->GetCsound()); 
    d->isRunning = true;
    //d->perfThread->Play();
}


CK_DLL_CTRL( csound_inputMessage)
{
    Csound_Data *d = (Csound_Data *)OBJ_MEMBER_UINT(SELF, csound_offset_data);
    Chuck_String * ckstring = GET_CK_STRING(ARGS);
    const char * tmp = ckstring->str.c_str();
    char * score = (char *) tmp;
    d->perfThread->InputMessage(score);
}
