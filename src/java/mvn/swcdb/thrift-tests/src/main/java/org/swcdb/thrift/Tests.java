/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


package org.swcdb.thrift;

import org.apache.thrift.TException;

import org.swcdb.thrift.Client;
import org.swcdb.thrift.gen.SpecSchemas;
import org.swcdb.thrift.gen.Schema;

import java.util.List;

public class Tests {
  
  public static void main(String [] args) {    

    Client client = null;

    try {
      client = Client.create("localhost", 18000, true);
      

      SpecSchemas spec = new SpecSchemas();
      List<Schema> schemas = client.list_columns(spec);
      
      for (Schema schema : schemas) {
        System.out.println(schema.toString());
      }

    } catch (org.swcdb.thrift.gen.Exception e) { 
      e.printStackTrace();
      System.exit(1);

    } catch (org.apache.thrift.TException e) { 
      e.printStackTrace();
      System.exit(1);

    } catch (java.lang.Exception e) { 
      e.printStackTrace();
      System.exit(1);
    }

    System.out.println("--- OK! --- ");
  }
  
}
