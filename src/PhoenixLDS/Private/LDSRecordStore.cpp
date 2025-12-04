

#include "LDSRecordStore.h"

using namespace nlohmann;

namespace Phoenix::LDS
{
    bool TryParse(const PHXString& string, ELDSValueType& outEnum)
    {
#define PARSE(str, estr, e) \
        if (str == estr) \
        { \
            outEnum = ELDSValueType::e; \
            return true; \
        }
        
        PARSE(string, "Int32", Int32)
        PARSE(string, "Integer", Int32)
        PARSE(string, "UInt32", UInt32)
        PARSE(string, "Name", Name)
        PARSE(string, "String", Name)
        PARSE(string, "Value", Value)
        PARSE(string, "Distance", Distance)
        PARSE(string, "Degrees", Degrees)
        PARSE(string, "Speed", Speed)
        PARSE(string, "Time", Time)
        PARSE(string, "Bool", Bool)
        PARSE(string, "Boolean", Bool)
        PARSE(string, "Text", Text)
        PARSE(string, "Array", Array)
        PARSE(string, "Object", Object)
        PARSE(string, "ObjectRef", ObjectRef)

#undef PARSE

        return false;
    }
}
