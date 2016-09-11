#include "score/io/Base64.h"
#include "score/vendored/modp_b64/modp_b64.h"

using namespace std;
namespace score { namespace io {

string Base64::decode(const string &encoded) {
  string result = encoded;
  modp_b64_decode(result);
  return result;
}

string Base64::encode(const string &plainText) {
  string result = plainText;
  modp_b64_encode(result);
  return result;
}

}} // score::io
