/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/Encoder.h"
#include "swcdb/db/client/Settings.h"

namespace SWC{ namespace Config {

void Settings::init_client_options() {
  cmdline_desc.add_options()
    ("swc.cfg.dyn.period", g_i32(600000),
     "Dynamic cfg-file check interval in ms, zero without")

    ("swc.mngr.host", g_strs(Strings()),
     "Manager Host: \"[cols range]|(hostname or ips-csv)|port\"")
    ("swc.mngr.port", i16(15000),
     "Manager default port if not defined in swc.mngr.host")
    ("swc.rgr.port", i16(16000), "Ranger port")

    ("swc.bkr.host", g_strs(Strings()),
     "Broker Host: \"(hostname or ips-csv)|port\"")
    ("swc.bkr.port", i16(17000), "Broker port")

    ("swc.client.Bkr.connection.timeout", g_i32(10000),
     "Broker client connect timeout")
    ("swc.client.Bkr.connection.probes", g_i32(1),
     "Broker client connect probes")
    ("swc.client.Bkr.connection.keepalive", g_i32(30000),
     "Broker client connection keepalive for ms since last action")

    ("swc.client.Bkr.comm.encoder",
      g_enum(
        int(SWC_DEFAULT_COMM_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")

    ("swc.client.Rgr.connection.timeout", g_i32(10000),
     "Ranger client connect timeout")
    ("swc.client.Rgr.connection.probes", g_i32(1),
     "Ranger client connect probes")
    ("swc.client.Rgr.connection.keepalive", g_i32(30000),
     "Ranger client connection keepalive for ms since last action")

    ("swc.client.Rgr.comm.encoder",
      g_enum(
        int(SWC_DEFAULT_COMM_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")

    ("swc.client.Rgr.range.res.expiry", g_i32(1800000),
     "Range Ranger resolution expiry in ms")

    ("swc.client.Mngr.connection.timeout", g_i32(10000),
     "Manager client connect timeout")
    ("swc.client.Mngr.connection.probes", g_i32(1),
     "Manager client connect probes")
    ("swc.client.Mngr.connection.keepalive", g_i32(30000),
     "Manager client connection keepalive for ms since last action")

    ("swc.client.Mngr.comm.encoder",
      g_enum(
        int(SWC_DEFAULT_COMM_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")

    ("swc.client.Mngr.range.master.expiry", g_i32(1800000),
     "Cached Master Range expiry in ms")

    ("swc.client.schema.expiry", g_i32(1800000),
     "Schemas expiry in ms")

    ("swc.client.send.buffer", g_i32(8388608),
     "Client send buffer size in bytes")
    ("swc.client.send.ahead", g_i8(3),
     "Client send number of buffers ahead")
    ("swc.client.send.timeout", g_i32(800000),
     "Client send timeout in ms")
    ("swc.client.send.timeout.bytes.ratio", g_i32(1000),
     "Timeout ratio to bytes, bytes/ratio=ms added to send timeout")

    ("swc.client.request.again.delay", g_i32(500),
     "Client request again delay size in ms")

    ("swc.client.recv.buffer", g_i32(8388608),
     "Client receive buffer size in bytes")
    ("swc.client.recv.ahead", g_i8(3),
     "Client receive number of buffers ahead")
    ("swc.client.recv.timeout", g_i32(800000),
     "Client receive timeout in ms")
  ;
}


}} // namespace SWC::Config
