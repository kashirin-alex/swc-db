  /*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


package org.swcdb.jdbc.thrift;

import org.swcdb.thrift.Client;

import java.util.Properties;
import java.util.concurrent.Executor;
import java.util.Map;


/* https://docs.oracle.com/javase/10/docs/api/java/sql/Connection.html */

public class Connection implements java.sql.Connection {

  static public int version_minor = 1;
  static public int version_major = 0;

  public Connection(String url, Properties info) 
                         throws java.sql.SQLException {    
    properties = new Properties(info);
    try {    
      // url = "jdbc:swcdb:thrift:hostname:18000/";             
      String[] url_parts = url.split(":");  
      m_client = Client.create(
        url_parts[3], 
        Integer.parseInt(url_parts[4]),
        Integer.parseInt(properties.getProperty("timeout_ms", "900000")), 
        true
      );
    } catch (java.lang.Exception e) {
      throw new java.sql.SQLException(e);
    }
  }
  
  @Override
  public int getNetworkTimeout() throws java.sql.SQLException {
    return Integer.parseInt(properties.getProperty("timeout_ms", "900000"));
  }
  

  @Override
  public void setNetworkTimeout(Executor executor, int ms) 
                                throws java.sql.SQLFeatureNotSupportedException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }
  

  @Override
  public void abort(Executor executor) throws java.sql.SQLException {
    m_client.close();
  }
  

  @Override
  public boolean isClosed() throws java.sql.SQLException {
    try {
      return !m_client.is_open();
    } catch(Exception e) {
      throw new java.sql.SQLException(e);
    }
  }

  @Override
  public boolean isValid(int timeout) throws java.sql.SQLException {
    try {
      if(isClosed())
        return false;
      // ClientAsync, timeout > abort
      // return m_client.status() == 0;
      return true;
    } catch(Exception e) {
      throw new java.sql.SQLException(e);
    }
  }


  @Override
  public boolean isReadOnly() throws java.sql.SQLException {
    return false;
  }

  @Override
  public void setReadOnly(boolean readOnly) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public boolean getAutoCommit() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void setAutoCommit(boolean autoCommit) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public Properties getClientInfo() throws java.sql.SQLException {
    try {
      if(!isClosed())
        throw new java.sql.SQLException();
    } catch(Exception e) {
      throw new java.sql.SQLException(e);
    }

    return new Properties(properties);
  }

  @Override
  public String getClientInfo(String name) throws java.sql.SQLException {
    try {
      if(!isClosed())
        throw new java.sql.SQLException();
    } catch(Exception e) {
      throw new java.sql.SQLException(e);
    }
    
    return properties.getProperty(name);
  }

  @Override
  public void setClientInfo(Properties info) throws java.sql.SQLClientInfoException {
    try {
      if(!isClosed())
        throw new java.sql.SQLClientInfoException();
    } catch(Exception e) {
      throw new java.sql.SQLClientInfoException();
    }

    properties = new Properties(info);
  }

  @Override
  public void setClientInfo(String name, String value) throws java.sql.SQLClientInfoException {
    try {
      if(!isClosed())
        throw new java.sql.SQLClientInfoException();
    } catch(Exception e) {
      throw new java.sql.SQLClientInfoException();
    }
    
    properties.setProperty(name, value);
  }
  

  @Override
  public void close() throws java.sql.SQLException {
    m_client.close();
  }


  @Override
  public String getSchema() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void setSchema(String schema) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public String nativeSQL(String sql) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public java.sql.Struct createStruct(String typeName, Object[] attributes)
                                  throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
    // java.sql.Struct structure = new java.sql.Struct();
    // return structure;
  }

  @Override
  public java.sql.Array createArrayOf(String typeName, Object[] elements)
                                throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
    // java.sql.Array arr = new java.sql.Array();
    // return arr;
  }

  @Override
  public String getCatalog() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void setCatalog(String catalog) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }
  

  @Override
  public java.sql.DatabaseMetaData getMetaData() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  
  @Override
  public java.sql.SQLXML createSQLXML() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.NClob createNClob() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.Blob createBlob() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.Clob createClob() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public int getHoldability() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void setHoldability(int holdability) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public java.sql.PreparedStatement prepareStatement(String sql) 
                                                     throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.PreparedStatement prepareStatement(String sql, String[] columnNames) 
                                                     throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.PreparedStatement prepareStatement(String sql, int[] columnIndexes) 
                                                     throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.PreparedStatement prepareStatement(String sql, int autoGeneratedKeys) 
                                                     throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.PreparedStatement prepareStatement(String sql,
                                                     int resultSetType,
                                                     int resultSetConcurrency) 
                                                     throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.PreparedStatement prepareStatement(String sql,
                                                     int resultSetType,
                                                     int resultSetConcurrency,
                                                     int resultSetHoldability) 
                                                     throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  
  @Override
  public java.sql.Statement createStatement() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.Statement createStatement(int resultSetType,
                                            int resultSetConcurrency) 
                                            throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }
  
  @Override
  public java.sql.Statement createStatement(int resultSetType,
                                            int resultSetConcurrency,
                                            int resultSetHoldability) 
                                            throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public java.sql.CallableStatement prepareCall(String sql) 
                                                throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.CallableStatement prepareCall(String sql,
                                                int resultSetType,
                                                int resultSetConcurrency) 
                                                throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.CallableStatement prepareCall(String sql,
                                                int resultSetType,
                                                int resultSetConcurrency,
                                                int resultSetHoldability) 
                                                throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public java.sql.Savepoint setSavepoint()
                                         throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public java.sql.Savepoint setSavepoint(String name)
                                         throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void releaseSavepoint(java.sql.Savepoint savepoint)
                               throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void rollback() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void rollback(java.sql.Savepoint savepoint) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void commit() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }
  

  @Override
  public java.sql.SQLWarning getWarnings() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void clearWarnings() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public int getTransactionIsolation() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public void setTransactionIsolation(int level) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public void setTypeMap(Map<String,Class<?>> map) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public Map<String,Class<?>> getTypeMap() throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }


  @Override
  public boolean isWrapperFor(Class<?> iface) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }

  @Override
  public <T> T unwrap​(Class<T> iface) throws java.sql.SQLException {
    throw new java.sql.SQLFeatureNotSupportedException();
  }



  private Properties properties;
  private Client     m_client;

}

