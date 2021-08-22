---
title: Ruby
sort: 4
---


# Using SWC-DB Ruby Thrift Client
The gem-package `swcdb` has the SWC-DB ```thrift``` module which consist of modules ```service``` and the default SWC-DB client.

## The Methods
The class `Swcdb::Thrift::Client` inherits the [SWC-DB Thrift Service]({{ site.baseurl }}/use/thriftclient/#service-service) and all the methods/functions in the SWC-DB Thrift Service are available in the ruby client. The methods with prefix ```.sql_``` require the structual syntax of the [SWC-DB SQL]({{ site.baseurl }}/use/sql/).


## The Object Types
All the Objects **[Enumerations]({{ site.baseurl }}/use/thriftclient/#enumerations), [Type-Declarations]({{ site.baseurl }}/use/thriftclient/#type-declarations), [Struct & Exception]({{ site.baseurl }}/use/thriftclient/#data-structures)** are as [The SWC-DB Thrift Modules]({{ site.baseurl }}/use/thriftclient/#the-swc-db-thrift-modules) and available in the module `Swcdb::Thrift::Gen::Service`.



## Before Using
Before you can start using the SWC-DB Ruby Package you need as [by instructions to Install the package]({{ site.baseurl }}/install/thrift_clients/#install-the-swc-db-ruby-package).


## An Exmaple with using `irb`
Enter Ruby Command Line:
```bash
irb
```

> Print schemas with the cids 1,2,3,4 - `swcdbThriftBroker` assumed to be listening on `localhost:18000`
```ruby
require 'swcdb/thrift/client'
client = Swcdb::Thrift::Client.new("localhost", 18000)
schemas = client.sql_list_columns("get schemas 1,2,3,4")
```
