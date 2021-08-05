# -*- coding: utf-8 -*-
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


from thrift.transport import TTransport, TSocket
TSSLSocket = None
from thrift.protocol.TBinaryProtocol import TBinaryProtocol

from swcdb.thrift.native import Service
from swcdb.thrift.native.ttypes import *


# Not Implemented thrift.transport.TSSLSocket
# def init_ssl(handler=None, formatter=None):
#    import logging
#    from thrift.transport import TSSLSocket as _ssl_socket
#    if not handler:
#        handler = logging.StreamHandler()
#    handler.setLevel(logging.DEBUG)
#    if not formatter:
#        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
#    handler.setFormatter(formatter)
#
#    _ssl_socket.logger.setLevel(logging.DEBUG)
#    _ssl_socket.logger.addHandler(handler)
#    global TSSLSocket
#    TSSLSocket = _ssl_socket
#    #


class Client(Service.Client):
    __slots__ = ['transport']

    def __init__(self, host, port, timeout_ms=900000, socket=None, do_open=True, framed=True):
    
        # if ssl:
        #    socket = TSSLSocket.TSSLSocket(
        #       host, port, validate=validate, ca_certs=ca_certs, keyfile=keyfile, certfile=certfile)

        s = TSocket.TSocket(host, port)
        if socket:
            s.setHandle(socket)
        s.setTimeout(timeout_ms)

        if framed:
            self.transport = TTransport.TFramedTransport(s)
        else:
            self.transport = TTransport.TBufferedTransport(s)

        Service.Client.__init__(self, TBinaryProtocol(self.transport))
        if not socket and do_open:
            self.open()
    #

    def open(self):
        self.transport.open()
    #

    def close(self):
        self.transport.close()
    #

#




