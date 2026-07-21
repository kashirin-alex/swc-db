---
title: Java
sort: 2
---


# Using SWC-DB Java Thrift Client
The SWC-DB Java Thrift Client is in `package org.swcdb.thrift;` and the Repository is available via Maven Central Repository https://repo1.maven.org/maven2/org/swcdb/thrift and https://oss.sonatype.org/content/groups/public/org/swcdb/thrift

a Maven Project's `pom.xml` requires to add to `<dependencies>`:
```xml
  <dependency>
    <groupId>org.swcdb</groupId>
    <artifactId>thrift</artifactId>
    <version>0.5.13</version>
  </dependency>
```
> Confirm the published artifact version on [Maven Central](https://search.maven.org/artifact/org.swcdb/thrift) — Java client releases can track the source tree (`CMakeLists.txt`) while downloadable server packages lag (see [Getting SWC-DB]({{ site.baseurl }}/install/getting_swcdb/)).

for other project types use the defintions are available at:
* [Maven Central Repository](https://search.maven.org/artifact/org.swcdb/thrift)
* [MVNrepository.com](https://mvnrepository.com/artifact/org.swcdb/thrift/)

When building from this tree with Java enabled, CMake also installs a JDBC driver JAR (`swcdb-jdbc-thrift-*-bundled.jar`). JDBC URL: `jdbc:swcdb:thrift:hostname:18000/`. Install notes: [Thrift Clients]({{ site.baseurl }}/install/thrift_clients/#java--maven--jdbc).


***


#### A Sample Program - list all the schemas:
##### 1) Save to `ListAllSchemas.java`:
```java
/* ListAllSchemas.java */

import org.swcdb.thrift.*;
import org.swcdb.thrift.gen.*;
import org.apache.thrift.TException;


public class ListAllSchemas {


  public static void main(String [] args) {

    Client client = null;

    try {
      client = Client.create("localhost", 18000, true);


      SpecSchemas spec = new SpecSchemas();
      java.util.List<Schema> schemas = client.list_columns(spec);

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
```


##### 2) Define the ClassPaths:
> for the example the JAR dependencies are at `/root/.m2/repository/`
```bash
SWCDB_VERSION="0.5.13"
CLASSPATHS=/root/.m2/repository/org/swcdb/thrift/${SWCDB_VERSION}/thrift-${SWCDB_VERSION}.jar;
CLASSPATHS=${CLASSPATHS}:/root/.m2/repository/org/apache/thrift/libthrift/0.20.0/libthrift-0.20.0.jar;
CLASSPATHS=${CLASSPATHS}:/root/.m2/repository/org/slf4j/slf4j-api/1.7.28/slf4j-api-1.7.28.jar;
```


##### 3) Compile:
```bash
javac -cp ${CLASSPATHS} ListAllSchemas.java
```


##### 4) Run:
```bash
java -cp ${CLASSPATHS}:./ ListAllSchemas
```
> for Real Results SWC-DB cluster needs to be running

