---
title: Setting up swcdb_cluster
sort: 3
---

# Setting up swcdb_cluster


### **swcdb_cluster uses fabric python-package**
Install the `fabric` python-package:
```bash
python3 -m pip install setuptools fabric;
```


### **swcdb_cluster requires password-less ssh authentication**
Create a RSA-Key:
```bash
ssh-keygen -t rsa;
```

To pre-authorize known localhosts-addresses in-order to skip manual host key approval:
```bash
cat /root/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys;
ssh-keyscan -t rsa localhost,ip6-localhost,localhost.localdomain,::1,::,127.0.0.1 >> ~/.ssh/known_hosts;
```

#### Known Issues
  * On execution ```paramiko.ssh_exception.SSHException``` exception is raised with a message "key cannot be used for signing" or an "Unknown/Unsupported" Key or not a valid RSA private key file.
  > The reason can be badly-configured or broken packages distributed by the OS, an immediate possible solution is to uninstall the packages installed by the OS package manager (on Ubuntu with `apt-get remove python-paramiko python-fabric`, on Archlinux `pacman -R python-paramiko python-fabric`) and to install the packages available with pip.
