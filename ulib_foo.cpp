#include "chuck_type.h"
#include "ulib_foo.h"
//#include "util_math.h"
//#include "ulib_std.h"

#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>

DLL_QUERY libfoo_query( Chuck_DL_Query * QUERY )
{
    // get global
    Chuck_Env * env = Chuck_Env::instance();
    // name
    QUERY->setname( QUERY, "Foo" );

    /*! \example
    math.sin( math.pi /2.0 ) => stdout;
    */

    // register deprecate
    //type_engine_register_deprecate( env, "math", "Math" );

    // add class
    QUERY->begin_class( QUERY, "Foo", "Object" );
    // sin
    QUERY->add_sfun( QUERY, bar, "float", "bar" );
    //QUERY->add_arg( QUERY, "float", "x" );
    // done
    QUERY->end_class( QUERY );

    return TRUE;
}

CK_DLL_SFUN( bar )
{
    RETURN->v_float = 3;
}
