/*
* TLS Record Handling
* (C) 2004-2010 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_RECORDS_H__
#define BOTAN_TLS_RECORDS_H__

#include <botan/tls_session_key.h>
#include <botan/tls_suites.h>
#include <botan/socket.h>
#include <botan/pipe.h>
#include <botan/secqueue.h>
#include <vector>

namespace Botan {

/**
* TLS Record Writer
*/
class BOTAN_DLL Record_Writer
   {
   public:
      void send(byte, const byte[], size_t);
      void send(byte, byte);
      void flush();

      void alert(Alert_Level, Alert_Type);

      void set_keys(const CipherSuite&, const SessionKeys&, Connection_Side);

      void set_version(Version_Code);

      void reset();

      Record_Writer(Socket& socket);

   private:
      void send_record(byte, const byte[], size_t);
      void send_record(byte, byte, byte, const byte[], size_t);

      Socket& socket;
      Pipe cipher, mac;
      SecureVector<byte> buffer;
      size_t buf_pos;

      size_t block_size, mac_size, iv_size;

      u64bit seq_no;
      byte major, minor, buf_type;
   };

/**
* TLS Record Reader
*/
class BOTAN_DLL Record_Reader
   {
   public:
      void add_input(const byte input[], size_t input_size);

      /**
      * @param msg_type (output variable)
      * @param buffer (output variable)
      * @return Number of bytes still needed (minimum), or 0 if success
      */
      size_t get_record(byte& msg_type,
                        MemoryRegion<byte>& buffer);

      SecureVector<byte> get_record(byte& msg_type);

      void set_keys(const CipherSuite& suite,
                    const SessionKeys& keys,
                    Connection_Side side);

      void set_version(Version_Code version);

      void reset();

      Record_Reader() { reset(); }
   private:
      SecureQueue input_queue;

      Pipe cipher, mac;
      size_t block_size, mac_size, iv_size;
      u64bit seq_no;
      byte major, minor;
   };

}

#endif
