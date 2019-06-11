/*
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/** @file
 * Internet address wrapper classes and utility functions.
 * The Endpoint structure manages a hostname:port pair, and the
 * InetAddr class wraps the sockaddr_in structure.
 */

#include "Compat.h"
#include "Serialization.h"

#include <cstdlib>
#include <cstring>

extern "C" {
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sigar.h>
}

#include "Logger.h"
#include "InetAddr.h"
#include "StringExt.h"

namespace Hypertable {

InetAddr::InetAddr() {
  HT_EXPECT(sizeof(sockaddr_in) == sizeof(InetAddr), Error::UNPOSSIBLE);
  memset(this, 0, sizeof(InetAddr));
}

InetAddr::InetAddr(const String &host, uint16_t port) {
  HT_EXPECT(sizeof(sockaddr_in) == sizeof(InetAddr), Error::UNPOSSIBLE);
  HT_EXPECT(initialize(this, host.c_str(), port), Error::BAD_DOMAIN_NAME);
}

InetAddr::InetAddr(const String &endpoint) {
  HT_EXPECT(sizeof(sockaddr_in) == sizeof(InetAddr), Error::UNPOSSIBLE);
  HT_EXPECT(initialize(this, endpoint.c_str()), Error::BAD_DOMAIN_NAME);
}

InetAddr::InetAddr(uint32_t ip32, uint16_t port) {
  HT_EXPECT(sizeof(sockaddr_in) == sizeof(InetAddr), Error::UNPOSSIBLE);
  initialize(this, ip32, port);
}

bool InetAddr::initialize(sockaddr_in *addr, const char *host, uint16_t port) {
  memset(addr, 0, sizeof(struct sockaddr_in));

  if (parse_ipv4(host, port, *addr)) {
    return true;
  }
  else {
#if defined(__linux__)
    // Let's hope this is not broken in the glibc we're using
    struct hostent hent, *he = 0;
    char hbuf[2048];
    int err;

    if (gethostbyname_r(host, &hent, hbuf, sizeof(hbuf), &he, &err) != 0
       || he == 0) {
      HT_ERRORF("gethostbyname '%s': error: %d", host, err);
      return false;
    }
#elif defined(__APPLE__) || defined(__sun__) || defined(__FreeBSD__)
    // This is supposed to be safe on Darwin (despite the man page)
    // and FreeBSD, as it's implemented with thread local storage.
    struct hostent *he = gethostbyname(host);

    if (he == 0) {
      String errmsg = (String)"gethostbyname(\"" + host + "\")";
      herror(errmsg.c_str());
      HT_THROW(Error::BAD_DOMAIN_NAME, errmsg);
      return false;
    }
#else
#  error TODO please implement me!
#endif
    memcpy(&addr->sin_addr.s_addr, he->h_addr_list[0], sizeof(uint32_t));
    if (addr->sin_addr.s_addr == 0) {
      uint8_t *ip = (uint8_t *)&addr->sin_addr.s_addr;
      ip[0] = 127;
      ip[3] = 1;
    }
  }
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  return true;
}

bool
InetAddr::is_ipv4(const char *ipin) {
  const char *ptr = ipin, *end = ipin + strlen(ipin);
  char *last;
  int component=0;
  int num_components=0;
  int base=10;

  while(ptr < end) {
    component = strtol(ptr, &last, base);
    num_components++;
    if (last == end)
      break;
    if (*last != '.' || last > end || component > 255 || component < 0 || num_components > 4)
      return false;
    ptr = last + 1;
  }
  if (num_components != 4 || component > 255 || component < 0)
    return false;
  return true;
}

bool
InetAddr::parse_ipv4(const char *ipin, uint16_t port, sockaddr_in &addr,
                     int base) {
  uint8_t *ip = (uint8_t *)&addr.sin_addr.s_addr;
  const char *ipstr = ipin, *end = ipin + strlen(ipin);
  char *last;
  int64_t n = strtoll(ipstr, &last, base);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (last == end && n > 0 && n < UINT32_MAX) {
    addr.sin_addr.s_addr = htonl(n);
    return true;
  }
  *ip++ = n;

  if (last > end || *last != '.')
    return false;

  ipstr = last + 1;
  *ip++ = strtol(ipstr, &last, base);

  if (last >= end || *last != '.')
    return false;

  ipstr = last + 1;
  *ip++ = strtol(ipstr, &last, base);

  if (last >= end || *last != '.')
    return false;

  ipstr = last + 1;
  *ip++ = strtol(ipstr, &last, base);

  if (last != end)
    return false;

  if (addr.sin_addr.s_addr == 0) {
    uint8_t *ip = (uint8_t *)&addr.sin_addr.s_addr;
    ip[0] = 127;
    ip[3] = 1;
  }

  return true;
}

Endpoint InetAddr::parse_endpoint(const char *endpoint, int default_port) {
  const char *colon = strchr(endpoint, ':');

  if (colon) {
    String host = String(endpoint, colon - endpoint);
    return Endpoint(host, atoi(colon + 1));
  }
  return Endpoint(endpoint, default_port);
}

bool InetAddr::initialize(sockaddr_in *addr, const char *addr_str) {
  Endpoint e = parse_endpoint(addr_str);

  if (e.port)
    return initialize(addr, e.host.c_str(), e.port);

  return initialize(addr, "localhost", atoi(addr_str));
}

bool InetAddr::initialize(sockaddr_in *addr, uint32_t haddr, uint16_t port) {
  memset(addr, 0 , sizeof(sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(haddr);
  addr->sin_port = htons(port);
  return true;
}

String InetAddr::format(const sockaddr_in &addr, int sep) {
  // inet_ntoa is not thread safe on many platforms and deprecated
  const uint8_t *ip = (uint8_t *)&addr.sin_addr.s_addr;
  return Hypertable::format("%d.%d.%d.%d%c%d", (int)ip[0], (int)ip[1],
      (int)ip[2],(int)ip[3], sep, (int)ntohs(addr.sin_port));
}

String InetAddr::format_ipaddress(const sockaddr_in &addr) {
  // inet_ntoa is not thread safe on many platforms and deprecated
  const uint8_t *ip = (uint8_t *)&addr.sin_addr.s_addr;
  return Hypertable::format("%d.%d.%d.%d", (int)ip[0], (int)ip[1],
			    (int)ip[2], (int)ip[3]);
}

String InetAddr::hex(const sockaddr_in &addr, int sep) {
  return Hypertable::format("%x%c%x", ntohl(addr.sin_addr.s_addr), sep,
                            ntohs(addr.sin_port));
}


size_t InetAddr::encoded_length() const {
  size_t length = encoded_length_internal();
  return 1 + Serialization::encoded_length_vi32(length) + length;
}

/**
 * @details
 * Encoding is as follows:
 * <table>
 * <tr>
 * <th>Encoding</th>
 * <th>Description</th>
 * </tr>
 * <tr>
 * <td>1 byte</td>
 * <td>Encoding version as returned by encoding_version()</td>
 * </tr>
 * <tr>
 * <td>vint</td>
 * <td>Length of encoded object as returned by encoded_length_internal()</td>
 * </tr>
 * <tr>
 * <td>variable</td>
 * <td>Object encoded with encode_internal()</td>
 * </tr>
 * </table>
 */
void InetAddr::encode(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, encoding_version());
  Serialization::encode_vi32(bufp, encoded_length_internal());
  encode_internal(bufp);
}

void InetAddr::decode(const uint8_t **bufp, size_t *remainp) {
  uint8_t version = Serialization::decode_i8(bufp, remainp);
  if (version > encoding_version())
    HT_THROWF(Error::PROTOCOL_ERROR, "Unsupported InetAddr version %d", (int)version);
  size_t encoding_length = Serialization::decode_vi32(bufp, remainp);
  const uint8_t *end = *bufp + encoding_length;
  size_t tmp_remain = encoding_length;
  decode_internal(version, bufp, &tmp_remain);
  HT_ASSERT(*bufp <= end);
  *remainp -= encoding_length;
  // If encoding is longer than we expect, that means we're decoding a newer
  // version, so skip the newer portion that we don't know about
  if (*bufp < end)
    *bufp = end;
}

void InetAddr::legacy_decode(const uint8_t **bufp, size_t *remainp) {
  Serialization::decode_i8(bufp, remainp);
  sin_family = Serialization::decode_i8(bufp, remainp);
  sin_port = Serialization::decode_i16(bufp, remainp);
  sin_addr.s_addr = Serialization::decode_i32(bufp, remainp);
}

uint8_t InetAddr::encoding_version() const {
  return 1;
}

size_t InetAddr::encoded_length_internal() const {
  return 8;
}

/// @details
/// Encoding is as follows:
/// <table>
/// <tr>
/// <th>Encoding</th>
/// <th>Description</th>
/// </tr>
/// <tr><td>i8</td><td>sizeof(sockaddr_in)</td></tr>
/// <tr><td>i8</td><td>Address family (sin_family)</td></tr>
/// <tr><td>i16</td><td>Port</td></tr>
/// <tr><td>i16</td><td>Address (sin_addr.s_addr)</td></tr>
/// </table>
void InetAddr::encode_internal(uint8_t **bufp) const {
  *(*bufp)++ = sizeof(sockaddr_in);
  *(*bufp)++ = sin_family;
  Serialization::encode_i16(bufp, sin_port);
  Serialization::encode_i32(bufp, sin_addr.s_addr);
}

void InetAddr::decode_internal(uint8_t version, const uint8_t **bufp,
                                 size_t *remainp) {
  Serialization::decode_i8(bufp, remainp);
  sin_family = Serialization::decode_i8(bufp, remainp);
  sin_port = Serialization::decode_i16(bufp, remainp);
  sin_addr.s_addr = Serialization::decode_i32(bufp, remainp);
}


std::ostream &operator<<(std::ostream &out, const Endpoint &e) {
  out << e.host <<':'<< e.port;
  return out;
}

std::ostream &operator<<(std::ostream &out, const sockaddr_in &a) {
  out << InetAddr::format(a);
  return out;
}

} // namespace Hypertable
