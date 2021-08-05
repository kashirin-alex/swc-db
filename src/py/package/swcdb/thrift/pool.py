# -*- coding: utf-8 -*-
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


import time
import socket
import logging
import threading
import struct

from swcdb.thrift import service
from thrift.transport.TTransport import TTransportException

__all__ = ['Pool', 'PoolService']


#


class PoolClient(service.Client):
    __slots__ = ['expiry']

    def __init__(self, pool, sock):
        self.expiry = int(time.time()) + pool.timeout
        service.Client.__init__(self, pool.host, pool.port, timeout_ms=pool.timeout, socket=sock)
        #

    def __repr__(self):
        return '%s(expiry=%d)' % (self.__class__, self.expiry)
        #

    #


class Pool(object):
    __slots__ = ['__sem', '__size', '__open', '__await', '__clients', '__stop', 'host', 'port', 'timeout', 'logger']

    def __init__(self, size, host="localhost", port=18000, timeout_ms=900000, logger=None):
        self.__sem = threading.Semaphore(0)
        self.__size = size
        self.__open = 0
        self.__await = 0
        self.__clients = []
        self.host = host
        self.port = port
        self.timeout = timeout_ms
        if logger is None:
            logging.basicConfig(format='%(asctime)s %(levelname)s: %(message)s')
            self.logger = logging.getLogger()
            self.logger.setLevel(logging.INFO)
        else:
            self.logger = logger
        self.__stop = None
        #

    def open(self):
        return self.__open
        #

    def available(self):
        return len(self.__clients)
        #

    def await(self):
        return self.__await
        #

    def size(self):
        return self.__size
        #

    def change_size(self, size):
        prev = self.__size
        self.__size = size
        if self.__await > 0 and size > prev:
            self.__sem.release()
            self.__sem.acquire()
        #

    def stop(self):
        self.__stop = True
        #

    def is_running(self):
        return self.__stop is None
        #

    def make_socket(self):
        r = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        r.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
        r.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)
        r.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        r.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        r.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        r.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

        r.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPIDLE, 10)
        r.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPINTVL, 15)
        r.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPCNT, 3)
        r.connect((self.host, self.port))
        return r
        #

    def sleep(self, seconds):
        try:
            time.sleep(seconds)
        except:
            self.logger.exception('sleep problem %s, to stop issue Pool.stop()' % self.__repr__())
        #

    def get(self):
        while self.is_running():
            try:
                if self.__clients:
                    ts = int(time.time())
                    client = self.__clients.pop(-1)
                    if client.expiry < ts:
                        self.discard(client)
                        continue
                    client.expiry = ts + self.timeout
                    return client

                if self.__size <= self.__open:
                    self.__await += 1
                    self.logger.info('Wait Client release (open=%d await=%d)' % (self.open(), self.await()))
                    try:
                        self.__sem.acquire()
                        self.sleep(0)
                    except:
                        self.sleep(1)
                    self.__await -= 1
                    continue
            except:
                self.logger.exception('Semaphore problem %s' % self.__repr__())
                self.sleep(0)
                continue

            self.logger.debug('Connect new Client %s' % self.__repr__())
            self.__open += 1
            try:
                r = self.make_socket()
                try:
                    return PoolClient(self, r)
                except:
                    self.logger.exception('Client connect problem %s' % self.__repr__())
                r.close()
            except:
                self.logger.exception('Socket connect problem %s' % self.__repr__())
            self.__open -= 1
            self.sleep(3)
        #

    def put(self, client):
        self.__clients.append(client)
        if self.__await > 0:
            self.__sem.release()
        #

    def discard(self, client):
        try:
            client.close()
        except:
            pass
        self.__open -= 1
        if self.__await > 0:
            self.__sem.release()
        #

    def close(self):
        while self.__clients:
            self.discard(self.__clients.pop(-1))
        #
    __del__ = close

    def __repr__(self):
        return '%s(host=%s port=%d timeout=%d size=%d open=%d await=%d available=%d%s) clients=[%s]' % (
            self.__class__,
            self.host, self.port, self.timeout,
            self.size(), self.open(), self.await(), self.available(),
            ("" if self.is_running() else " STOPPED"),
            ("\n ".join([""] + [client.__repr__() for client in self.__clients] + [""])) if self.__clients else ""
        )
        #
    #


#


class InsistentClient(object):
    __slots__ = ['__pool', '__method']

    def __init__(self, pool, meth):
        self.__pool = pool
        self.__method = getattr(PoolClient, meth)
        #

    def __call__(self, *args, **kwargs):
        while self.__pool.is_running():
            err = None
            client = self.__pool.get()
            try:
                return self.__method(client, *args, **kwargs)
            except service.Exception as e:
                err = True
                if 3061 <= e.code <= 3062 or 3041 <= e.code <= 3056:
                    raise e
                self.__pool.logger.exception('call problem code=%d %s' % (e.code, self.__repr__()))
                self.__pool.sleep(1)
            except (TypeError, AttributeError, struct.error) as e:
                err = True
                raise e
            except TTransportException:
                err = True
                self.__pool.logger.exception('call problem %s' % self.__repr__())
            except:
                err = True
                self.__pool.logger.exception('call problem %s' % self.__repr__())
                self.__pool.sleep(1)
            finally:
                if err is None:
                    self.__pool.put(client)
                else:
                    self.__pool.discard(client)
        #
    #


class PoolService(Pool):
    __slots__ = []

    def __init__(self, *args, **kwargs):
        Pool.__init__(self, *args, **kwargs)
        #

    def __getattr__(self, method):
        return InsistentClient(self, method)
        #
    #
