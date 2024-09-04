#pragma once
// clang-format off
#include "easylogging++.h"
// clang-format on

#define PRINTABLE_DEFINITIONS(CLASS_NAME)                            \
    operator std::string() const;                                    \
                                                                     \
    inline std::ostream &operator<<(std::ostream &os) const {        \
        os << (std::string) * this;                                  \
        return os;                                                   \
    };                                                               \
                                                                     \
    friend inline el::base::type::ostream_t &operator<<(             \
        el::base::type::ostream_t &os, const CLASS_NAME &loggable) { \
        os << (std::string)loggable;                                 \
        return os;                                                   \
    };
