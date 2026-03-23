#include "ImGuiPropertyGrid.h"

#include "imgui.h"
#include <cstddef>
#include <ranges>
#include <vector>

using namespace Phoenix;

void DrawPropertyEditor(void* obj, const PropertyDescriptor& propertyDesc)
{
#define NUMERIC_EDITOR(type, imgui_type) \
    { \
        type v_min = std::numeric_limits<type>::min(), v_max = std::numeric_limits<type>::max(); \
        ImGui::SetNextItemWidth(-FLT_MIN); \
        type v = propertyDesc.PropertyAccessor->Get<type>(obj); \
        if (ImGui::DragScalar("##Editor", imgui_type, &v, 1.0f, &v_min, &v_max)) \
        { \
            propertyDesc.PropertyAccessor->Set(obj, v); \
        } \
    }

    switch (propertyDesc.ValueType)
    {
        case EGenericValueType::Int8:      NUMERIC_EDITOR(int8, ImGuiDataType_S8) break;
        case EGenericValueType::UInt8:     NUMERIC_EDITOR(uint8, ImGuiDataType_U8) break;
        case EGenericValueType::Int16:     NUMERIC_EDITOR(int16, ImGuiDataType_S16) break;
        case EGenericValueType::UInt16:    NUMERIC_EDITOR(uint16, ImGuiDataType_U16) break;
        case EGenericValueType::Int32:     NUMERIC_EDITOR(int32, ImGuiDataType_S32) break;
        case EGenericValueType::UInt32:    NUMERIC_EDITOR(uint32, ImGuiDataType_U32) break;
        case EGenericValueType::Int64:     NUMERIC_EDITOR(int64, ImGuiDataType_S64) break;
        case EGenericValueType::UInt64:    NUMERIC_EDITOR(uint64, ImGuiDataType_U64) break;
        case EGenericValueType::Float:     NUMERIC_EDITOR(float, ImGuiDataType_Float) break;
        case EGenericValueType::Double:    NUMERIC_EDITOR(double, ImGuiDataType_Double) break;
        case EGenericValueType::Bool:
            {
                bool v = propertyDesc.PropertyAccessor->Get<bool>(obj);
                if (ImGui::Checkbox("##Editor", &v))
                {
                    propertyDesc.PropertyAccessor->Set(obj, v);
                }
                break;
            }
        case EGenericValueType::String:
            {
                std::string v = propertyDesc.PropertyAccessor->Get<std::string>(obj);
                
                char buff[MAX_PATH];
                strcpy_s(buff, MAX_PATH, v.data());

                if (ImGui::InputText("##Editor", buff, MAX_PATH))
                {
                    v = buff;
                    propertyDesc.PropertyAccessor->Set(obj, v);
                }
                break;
            }
        case EGenericValueType::Name:          break;
        case EGenericValueType::FixedPoint:    break;
        default: break;
    }

#undef NUMERIC_EDITOR
}

void DrawPropertyEditor(const void* obj, const PropertyDescriptor& propertyDesc)
{
    ImGui::BeginDisabled(true);

#define NUMERIC_EDITOR(type, imgui_type) \
    { \
        type v_min = std::numeric_limits<type>::min(), v_max = std::numeric_limits<type>::max(); \
        ImGui::SetNextItemWidth(-FLT_MIN); \
        type v = propertyDesc.PropertyAccessor->Get<type>(obj); \
        ImGui::DragScalar("##Editor", imgui_type, &v, 1.0f, &v_min, &v_max); \
    }

    switch (propertyDesc.ValueType)
    {
        case EGenericValueType::Int8:      NUMERIC_EDITOR(int8, ImGuiDataType_S8) break;
        case EGenericValueType::UInt8:     NUMERIC_EDITOR(uint8, ImGuiDataType_U8) break;
        case EGenericValueType::Int16:     NUMERIC_EDITOR(int16, ImGuiDataType_S16) break;
        case EGenericValueType::UInt16:    NUMERIC_EDITOR(uint16, ImGuiDataType_U16) break;
        case EGenericValueType::Int32:     NUMERIC_EDITOR(int32, ImGuiDataType_S32) break;
        case EGenericValueType::UInt32:    NUMERIC_EDITOR(uint32, ImGuiDataType_U32) break;
        case EGenericValueType::Int64:     NUMERIC_EDITOR(int64, ImGuiDataType_S64) break;
        case EGenericValueType::UInt64:    NUMERIC_EDITOR(uint64, ImGuiDataType_U64) break;
        case EGenericValueType::Float:     NUMERIC_EDITOR(float, ImGuiDataType_Float) break;
        case EGenericValueType::Double:    NUMERIC_EDITOR(double, ImGuiDataType_Double) break;
        case EGenericValueType::Bool:
            {
                bool v = propertyDesc.PropertyAccessor->Get<bool>(obj);
                ImGui::Checkbox("##Editor", &v);
                break;
            }
        case EGenericValueType::String:
            {
                std::string v = propertyDesc.PropertyAccessor->Get<std::string>(obj);
                
                char buff[MAX_PATH];
                strcpy_s(buff, MAX_PATH, v.data());

                ImGui::InputText("##Editor", buff, MAX_PATH);
                break;
            }
        case EGenericValueType::Name:
            {
                FName v = propertyDesc.PropertyAccessor->Get<FName>(obj);
                // TODO (jfarris): this won't work for release builds but whatever
                if (const char* string = FName::GetNameEntry(v))
                {
                    ImGui::InputText("##Editor", const_cast<char*>(string), strlen(string), ImGuiInputTextFlags_ReadOnly);
                }
                break;
            }
        case EGenericValueType::FixedPoint:
            {
                auto iter = propertyDesc.Metadata.find("FractionalBits");
                if (iter != propertyDesc.Metadata.end())
                {
                    TFixed<1> fp = propertyDesc.PropertyAccessor->Get<TFixed<1>>(obj);
                    uint8 b = (uint8)std::stoi(iter->second.c_str());
                    double val = ConvertFromQ<int64, double>(fp.Value, b);
                    ImGui::DragScalar("##Editor", ImGuiDataType_Double, &val);
                }
                break;
            }
        case EGenericValueType::Struct:
            {
                // Nested struct: show sub-properties inline (read-only for now)
                if (propertyDesc.StructDescriptor)
                {
                    std::vector<std::byte> tmp(propertyDesc.StructDescriptor->GetSize());
                    propertyDesc.PropertyAccessor->Get(obj, tmp.data(), tmp.size());
                    for (const auto& subProp : propertyDesc.StructDescriptor->GetProperties() | std::views::values)
                    {
                        ImGui::TextUnformatted(subProp.Name.c_str());
                        ImGui::SameLine();
                        DrawPropertyEditor(tmp.data(), subProp);
                    }
                }
                break;
            }
        default: break;
    }

#undef NUMERIC_EDITOR

    ImGui::EndDisabled();
}

void DrawPropertyGrid(void* obj, const TypeDescriptor& descriptor)
{
    if (ImGui::BeginTable("##properties", 2))
    {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f); // Default twice larger

        for (const auto& methodDesc : descriptor.GetMethods() | std::views::values)
        {
            ImGui::TableNextRow();
            ImGui::PushID(methodDesc.Name.c_str());
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(methodDesc.Name.c_str());
            ImGui::TableNextColumn();

            void* actualObj = methodDesc.IsStatic() ? nullptr : obj;

            ImGui::BeginDisabled(!methodDesc.CanExecute(actualObj));
            if (ImGui::Button(methodDesc.Name.c_str()))
            {
                methodDesc.Execute(actualObj);
            }
            ImGui::EndDisabled();

            ImGui::PopID();
        }

        for (const auto& propertyDesc : descriptor.GetProperties() | std::views::values)
        {
            ImGui::TableNextRow();
            ImGui::PushID(propertyDesc.Name.c_str());
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(propertyDesc.Name.c_str());
            ImGui::TableNextColumn();

            void* actualObj = propertyDesc.PropertyAccessor->IsStatic() ? nullptr : obj;
            DrawPropertyEditor(actualObj, propertyDesc);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void DrawPropertyGrid(const void* obj, const TypeDescriptor& descriptor)
{
    if (ImGui::BeginTable("##properties", 2))
    {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f); // Default twice larger

        for (const auto& propertyDesc : descriptor.GetProperties() | std::views::values)
        {
            ImGui::TableNextRow();
            ImGui::PushID(propertyDesc.Name.c_str());
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(propertyDesc.Name.c_str());
            ImGui::TableNextColumn();

            const void* actualObj = propertyDesc.PropertyAccessor->IsStatic() ? nullptr : obj;
            DrawPropertyEditor(actualObj, propertyDesc);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}
