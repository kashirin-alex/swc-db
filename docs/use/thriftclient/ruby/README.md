---
title: Ruby
sort: 4
---


# Using SWC-DB Ruby Thrift Client
The gem-package `swcdb` has the SWC-DB ```thrift``` module which consist of modules ```service``` and the default SWC-DB client.

## The Methods
The class `Swcdb::Thrift::Client` inherits the [SWC-DB Thrift Service]({{ site.baseurl }}/use/thriftclient/#service-service) and all the methods/functions in the SWC-DB Thrift Service are available in the ruby client. The methods with prefix ```.sql_``` require the structual syntax of the [SWC-DB SQL]({{ site.baseurl }}/use/sql/).


## The Object Types
All the Objects **[Enumerations]({{ site.baseurl }}/use/thriftclient/#enumerations), [Type-Declarations]({{ site.baseurl }}/use/thriftclient/#type-declarations), [Struct & Exception]({{ site.baseurl }}/use/thriftclient/#data-structures)** are as [The SWC-DB Thrift Modules]({{ site.baseurl }}/use/thriftclient/#the-swc-db-thrift-modules) and available in the module `Swcdb::Thrift::Gen`.



## Before Using
Before you can start using the SWC-DB Ruby Package you need as [by instructions to Install the package]({{ site.baseurl }}/install/thrift_clients/#install-the-swc-db-ruby-package).


## An Exmaple with using `irb`
Enter Ruby CLI (in quiet mode):
```bash
irb --noecho
```

> Print schemas with the cids 1,2,3,4 - `swcdbThriftBroker` assumed to be listening on `localhost:18000`
```ruby
require 'swcdb/thrift/client'

client = Swcdb::Thrift::Client.new("localhost", 18000)

schemas = client.sql_list_columns("get schemas 1,2,3,4")

print schemas

client.close()
```

> Scan with Specs and Print SYS_CID_STATS(cid:10) cells with the RSS-Memory field_id:3 (FIELD_RSS_USED_MIN) above 2MB.
```ruby
require 'swcdb/thrift/client'

Service = Swcdb::Thrift::Gen  # use-as Service

specs = Service::SpecScan.new(
  columns_serial: [
    Service::SpecColumnSerial.new(
      cid: 10,
      intervals: [
        Service::SpecIntervalSerial.new(
          key_intervals: [
            Service::SpecKeyInterval.new(
              start: [
                Service::SpecFraction.new(comp: Service::Comp::GE, f: ""),
                Service::SpecFraction.new(comp: Service::Comp::GE, f: ""),
                Service::SpecFraction.new(comp: Service::Comp::GE, f: ""),
                Service::SpecFraction.new(comp: Service::Comp::GE, f: ""),
                Service::SpecFraction.new(comp: Service::Comp::GE, f: ""),
                Service::SpecFraction.new(comp: Service::Comp::EQ, f: "mem")
              ]
            )
          ],
          values: [
            Service::SpecValueSerial.new(
              comp: Service::Comp::EQ,
              fields: [
                Service::SpecValueSerialField.new(
                  field_id: 3,
                  spec_int64:Service::SpecValueSerial_INT64.new(
                    comp: Service::Comp::GT,
                    v: 2
                  )
                )
              ]
            )
          ]
        )
      ]
    )
  ]
)

client = Swcdb::Thrift::Client.new("localhost", 18000)

result = client.scan(specs)

for cell in result.serial_cells do
  print cell.k
  print "\n"
end

client.close()
```
