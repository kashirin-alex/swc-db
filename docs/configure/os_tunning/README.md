---
title: OS Tunning
sort: 10
---


# Configuring & Tunning the Operating System 

* [Useable Ports by Kernel](#useable-ports-by-kernel)
* [Accessible Network](#accessible-network)
* [Memory Performance](#memory-performance)


*****


## Useable Ports by Kernel

To let the Kernel use the up most ports, the recommendation is to make all port useable and reserve the nominated services ports. That can be achieved by applying configrations to the `/etc/sysctl.conf`.

```bash
# SET WELLKNOWN UNUSED PORTS AVAILABLE
"net.ipv4.ip_unprivileged_port_start = 0"  >> /etc/sysctl.conf;
```

```bash
# LET ANY PORT TO BE AVAILABLE FOR KERNEL USE ASSIGMENT
"net.ipv4.ip_local_port_range = 1 65535" >> /etc/sysctl.conf;
```

```bash
# DEFINE THE PORTS TO RESERVE - THE PORT-SERVICE DEPENDS ON YOUR SYSTEM DESIGNATION

### KNOWN SYSTEM COMMON
RESERVED_PORTS="";                                        # 
RESERVED_PORTS="${RESERVED_PORTS},22"                     # SSH-SERVER
RESERVED_PORTS="${RESERVED_PORTS},53"                     # LOCAL/PUBLIC DNS

### SWC-DB RELATED
RESERVED_PORTS="${RESERVED_PORTS},15000"                  # SWCDB-MANAGER
RESERVED_PORTS="${RESERVED_PORTS},16000"                  # SWCDB-RANGER
RESERVED_PORTS="${RESERVED_PORTS},17000"                  # SWCDB-FSBROKER
RESERVED_PORTS="${RESERVED_PORTS},18000"                  # SWCDB-THRIFTBROKER

### APACHE-HADOOP RELATED
RESERVED_PORTS="${RESERVED_PORTS},2181,2888,3888"         # ZOOKEEPER
RESERVED_PORTS="${RESERVED_PORTS},8040,8042"              # NODE-MANAGER
RESERVED_PORTS="${RESERVED_PORTS},8030-8033,8088,8090"    # RESOURCE-MANAGER
RESERVED_PORTS="${RESERVED_PORTS},8485"                   # QUORUM-JOURNAL-MANAGER
RESERVED_PORTS="${RESERVED_PORTS},50100,50105"            # HADOOP-BACKUP-NODE
RESERVED_PORTS="${RESERVED_PORTS},50010,50020,50075"      # HADOOP-DATA-NODE
RESERVED_PORTS="${RESERVED_PORTS},9000,50070"             # HADOOP-NAME-NODE
RESERVED_PORTS="${RESERVED_PORTS},50090"                  # HADOOP-SECONDARYNAME-NODE
RESERVED_PORTS="${RESERVED_PORTS},50030"                  # HADOOP-JOBTRACKER
RESERVED_PORTS="${RESERVED_PORTS},50060"                  # HADOOP-TASKTRACKER

# SET THE RESERVED PORTS
"net.ipv4.ip_local_reserved_ports = ${RESERVED_PORTS}" >> /etc/sysctl.conf;
```
_The values used are by known common/default and do not extend beyond the minimal needs to run SWC-DB and HDFS. Add 80,443 to RESERVED_PORTS if your system is going to run http/s server and etc._


*****


## Accessible Network
[nftables(NetFilter Tables)](https://netfilter.org/projects/nftables/) is the recommend application(Firewall) to manage the networking access.

### ***The Recommended nftables Rules to Apply:***

#### 1. Each Server Define it's own origin
***Host's Defintion Rules-File: /YourPathTo/nftables/host.nft***
```cpp
define host_ipv4 = {
  THIS-SERVER'S-IPv4-1,
  THIS-SERVER'S-IPv4-1
};
define host_ipv6 = {
  THIS-SERVER'S-IPv6-1,
  THIS-SERVER'S-IPv6-2
};

define allowed_udp_ports = {53};
define allowed_tcp_ports = {22,80,443,853};
```
_The allowed ports are the Services Ports that should be accessible/open to the wide-network._


#### 2. Each Server set with the Definitions of the whole cluster
***Cluster's IPv4 Rules-File:  /YourPathTo/nftables/cluster-ipv4.nft***
```cpp
set cluster_ipv4 {
  type ipv4_addr
  elements = {
    FILL-THE-IPv4s-1,
    FILL-THE-IPv4s-2
  }
};
```

***Cluster's IPv6 Rules-File: /YourPathTo/nftables/cluster-ipv6.nft***
```cpp
set cluster_ipv6 {
  type ipv6_addr
  elements = {
    FILL-THE-IPv6s-1,
    FILL-THE-IPv6s-2
  }
};
```


#### 3. The Networking Rules to the Cluster corelation
***The Rules-File: /YourPathTo/nftables/rules.nft*** To be used for loading nft-rules `nft -f /YourPathTo/nftables/rules.nft`
```cpp
flush ruleset;

include "/YourPathTo/nftables/host.nft";
  
table inet filter {

  chain host_out {
    ip  saddr 127.0.0.1/8 goto loc_state
    ip6 saddr ::1/128  goto loc_state
    ip  saddr $host_ipv4 goto loc_state
    ip6 saddr $host_ipv6 goto loc_state
  }

  include "/YourPathTo/nftables/cluster-ipv4.nft";
  include "/YourPathTo/nftables/cluster-ipv6.nft";
  
  chain host {
    ip  saddr 127.0.0.1/8 ip daddr 127.0.0.1/8 goto loc_state
    ip6 saddr ::1/128 ip6 daddr ::1/128 goto loc_state
    
    ip  saddr $host_ipv4 ip  daddr $host_ipv4 goto loc_state
    ip6 saddr $host_ipv6 ip6 daddr $host_ipv6 goto loc_state
    
    ip  saddr $host_ipv4 ip daddr 127.0.0.1/8 goto loc_state
    ip  saddr 127.0.0.1/8 ip daddr $host_ipv4 goto loc_state
    ip6 saddr $host_ipv6 ip6 daddr ::1/128 goto loc_state
    ip6 saddr ::1/128 ip6 daddr $host_ipv6 goto loc_state
  }
  chain loc_state {
    ct state invalid reject;
    accept;
  }
  chain input {
    type filter hook input priority 0; policy drop;
    ct state established, related accept;
    
    iifname lo goto host
    jump host
    
    ip  saddr @cluster_ipv4 goto loc_state
    ip6 saddr @cluster_ipv6 goto loc_state  
    
    ct state invalid drop;
    ip  daddr != $host_ipv4 counter log prefix "IPv4_IN_DROPPED" drop
    ip6 daddr != $host_ipv6 counter log prefix "IPv6_IN_DROPPED" drop
    
    ip protocol icmp   icmp type   { echo-request, destination-unreachable, router-advertisement, time-exceeded, parameter-problem } accept
    ip6 nexthdr icmpv6 icmpv6 type { echo-request, destination-unreachable, packet-too-big, time-exceeded, parameter-problem, nd-router-advert, nd-neighbor-solicit, nd-neighbor-advert } accept

    udp dport $allowed_udp_ports accept
    tcp dport $allowed_tcp_ports accept
  }
  chain output {
    type filter hook output priority 0; policy drop;
    ct state established, related accept;
    ct state invalid reject
    goto host_out
  }
  chain forward {
    type filter hook forward priority 0; policy accept;
    ct state established, related accept;
    ct state invalid reject
    ip  saddr @cluster_ipv4 ip  daddr @cluster_ipv4 accept
    ip6 saddr @cluster_ipv6 ip6 daddr @cluster_ipv6 accept
    goto host
  }
};
```


#### 4. Confirm The NetFilter Tables Rules are correct and loaded
* Execute load Rules: \
```nft -f /YourPathTo/nftables/rules.nft;```
* Enter `nft` in Interactive mode. \
```nft -i;```
* List the Rules \
```nft> list ruleset;```


#### 5. Applying the nftables rules with the system start
Create and Apply '/etc/init.d/nft' with nft loading the Cluster Rules.
```bash
echo '#! /bin/sh' > /etc/init.d/nft;
echo "nft -f /YourPathTo/nftables/rules.nft;" >> /etc/init.d/nft;
echo "exit 0;" >> /etc/init.d/nft;
chmod 755 /etc/init.d/nft;
```


*****


## Memory Performance
Keep the Database (memory-pages) in RSS without using file-backed pages swapping.
```bash
echo "vm.swappiness = 0" >> /etc/sysctl.conf;
```

