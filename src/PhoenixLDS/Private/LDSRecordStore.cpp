

#include "LDSRecordStore.h"

using namespace nlohmann;

namespace Phoenix::LDS
{
    bool TryParse(const PHXString& string, ELDSValueType& outEnum)
    {
#define PARSE(str, e) \
        if (str == #e) \
        { \
            outEnum = ELDSValueType::e; \
            return true; \
        }
        
        PARSE(string, Int32)
        PARSE(string, UInt32)
        PARSE(string, Name)
        PARSE(string, Value)
        PARSE(string, Distance)
        PARSE(string, Degrees)
        PARSE(string, Speed)
        PARSE(string, Bool)
        PARSE(string, Array)
        PARSE(string, Object)

#undef PARSE

        return false;
    }
}
