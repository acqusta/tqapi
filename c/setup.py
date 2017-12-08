from distutils.core import setup, Extension

import numpy as np
import os

if hasattr(os, 'uname'):
    OSNAME = os.uname()[0]
else:
    OSNAME = 'Windows'

msgpack = ( 'msgpack',
    {
        'sources' : [
            "lib/msgpack/src/vrefbuffer.c",
            "lib/msgpack/src/unpack.c",
            "lib/msgpack/src/objectc.c",
            "lib/msgpack/src/zone.c",
            "lib/msgpack/src/version.c",
        ],

        'include_dirs' : [
            'lib/msgpack/include'
        ]
    })


# glog

glog = ( 'glog', {
    'sources' : [
        "lib/glog-master/src/logging.cc",
        "lib/glog-master/src/raw_logging.cc",
        "lib/glog-master/src/utilities.cc",
        "lib/glog-master/src/vlog_is_on.cc",
        "lib/glog-master/src/symbolize.cc",
        "lib/glog-master/src/demangle.cc",
        "lib/glog-master/src/signalhandler.cc"
    ],
    'define_macros' : [
        ('GOOGLE_GLOG_DLL_DECL', ''),
        ('GLOG_NO_ABBREVIATED_SEVERITIES',''),
        ('HAVE_CONFIG_H', '1')
    ],
    'include_dirs' : [
        'lib/glog-master/tq',
        'lib/glog-master/tq/osx',
        'lib/glog-master/src'
    ]
})


sources = [
    "lib/snappy/snappy-stubs-internal.cc",
    "lib/snappy/snappy-c.cc",
    "lib/snappy/snappy-sinksource.cc",
    "lib/snappy/snappy.cc",
    "lib/myutils/mprpc.cpp",
    "lib/myutils/socket_connection.cpp",
    "lib/loop/RunLoop.cpp",
    "lib/loop/MessageLoop.cpp",
    "pyext/tqapi_tapi.cpp",
    "pyext/tqapi_dapi.cpp",
    "pyext/tqapi_py.cpp",
    "c/impl_tquant_api.cpp",
    "c/tquant_api_test.cpp",
]

define_macros = [
    ('MAJOR_VERSION', '1'),
    ('MINOR_VERSION', '0')
]
    
include_dirs = [
    'lib',
    'lib/msgpack/include',
    'pyext',
    'c',
    np.get_include()
]

module = Extension('tqapi._tqapi',
                   define_macros = define_macros,
                   include_dirs = include_dirs,
                   libraries = [ 'msgpack', 'ws2_32'],
                   library_dirs = [],
                   #install_requires = ['numpy'],
                   sources = sources)

if OSNAME == "Windows":
    module.extra_compile_args = ['/MT']
else:
    module.extra_compile_args = ['-std=c++11','']
    

#export MACOSX_DEPLOYMENT_TARGET=10.10

setup(packages = [ 'tqapi' ],
      package_dir = { 'tqapi' : 'pyext'},
      libraries = [ msgpack ],
      name = 'tqapi',
      version = '1.0',
      ext_modules = [module])
