/*
 * Copyright 2016 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "score/vendored/folly/portability/Memory.h"
#include "score/vendored/folly/portability/Config.h"

#define MEMORY_NS_BEGIN \
namespace score { namespace vendored { namespace folly { namespace detail {

#define MEMORY_NS_END \
}}}} // score::vendored::folly::detail


#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 ||                    \
    (defined(__ANDROID__) && (__ANDROID_API__ > 15)) ||                      \
    (defined(__APPLE__) && (__MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_6 || \
                            __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_3_0))
#include <errno.h>
MEMORY_NS_BEGIN

// Use posix_memalign, but mimic the behaviour of memalign
void* aligned_malloc(size_t size, size_t align) {
  void* ptr = nullptr;
  int rc = posix_memalign(&ptr, align, size);
  if (rc == 0) {
    return ptr;
  }
  errno = rc;
  return nullptr;
}

void aligned_free(void* aligned_ptr) {
  free(aligned_ptr);
}
MEMORY_NS_END

#elif defined(_WIN32)

#include <malloc.h> // nolint

MEMORY_NS_BEGIN

void* aligned_malloc(size_t size, size_t align) {
  return _aligned_malloc(size, align);
}

void aligned_free(void* aligned_ptr) {
  _aligned_free(aligned_ptr);
}
MEMORY_NS_END

#else
#include <malloc.h> // nolint


MEMORY_NS_BEGIN
void* aligned_malloc(size_t size, size_t align) {
  return memalign(align, size);
}

void aligned_free(void* aligned_ptr) {
  free(aligned_ptr);
}

MEMORY_NS_END
#endif

