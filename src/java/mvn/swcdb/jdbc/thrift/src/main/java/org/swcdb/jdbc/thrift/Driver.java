/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


package org.swcdb.jdbc.thrift;

import java.util.Properties;
import java.util.concurrent.Executor;
import java.util.logging.Logger;



/* https://docs.oracle.com/javase/10/docs/api/java/sql/Driver.html */

public class Driver implements java.sql.Driver {

  public Driver() {

  }
  
  @Override
  public java.sql.Connection connect(String url, Properties info) 
                                      throws java.sql.SQLException {
    return new Connection(url, info);
  }

  @Override
  public Logger getParentLogger()
                                throws java.sql.SQLFeatureNotSupportedException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public boolean jdbcCompliant() {
    return false;
  }

  @Override
  public int getMinorVersion() {
    return org.swcdb.jdbc.thrift.Connection.version_minor;
  }

  @Override
  public int getMajorVersion() {
    return org.swcdb.jdbc.thrift.Connection.version_major;
  }

  @Override
  public java.sql.DriverPropertyInfo[] getPropertyInfo(String url, Properties info)
                                                       throws java.sql.SQLException {
    java.sql.DriverPropertyInfo[] props = {
      new java.sql.DriverPropertyInfo("timeout_ms", "900000")
    };
    props[0].description = "Client Connection Timeout in ms";
    
    return props;
  }

  @Override
  public boolean acceptsURL(String url)
                            throws java.sql.SQLException {
    // url = "jdbc:swcdb:thrift:hostname:18000/";
    String[] url_parts = url.toLowerCase().split(":");  
    return  url_parts.length > 2 && 
            url_parts[0].equals("jdbc") && 
            url_parts[1].equals("swcdb") && 
            url_parts[2].equals("thrift");
  }
  
  
}


