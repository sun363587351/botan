/*
* (C) 2014,2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "tests.h"

#if defined(BOTAN_HAS_STREAM_CIPHER)
   #include <botan/stream_cipher.h>
#endif

namespace Botan_Tests {

#if defined(BOTAN_HAS_STREAM_CIPHER)

class Stream_Cipher_Tests final : public Text_Based_Test
   {
   public:
      Stream_Cipher_Tests(): Text_Based_Test("stream", "Key,Out", "In,Nonce,Seek") {}

      Test::Result run_one_test(const std::string& algo, const VarMap& vars) override
         {
         const std::vector<uint8_t> key      = get_req_bin(vars, "Key");
         const std::vector<uint8_t> expected = get_req_bin(vars, "Out");
         const std::vector<uint8_t> nonce    = get_opt_bin(vars, "Nonce");
         const size_t seek                   = get_opt_sz(vars, "Seek", 0);
         std::vector<uint8_t> input          = get_opt_bin(vars, "In");

         if(input.empty())
            {
            input.resize(expected.size());
            }

         Test::Result result(algo);

         const std::vector<std::string> providers =
            provider_filter(Botan::StreamCipher::providers(algo));

         if(providers.empty())
            {
            result.note_missing("block cipher " + algo);
            return result;
            }

         for(auto const& provider_ask : providers)
            {
            std::unique_ptr<Botan::StreamCipher> cipher(Botan::StreamCipher::create(algo, provider_ask));

            if(!cipher)
               {
               result.test_failure("Stream " + algo + " supported by " + provider_ask + " but not found");
               continue;
               }

            const std::string provider(cipher->provider());
            result.test_is_nonempty("provider", provider);
            result.test_eq(provider, cipher->name(), algo);

            try
               {
               std::vector<uint8_t> buf(128);
               cipher->cipher1(buf.data(), buf.size());
               result.test_failure("Was able to encrypt without a key being set");
               }
            catch(Botan::Invalid_State&)
               {
               result.test_success("Trying to encrypt with no key set fails");
               }

            try
               {
               cipher->seek(0);
               result.test_failure("Was able to seek without a key being set");
               }
            catch(Botan::Invalid_State&)
               {
               result.test_success("Trying to seek with no key set fails");
               }
            catch(Botan::Not_Implemented&)
               {
               result.test_success("Trying to seek failed because not implemented");
               }

            cipher->set_key(key);

            /*
            Test invalid nonce sizes. this assumes no implemented cipher supports a nonce of 65000
            */
            const size_t large_nonce_size = 65000;
            result.confirm("Stream cipher does not support very large nonce", cipher->valid_iv_length(large_nonce_size) == false);

            result.test_throws("Throws if invalid nonce size given",
                               [&]() { cipher->set_iv(nullptr, large_nonce_size); });

            if(nonce.size())
               {
               if(!cipher->valid_iv_length(nonce.size()))
                  {
                  throw Test_Error("Invalid nonce for " + algo);
                  }
               cipher->set_iv(nonce.data(), nonce.size());
               }
            else
               {
               /*
               * If no nonce was set then implicitly the cipher is using a
               * null/empty nonce. Call set_iv with such a nonce to make sure
               * set_iv accepts it.
               */
               if(!cipher->valid_iv_length(0))
                  {
                  throw Test_Error("Stream cipher " + algo + " requires nonce but none provided");
                  }
               cipher->set_iv(nullptr, 0);
               }

            if(seek != 0)
               {
               cipher->seek(seek);
               }

            // Test that clone works and does not affect parent object
            std::unique_ptr<Botan::StreamCipher> clone(cipher->clone());
            result.confirm("Clone has different pointer", cipher.get() != clone.get());
            result.test_eq("Clone has same name", cipher->name(), clone->name());
            clone->set_key(Test::rng().random_vec(cipher->maximum_keylength()));

            {
            std::vector<uint8_t> buf = input;
            cipher->encrypt(buf);
            result.test_eq(provider, "encrypt", buf, expected);
            }

            cipher->clear();

            try
               {
               std::vector<uint8_t> buf(128);
               cipher->cipher1(buf.data(), buf.size());
               result.test_failure("Was able to encrypt without a key being set (after clear)");
               }
            catch(Botan::Invalid_State&)
               {
               result.test_success("Trying to encrypt with no key set (after clear) fails");
               }
            }

         return result;
         }
   };

BOTAN_REGISTER_TEST("stream", Stream_Cipher_Tests);

#endif

}
