EM_log( CK_LOG_SEVERE, "class 'ReverbSC' ...");
if( !load_module( env, revsc_query, "ReverbSC", "global" ) ) goto error;

EM_log( CK_LOG_SEVERE, "class 'Clip' ...");
if( !load_module( env, revsc_query, "Clip", "global" ) ) goto error;

EM_log( CK_LOG_SEVERE, "class 'Foo' ...");
if( !load_module( env, libfoo_query, "Foo", "global" ) ) goto error;

EM_log( CK_LOG_SEVERE, "class 'Csnd' ...");
if( !load_module( env, libcsnd_query, "Csnd", "global" ) ) goto error;

EM_log( CK_LOG_SEVERE, "class 'Csound' ...");
if( !load_module( env, csound_query, "Csound", "global" ) ) goto error;
