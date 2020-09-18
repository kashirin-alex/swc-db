---
title: Setting up swcdb_cluster
sort: 3
---

# Setting up swcdb_cluster


* **swcdb_cluster uses fabric python-package**

```bash
python3 -m pip install setuptools fabric;
```


* **swcdb_cluster requires password-less ssh authentication**

```bash 
ssh-keygen -t rsa;
```

```bash 
cat /root/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys;
ssh-keyscan -t rsa localhost,ip6-localhost,localhost.localdomain,::1,::,127.0.0.1 >> ~/.ssh/known_hosts;
```
