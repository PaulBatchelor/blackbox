#include "chuck_type.h"
#include "ulib_csnd.h"
//#include "util_math.h"
//#include "ulib_std.h"

#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <csound/csound.hpp>

DLL_QUERY libcsnd_query( Chuck_DL_Query * QUERY )
{
    // get global
    Chuck_Env * env = Chuck_Env::instance();
    // name
    QUERY->setname( QUERY, "Csnd" );

    /*! \example
    math.sin( math.pi /2.0 ) => stdout;
    */

    // register deprecate
    //type_engine_register_deprecate( env, "math", "Math" );
    //Csound *cs = new Csound();
    // add class
    QUERY->begin_class( QUERY, "Csnd", "Object" );
    // sin
    QUERY->add_sfun( QUERY, inputMessage, "float", "inputMessage" );
    //QUERY->add_arg( QUERY, "float", "x" );
    // done
    QUERY->end_class( QUERY );

    return TRUE;
}

CK_DLL_SFUN( inputMessage )
{
    RETURN->v_float = 3;
}
