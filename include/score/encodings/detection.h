#pragma once
#include <string>
#include "score/Maybe.h"
#include "score/encodings/Encoding.h"

namespace score { namespace encodings {

const char *detectEncodingStr(const std::string &text);
Maybe<Encoding> detectEncoding(const std::string &text);

}} // score::encodings