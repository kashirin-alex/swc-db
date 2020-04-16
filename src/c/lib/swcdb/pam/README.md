


### Pluggable Authentication Module (PAM): "pam_swcdb_max_retries.so"

##### The pam_swcdb_maxretries.so Moudle Objectives are:
  * Authentications limits by the Remote Address
  * Max Tries Limit applied inclusivly between sessions (to passwords and keys )
  * Automatic TTL based release. 
  * Cluster-wide bounding state.
  *  <>  username is not in the consideration for limit


#### PAM services configurations:
The module can be and should be the first in the service groups

#####  AUTH:
Applicable PAM config file ```/etc/pam.d/common-auth```
```bash
auth required pam_swcdb_maxretries.so -h HOST -p PORT -t 30000 -c COLM -k "['ssh', '%s']" -m 10
```

#####  SESSION:
Applicable PAM config file ```/etc/pam.d/common-session```
```bash
session required pam_swcdb_maxretries.so -h HOST -p PORT -t 30000 -c COLM -k "['ssh', '%s']"
```

######  Configuration Options:
    * host      [-h]  SWC-DB Thrift Broker host                                      (=localhost)
    * port      [-p]  SWC-DB Thrift Broker port                                      (=18000)
    * timeout   [-t]  SWC-DB Thrift Client timeout in ms                             (=30000)
    * column    [-c]  SWC-DB column-name to use                                      (=REQUIRED)
    * key       [-k]  fillin fractions, requires one '%s' arg for Remote IP address  (=["%s"])
    * maxtries  [-m]- Maximum Allowed Tries Count before Auth return PAM_MAXTRIES    (=10)


#### The Module Proccess and Requirments:

  #### Requires
  Column schema required to be a COUNTER type and if desired with TTL.


  ##### At PAM authenticate: 
    * On no remote host or NULL, return is PAM_PERM_DENIED
    * On bad configurations, missing Column, return is PAM_SUCCESS
    * On MaxTries confirm check, 
        if Remote Address count tries is -ge, return is PAM_MAXTRIES else PAM_SUCCESS
        and Increment a try count
  

  ##### At PAM session open:
    * On no remote host or NULL, return is PAM_SESSION_ERR
    * On bad configurations, missing Column, return is PAM_SUCCESS
    * Try count is set to Zero and return is PAM_SUCCESS
     
     
  ##### At Exceptions:
    * on ThriftClient error (connection/exceptions),  return is PAM_SUCCESS


#### Logging (syslog/authlog):

  Logs in authlog, starting with "SWC-DB PAM: " keeping track on
  * Queries (updates & selects)
  * Thrift Client Exceptions


