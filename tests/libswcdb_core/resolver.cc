/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/Resolver.h"

#include <string>
#include <vector>


namespace {


void test_ip_classify() {
  SWC_ASSERT(SWC::Comm::Resolver::is_ipv4_address("127.0.0.1"));
  SWC_ASSERT(SWC::Comm::Resolver::is_ipv4_address("192.168.1.10"));
  SWC_ASSERT(!SWC::Comm::Resolver::is_ipv4_address("not-an-ip"));
  SWC_ASSERT(!SWC::Comm::Resolver::is_ipv4_address("::1"));

  SWC_ASSERT(SWC::Comm::Resolver::is_ipv6_address("::1"));
  SWC_ASSERT(SWC::Comm::Resolver::is_ipv6_address("2001:db8::1"));
  SWC_ASSERT(!SWC::Comm::Resolver::is_ipv6_address("127.0.0.1"));
  SWC_ASSERT(!SWC::Comm::Resolver::is_ipv6_address("hostname"));
}


void test_networks() {
  SWC::Config::Strings cidrs;
  cidrs.push_back("127.0.0.0/8");
  cidrs.push_back("::1/128");

  SWC::Comm::Networks_v4 nets_v4;
  SWC::Comm::Networks_v6 nets_v6;
  asio::error_code ec;
  SWC::Comm::Resolver::get_networks(cidrs, nets_v4, nets_v6, ec);
  SWC_ASSERT(!ec);
  SWC_ASSERT(nets_v4.size() == 1);
  SWC_ASSERT(nets_v6.size() == 1);

  SWC::Comm::EndPoint local4(asio::ip::make_address_v4("127.0.0.1"), 1000);
  SWC::Comm::EndPoint other4(asio::ip::make_address_v4("10.0.0.1"), 1000);
  SWC::Comm::EndPoint local6(asio::ip::make_address_v6("::1"), 1000);

  SWC_ASSERT(SWC::Comm::Resolver::is_network(local4, nets_v4, nets_v6));
  SWC_ASSERT(!SWC::Comm::Resolver::is_network(other4, nets_v4, nets_v6));
  SWC_ASSERT(SWC::Comm::Resolver::is_network(local6, nets_v4, nets_v6));
}


void test_endpoint_helpers() {
  SWC::Comm::EndPoint a(asio::ip::make_address_v4("127.0.0.1"), 1234);
  SWC::Comm::EndPoint b(asio::ip::make_address_v4("127.0.0.1"), 1234);
  SWC::Comm::EndPoint c(asio::ip::make_address_v4("127.0.0.1"), 4321);

  SWC::Comm::EndPoints eps;
  eps.push_back(a);
  SWC_ASSERT(SWC::Comm::has_endpoint(a, eps));
  SWC_ASSERT(!SWC::Comm::has_endpoint(c, eps));

  SWC::Comm::EndPoints eps2;
  eps2.push_back(b);
  SWC_ASSERT(SWC::Comm::equal_endpoints(eps, eps2));
  eps2.push_back(c);
  SWC_ASSERT(!SWC::Comm::equal_endpoints(eps, eps2));

  SWC_ASSERT(SWC::Comm::endpoint_hash(a) == SWC::Comm::endpoint_hash(b));
}


void test_endpoint_serialization() {
  SWC::Comm::EndPoint v4(asio::ip::make_address_v4("192.0.2.10"), 8080);
  SWC::Comm::EndPoint v6(asio::ip::make_address_v6("2001:db8::2"), 9090);

  {
    size_t len = SWC::Serialization::encoded_length(v4);
    std::vector<uint8_t> buf(len);
    uint8_t* p = buf.data();
    SWC::Serialization::encode(&p, v4);
    SWC_ASSERT(size_t(p - buf.data()) == len);

    const uint8_t* rp = buf.data();
    size_t remain = len;
    auto decoded = SWC::Serialization::decode(&rp, &remain);
    SWC_ASSERT(remain == 0);
    SWC_ASSERT(decoded == v4);
  }

  {
    size_t len = SWC::Serialization::encoded_length(v6);
    std::vector<uint8_t> buf(len);
    uint8_t* p = buf.data();
    SWC::Serialization::encode(&p, v6);
    const uint8_t* rp = buf.data();
    size_t remain = len;
    auto decoded = SWC::Serialization::decode(&rp, &remain);
    SWC_ASSERT(remain == 0);
    SWC_ASSERT(decoded == v6);
  }

  {
    SWC::Comm::EndPoints endpoints;
    endpoints.push_back(v4);
    endpoints.push_back(v6);
    size_t len = SWC::Serialization::encoded_length(endpoints);
    std::vector<uint8_t> buf(len);
    uint8_t* p = buf.data();
    SWC::Serialization::encode(&p, endpoints);

    SWC::Comm::EndPoints decoded;
    const uint8_t* rp = buf.data();
    size_t remain = len;
    SWC::Serialization::decode(&rp, &remain, decoded);
    SWC_ASSERT(remain == 0);
    SWC_ASSERT(SWC::Comm::equal_endpoints(endpoints, decoded));
  }

  {
    bool threw = false;
    try {
      uint8_t tiny[2] = { 0x01, 0x02 };
      const uint8_t* rp = tiny;
      size_t remain = 2;
      (void)SWC::Serialization::decode(&rp, &remain);
    } catch(const SWC::Error::Exception& e) {
      threw = true;
      SWC_ASSERT(e.code() == SWC::Error::SERIALIZATION_INPUT_OVERRUN);
    }
    SWC_ASSERT(threw);
  }
}


void test_get_endpoints_literal() {
  SWC::Config::Strings addrs;
  addrs.push_back("127.0.0.1-1234");
  SWC::Comm::Networks nets;
  auto endpoints = SWC::Comm::Resolver::get_endpoints(
    9999, addrs, std::string(), nets, false);
  SWC_ASSERT(endpoints.size() == 1);
  SWC_ASSERT(endpoints[0].address().to_v4().to_string() == "127.0.0.1");
  SWC_ASSERT(endpoints[0].port() == 1234);

  SWC::Config::Strings addrs_default;
  addrs_default.push_back("127.0.0.1");
  auto endpoints2 = SWC::Comm::Resolver::get_endpoints(
    5555, addrs_default, std::string(), nets, false);
  SWC_ASSERT(endpoints2.size() == 1);
  SWC_ASSERT(endpoints2[0].port() == 5555);
}


} // namespace


int main() {
  test_ip_classify();
  test_networks();
  test_endpoint_helpers();
  test_endpoint_serialization();
  test_get_endpoints_literal();

  std::cout << "resolver OK\n";
  return 0;
}
