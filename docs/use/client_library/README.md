---
title: The C++ Client/API (libswcdb)
sort: 4
---



# The SWC-DB C++ Client/API (libswcdb)
The SWC-DB C++ Client API consists of all the required Types and Functions to manage columns, make queries and work with data-types. Applications are required to be built with `-I` having the `swcdb/` on the path and linked with `-lswcdb` or with the tree of dependencies for a static-linkage.

***



### The SWC-DB Initialization and Configuration Instance
The initialization and configuration is applied with an instance of [SWC::Config::Settings](https://cpp.swcdb.org/classSWC_1_1Config_1_1Settings.html).
As the Settings instance life-time is required to be through the application runtime, it is strongly suggested to use a continues storage for it or use a managing class  [SWC::Env::Config](https://cpp.swcdb.org/classSWC_1_1Env_1_1Config.html) of the singleton Settings instance.
* _a User managed continual storage:_
  ```cpp
  //#include "swcdb/core/config/Settings.h"
  #include "swcdb/db/client/Clients.h"

  int main(int argc, char** argv) {
    SWC::Config::Settings::Ptr setting(new SWC::Config::Settings());
    setting->init(argc, argv, nullptr, nullptr);
    // ...
  }
  ```
* _a SWC::Env::Config managed storage:_
  ```cpp
  //#include "swcdb/core/config/Settings.h"
  #include "swcdb/db/client/Clients.h"

  int main(int argc, char** argv) {
    SWC::Env::Config::init(argc, argv, nullptr, nullptr);
    auto setting = SWC::Env::Config::settings();
    // ...
  }
  ```

> Users can add custom configuration properties with [`SWC::Config::Settings::init_option_t`](https://cpp.swcdb.org/classSWC_1_1Config_1_1Settings.html#ac6281d4ca7c7c6d46bf75db0c88b1b1c) a static function `typedef void SWC::Config::Settings::init_option_t(Settings *)`. Commonly used in SWC-DB applications as `(argc, argv, &init_app_options, &init_post_cmd_args)` or without new properties by applying nullptr.

The SWC::Config::Settings instance can be populated with all (dependent on run time requirements) the required configuration properties without using the [`.cfg` files]({{ site.baseurl }}/configure/the_config_files/#the-roles-and-cfg-filename).

***



### The SWC-DB Clients
The [SWC::client::Clients](https://cpp.swcdb.org/classSWC_1_1client_1_1Clients.html) instance manages the io-contexts, schemas cache, servers resolutions to role/range and the connections queues for the SWC-DB Manager, Ranger and Broker services. The required life-time of an instance is through the usage of the services. A singleton instance for the purpose is the [`SWC::Env::Clients::init(const SWC::client::Clients::Ptr&)`](https://cpp.swcdb.org/classSWC_1_1Env_1_1Clients.html). The Clients constructor has 3 overloaders separated by the combinations of services to initialize Manager+Ranger+Broker, Manager+Ranger or only Broker

```cpp
#include "swcdb/db/client/Clients.h"

int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv, nullptr, nullptr);

  bool with_broker = false; // a Common config in tests and examples

  SWC::Env::Clients::init(
    (with_broker
      ? SWC::client::Clients::make(
          *SWC::Env::Config::settings(),
          SWC::Comm::IoContext::make("Clients", 8),
          nullptr  // std::make_shared<client::BrokerContext>()
        )
      : SWC::client::Clients::make(
          *SWC::Env::Config::settings(),
          SWC::Comm::IoContext::make("Clients", 8),
          nullptr, // std::make_shared<client::ManagerContext>()
          nullptr  // std::make_shared<client::RangerContext>()
        )
    )->init()
  );

  //..
  SWC::client::Clients::Ptr clients = SWC::Env::Clients::get();
  //..

  SWC::Env::Clients::get()->stop();
  return 0;
}
```

The SWC-DB Clients Instance is required by most of [protocols commands'](https://cpp.swcdb.org/db_2Protocol_2Commands_8h.html) requests classes in the namespaces `SWC::Comm::Protocol::{Bkr,Mngr,Rgr}::Req::` and by the pre-defined query handlers in namespace [`SWC::client::Query::`](https://cpp.swcdb.org/namespaceSWC_1_1client_1_1Query.html).

#### The available Request classes by protocol service:
* [SWC-DB Broker](https://cpp.swcdb.org/namespaceSWC_1_1Comm_1_1Protocol_1_1Bkr_1_1Req.html)
* [SWC-DB Manager](https://cpp.swcdb.org/namespaceSWC_1_1Comm_1_1Protocol_1_1Mngr_1_1Req.html)
* [SWC-DB Ranger](https://cpp.swcdb.org/namespaceSWC_1_1Comm_1_1Protocol_1_1Rgr_1_1Req.html)

#### The available Query handlers:
* [Select](https://cpp.swcdb.org/namespaceSWC_1_1client_1_1Query_1_1Select.html)
  * [Scanner](https://cpp.swcdb.org/classSWC_1_1client_1_1Query_1_1Select_1_1Scanner.html)
  * [Broker-Scanner](https://cpp.swcdb.org/classSWC_1_1client_1_1Query_1_1Select_1_1BrokerScanner.html)
* [Update](https://cpp.swcdb.org/namespaceSWC_1_1client_1_1Query_1_1Update.html)
  * [Commiter](https://cpp.swcdb.org/classSWC_1_1client_1_1Query_1_1Update_1_1Committer.html)
  * [Broker-Commiter](https://cpp.swcdb.org/classSWC_1_1client_1_1Query_1_1Update_1_1BrokerCommitter.html)
  * [Metric-Reporting](https://cpp.swcdb.org/classSWC_1_1client_1_1Query_1_1Update_1_1Handlers_1_1Metric_1_1Reporting.html)

***



## _The documentations are continuously in progress!_

#### More information can be found at:
* [SWC::DB namespace](https://cpp.swcdb.org/namespaceSWC_1_1DB.html)
* [SWC::client::Clients class](https://cpp.swcdb.org/classSWC_1_1client_1_1Clients.html)
* [SWC::Comm::Protocol namespace](https://cpp.swcdb.org/namespaceSWC_1_1Comm_1_1Protocol.html)
* [SWC-DB Examples](https://github.com/kashirin-alex/swc-db/tree/master/examples)
* [SWC-DB Client Tests](https://github.com/kashirin-alex/swc-db/tree/master/tests/integration/client)
* [SWC-DB Manager Tests](https://github.com/kashirin-alex/swc-db/tree/master/tests/integration/manager)
* [Query Select - Implementation Cases](https://github.com/search?q=client%3A%3AQuery%3A%3ASelect%3A%3AHandlers%3A%3A+repo%3Akashirin-alex%2Fswc-db+language%3AC%2B%2B+language%3AC%2B%2B&type=Code)
* [Query Update - Implementation Cases](https://github.com/search?p=2&q=client%3A%3AQuery%3A%3AUpdate%3A%3AHandlers%3A%3A+repo%3Akashirin-alex%2Fswc-db+language%3AC%2B%2B+language%3AC%2B%2B&type=Code)

