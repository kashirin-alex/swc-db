

### Debian SWC-DB package

After `cmake` configure and `make`, instead of doing `make install` proceed with `checkinstall`.

Fill the details:
* SWCDB_MAINTAINER="";
* SWCDB_VERSION=;
* PKG_REQUIRES="";
* PKG_RECOMMENDS="";
* PKG_SUGGESTS="";

**_The Described details are of the SWC-DB package [`swcdb-0.5.1.Ubuntu_20_04_2_LTS.amd64.deb`](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.1/swcdb-0.5.1.Ubuntu_20_04_2_LTS.amd64.deb)._**


```bash

apt-get install -y checkinstall;

SWCDB_MAINTAINER="kashirin.alex@gmail.com";
SWCDB_VERSION=0.5.1;
PKG_REQUIRES="libtcmalloc-minimal4, libre2-5, libsnappy1v5, libthrift-0.13.0, libzstd1, libreadline8, libssl1.1";
PKG_RECOMMENDS="python3-fabric";
PKG_SUGGESTS="libthrift-c-glib0, libcephfs2, default-jre";

checkinstall --type=debian --exclude=/usr --pakdir=packages \
  --maintainer=${SWCDB_MAINTAINER} \
  --pkgname=swcdb \
  --provides=swcdb \
  --pkggroup=swcdb \
  --pkgversion=1 \
  --pkgrelease=${SWCDB_VERSION} \
  --pkglicense=GPLv3 \
  --pkgaltsource="https://github.com/kashirin-alex/swc-db/releases" \
  --requires="${PKG_REQUIRES}" \
  --recommends="${PKG_RECOMMENDS}" \
  --suggests="${PKG_SUGGESTS}" \
  ;

os_r=$(cat /usr/lib/os-release | grep "^PRETTY_NAME=" | sed -r 's/(PRETTY_NAME=|\")//g' | sed -r 's/(\.|\s)/_/g' );
mv packages/swcdb_1-${SWCDB_VERSION}_amd64.deb packages/swcdb-${SWCDB_VERSION}.${os_r}.amd64.deb;

```
> The package will be ready at `packages` folder.
* Remove with `dpkg -r swcdb;`
* Install with `dpkg -i swcdb-0.5.1.Ubuntu_20_04_2_LTS.amd64.deb;`
