#pragma once
#define ELPP_STL_LOGGING
#include "easylogging++.h"

#define PRINTABLE_DEFINITIONS(CLASS_NAME) \
    operator std::string() const; \
\
    inline std::ostream& operator <<(std::ostream& os) const {\
        os << (std::string)*this;\
        return os;\
    };\
\
    friend inline el::base::type::ostream_t& operator<<(\
        el::base::type::ostream_t& os,\
        const CLASS_NAME& loggable\
    ) {\
        os << (std::string)loggable;\
        return os;\
    };
