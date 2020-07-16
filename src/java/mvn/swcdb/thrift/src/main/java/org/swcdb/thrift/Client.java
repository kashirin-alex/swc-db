/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


package org.swcdb.thrift;

import org.apache.thrift.TException;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TFramedTransport;
import org.apache.thrift.transport.TTransportException;
import org.apache.thrift.protocol.TBinaryProtocol;

import org.swcdb.thrift.gen.Service;


public class Client extends Service.Client {

  ///
  public static Client create(String host, int port, int timeout_ms,
                              boolean do_open, int max_framesize)
                              throws TTransportException, TException {
    return new Client(
      new TFramedTransport(
        new TSocket(host, port, timeout_ms), max_framesize), 
        do_open);
  }

  public static Client create(String host, int port, int timeout_ms,
                              boolean do_open)
                              throws TTransportException, TException {
    return new Client(
      new TFramedTransport(
        new TSocket(host, port, timeout_ms)), 
        do_open);
  }

  public static Client create(String host, int port, boolean do_open)
                              throws TTransportException, TException {
    return new Client(
      new TFramedTransport(
        new TSocket(host, port)), 
        do_open);
  }
  ///


  public Client(TFramedTransport transport, boolean do_open) 
                throws TTransportException, TException {
    super( new TBinaryProtocol(transport) );
    this.transport = transport;
    if(do_open)
      this.transport.open();
  }

  public boolean is_open() throws TTransportException, TException {
    return transport.isOpen();
  }

  public void open() throws TTransportException, TException {
    if(!is_open())
      transport.open();
  }

  public void close() {
    try {
      if(is_open())
        transport.close();
    } catch (java.lang.Exception e) { }
  }


  private TFramedTransport transport;

}
