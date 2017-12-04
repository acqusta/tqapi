from distutils.core import setup, Extension

import numpy as np

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


# zeromq

zeromq = [ 'zeromq',
    {
        'sources' : [
            "lib/zeromq-4.2.1//src/tipc_connecter.cpp",
            "lib/zeromq-4.2.1//src/gssapi_mechanism_base.cpp",
            "lib/zeromq-4.2.1//src/dish.cpp",
            "lib/zeromq-4.2.1//src/tipc_address.cpp",
            "lib/zeromq-4.2.1//src/scatter.cpp",
            "lib/zeromq-4.2.1//src/v1_decoder.cpp",
            "lib/zeromq-4.2.1//src/reaper.cpp",
            "lib/zeromq-4.2.1/src/err.cpp",
            "lib/zeromq-4.2.1/src/pipe.cpp",
            "lib/zeromq-4.2.1/src/thread.cpp",
            "lib/zeromq-4.2.1/src/plain_server.cpp",
            "lib/zeromq-4.2.1/src/pull.cpp",
            "lib/zeromq-4.2.1/src/select.cpp",
            "lib/zeromq-4.2.1/src/curve_client.cpp",
            "lib/zeromq-4.2.1/src/poller_base.cpp",
            "lib/zeromq-4.2.1/src/udp_engine.cpp",
            "lib/zeromq-4.2.1/src/pollset.cpp",
            "lib/zeromq-4.2.1/src/client.cpp",
            "lib/zeromq-4.2.1/src/gather.cpp",
            "lib/zeromq-4.2.1/src/socket_poller.cpp",
            "lib/zeromq-4.2.1/src/pgm_receiver.cpp",
            "lib/zeromq-4.2.1/src/rep.cpp",
            "lib/zeromq-4.2.1/src/req.cpp",
            "lib/zeromq-4.2.1/src/precompiled.cpp",
            "lib/zeromq-4.2.1/src/kqueue.cpp",
            "lib/zeromq-4.2.1/src/io_object.cpp",
            "lib/zeromq-4.2.1/src/v2_encoder.cpp",
            "lib/zeromq-4.2.1/src/tcp.cpp",
            "lib/zeromq-4.2.1/src/v1_encoder.cpp",
            "lib/zeromq-4.2.1/src/vmci_address.cpp",
            "lib/zeromq-4.2.1/src/random.cpp",
            "lib/zeromq-4.2.1/src/stream_engine.cpp",
            "lib/zeromq-4.2.1/src/poll.cpp",
            "lib/zeromq-4.2.1/src/gssapi_server.cpp",
            "lib/zeromq-4.2.1/src/object.cpp",
            "lib/zeromq-4.2.1/src/ip.cpp",
            "lib/zeromq-4.2.1/src/mechanism.cpp",
            "lib/zeromq-4.2.1/src/clock.cpp",
            "lib/zeromq-4.2.1/src/epoll.cpp",
            "lib/zeromq-4.2.1/src/router.cpp",
            "lib/zeromq-4.2.1/src/trie.cpp",
            "lib/zeromq-4.2.1/src/push.cpp",
            "lib/zeromq-4.2.1/src/options.cpp",
            "lib/zeromq-4.2.1/src/socket_base.cpp",
            "lib/zeromq-4.2.1/src/v2_decoder.cpp",
            "lib/zeromq-4.2.1/src/tcp_address.cpp",
            "lib/zeromq-4.2.1/src/mtrie.cpp",
            "lib/zeromq-4.2.1/src/zmq.cpp",
            "lib/zeromq-4.2.1/src/mailbox_safe.cpp",
            "lib/zeromq-4.2.1/src/io_thread.cpp",
            "lib/zeromq-4.2.1/src/proxy.cpp",
            "lib/zeromq-4.2.1/src/gssapi_client.cpp",
            "lib/zeromq-4.2.1/src/mailbox.cpp",
            "lib/zeromq-4.2.1/src/udp_address.cpp",
            "lib/zeromq-4.2.1/src/raw_decoder.cpp",
            "lib/zeromq-4.2.1/src/devpoll.cpp",
            "lib/zeromq-4.2.1/src/pgm_sender.cpp",
            "lib/zeromq-4.2.1/src/vmci_connecter.cpp",
            "lib/zeromq-4.2.1/src/address.cpp",
            "lib/zeromq-4.2.1/src/socks.cpp",
            "lib/zeromq-4.2.1/src/stream.cpp",
            "lib/zeromq-4.2.1/src/xsub.cpp",
            "lib/zeromq-4.2.1/src/session_base.cpp",
            "lib/zeromq-4.2.1/src/own.cpp",
            "lib/zeromq-4.2.1/src/ctx.cpp",
            "lib/zeromq-4.2.1/src/fq.cpp",
            "lib/zeromq-4.2.1/src/tcp_listener.cpp",
            "lib/zeromq-4.2.1/src/timers.cpp",
            "lib/zeromq-4.2.1/src/pub.cpp",
            "lib/zeromq-4.2.1/src/ipc_address.cpp",
            "lib/zeromq-4.2.1/src/metadata.cpp",
            "lib/zeromq-4.2.1/src/radio.cpp",
            "lib/zeromq-4.2.1/src/plain_client.cpp",
            "lib/zeromq-4.2.1/src/curve_server.cpp",
            "lib/zeromq-4.2.1/src/lb.cpp",
            "lib/zeromq-4.2.1/src/pair.cpp",
            "lib/zeromq-4.2.1/src/dist.cpp",
            "lib/zeromq-4.2.1/src/vmci_listener.cpp",
            "lib/zeromq-4.2.1/src/norm_engine.cpp",
            "lib/zeromq-4.2.1/src/socks_connecter.cpp",
            "lib/zeromq-4.2.1/src/vmci.cpp",
            "lib/zeromq-4.2.1/src/raw_encoder.cpp",
            "lib/zeromq-4.2.1/src/pgm_socket.cpp",
            "lib/zeromq-4.2.1/src/signaler.cpp",
            "lib/zeromq-4.2.1/src/tcp_connecter.cpp",
            "lib/zeromq-4.2.1/src/ipc_connecter.cpp",
            "lib/zeromq-4.2.1/src/zmq_utils.cpp",
            "lib/zeromq-4.2.1/src/xpub.cpp",
            "lib/zeromq-4.2.1/src/null_mechanism.cpp",
            "lib/zeromq-4.2.1/src/dgram.cpp",
            "lib/zeromq-4.2.1/src/msg.cpp",
            "lib/zeromq-4.2.1/src/tipc_listener.cpp",
            "lib/zeromq-4.2.1/src/ipc_listener.cpp",
            "lib/zeromq-4.2.1/src/decoder_allocators.cpp",
            "lib/zeromq-4.2.1/src/sub.cpp",
            "lib/zeromq-4.2.1/src/dealer.cpp",
            "lib/zeromq-4.2.1/src/server.cpp"
        ],

        'define_macros' : [ ('ZMQ_STATIC', 1)],
        'include_dirs'  : [
            'lib/zeromq-4.2.1/include',
            'lib/zeromq-4.2.1/tq/osx'
        ]
    }]


sources = [
    "lib/snappy/snappy-stubs-internal.cc",
    "lib/snappy/snappy-c.cc",
    "lib/snappy/snappy-sinksource.cc",
    "lib/snappy/snappy.cc",
    "lib/myutils/jsonrpc.cpp",
    "lib/myutils/zmq_connection.cpp",
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
    ('MINOR_VERSION', '0'),
    ('GOOGLE_GLOG_DLL_DECL', ''),
    ('GLOG_NO_ABBREVIATED_SEVERITIES',''),
    ('ZMQ_STATIC', '1'),
    #('HAVE_CONFIG_H', '1')
]
    
include_dirs = [
    'lib',
    'lib/msgpack/include',
    'pyext',
    'c',
    'lib/glog-master/tq',
    #'lib/glog-master/tq/osx',
    'lib/glog-master/src',
    'lib/zeromq-4.2.1/include',
    np.get_include()
]

module = Extension('tqapi._tqapi',
                   define_macros = define_macros,
                   include_dirs = include_dirs,
                   libraries = [ 'msgpack', 'zeromq', 'glog'],
                   library_dirs = [],
                   #install_requires = ['numpy'],
                   sources = sources)

module.extra_compile_args = ['-std=c++11','']

#export MACOSX_DEPLOYMENT_TARGET=10.10

setup(packages = [ 'tqapi' ],
      package_dir = { 'tqapi' : 'pyext'},
      libraries = [ msgpack, zeromq, glog ],
      name = 'tqapi',
      version = '1.0',
      ext_modules = [module])
