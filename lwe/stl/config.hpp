#ifndef LWE_STL_CONFIG
#define LWE_STL_CONFIG

#include "../base/base.h"

LWE_BEGIN
namespace stl {
namespace config {

static constexpr size_t DEF_SVO = 8; //!< default small vector optimization size

} // namespace config

using namespace config;
} // namespace stl
LWE_END
#endif
