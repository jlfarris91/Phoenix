#include "PhoenixSim/LDS/LDSValue.h"

namespace Phoenix::LDS
{
    bool TryParse(const std::string& string, ELDSValueType& outEnum)
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
        PARSE(string, "Asset", Asset)
        PARSE(string, "Enum", Enum)
        PARSE(string, "EnumFlags", EnumFlags)
        PARSE(string, "Expression", Expression)

#undef PARSE

        return false;
    }

    std::string ToString(ELDSValueType valueType)
    {
#define TO_STRING(estr, e) case ELDSValueType::e: return estr;
        switch (valueType)
        {
            TO_STRING("Int32", Int32)
            TO_STRING("UInt32", UInt32)
            TO_STRING("Name", Name)
            TO_STRING("Value", Value)
            TO_STRING("Distance", Distance)
            TO_STRING("Degrees", Degrees)
            TO_STRING("Speed", Speed)
            TO_STRING("Time", Time)
            TO_STRING("Bool", Bool)
            TO_STRING("Text", Text)
            TO_STRING("Array", Array)
            TO_STRING("Object", Object)
            TO_STRING("ObjectRef", ObjectRef)
            TO_STRING("Asset", Asset)
            TO_STRING("Enum", Enum)
            TO_STRING("EnumFlags", EnumFlags)
            TO_STRING("Expression", Expression)
            TO_STRING("Unknown", Unknown)
            default: return "Unknown";
        }
#undef TO_STRING
    }
}
