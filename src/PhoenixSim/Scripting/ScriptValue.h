// ── REMOVED ───────────────────────────────────────────────────────────────────
//
// ScriptValue / EScriptValueKind have been replaced by:
//
//   PhoenixSim/Reflection/GenericValue.h
//       GenericValue, ParamTypeRef, GenericConverter<T>, MakeParamTypeRef<T>()
//
// EPropertyValueType (the precise primitive type enum) supersedes the coarse
// EScriptValueKind.  LuaRuntime converts GenericValue ↔ Lua directly.
#error "ScriptValue.h has been removed. Include PhoenixSim/Reflection/GenericValue.h instead."
