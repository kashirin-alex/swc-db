require 'swcdb/thrift/gen/service'
# require 'thrift/protocol/binary_protocol_accelerated'



module Swcdb
  module Thrift
    # The SWC-DB Thrift Client
    class Client < Gen::Service::Client

      # SWC-DB Thrift Client initializer
      #
      # Example:
      #   ~/ # irb
      #   irb> require 'swcdb/thrift/client'
      #   irb> client = Swcdb::Thrift::Client.new("localhost", 18000)
      #   irb> schemas = client.sql_list_columns("get columns 1,2,3,4")
      #
      # Arguments:
      #   @param [String] host
      #   @param [Integer] port
      #   @param [Integer] timeout_ms
      #   @param [Boolean] do_open
      #   @param [Boolean] framed
      #   @param [Boolean] accelerated
      def initialize(host, port, timeout_ms = 900000, do_open = true, framed = true, accelerated = true)
        socket = ::Thrift::Socket.new(host, port, timeout_ms)

        if framed
          @transport = ::Thrift::FramedTransport.new(socket)
        else
          @transport = ::Thrift::BufferedTransport.new(socket)
        end

        if accelerated
          protocol = ::Thrift::BinaryProtocolAccelerated.new(@transport)
        else
          protocol = ::Thrift::BinaryProtocol.new(@transport)
        end

        super(protocol)

        open() if do_open
      end

      # Open Client Connection
      def open()
        @transport.open()
      end

      # Close Client Connection
      def close()
        @transport.close()
      end

    end
  end
end
