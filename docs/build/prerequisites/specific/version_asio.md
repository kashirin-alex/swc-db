
# ASIO VERSION

_**if libasio-dev under version asio-1.18**_
_use latest version from https://sourceforge.net/projects/asio/_
```bash

mkdir asio; cd asio;
ASIO_VERSION="1.18.0";
wget https://sourceforge.net/projects/asio/files/asio/${ASIO_VERSION}%20%28Stable%29/asio-${ASIO_VERSION}.tar.gz/download \
 -O asio-${ASIO_VERSION}.tar.gz;
tar -xf asio-${ASIO_VERSION}.tar.gz;

ASIO_INCLUDE_PATH="$(pwd)/asio-${ASIO_VERSION}/include";
cd ..;

```


