#include "ImGuiPropertyGrid.h"

#include <algorithm>

#include "imgui.h"
#include <cstddef>
#include <ranges>
#include <vector>

#include "Phoenix.Sim/FixedPoint/FixedPoint.h"
#include "Phoenix.Sim/Reflection/PropertyDescriptor.h"
#include "Phoenix.Sim/Reflection/TypeDescriptor.h"

using namespace Phoenix;

typedef std::function<void(const void*)> PropertyEditorSetValueCallback;
typedef std::function<void(const Variant&)> PropertyEditorExecuteCallback;

void DrawClassEditor(void* obj, const TypeDescriptor& type, const MemberDescriptor* parentDesc, uint32 depth);

void DrawPropertyEditor(
    void* obj,
    const TypeDescriptor& type,
    const MemberDescriptor* parentDesc,
    const PropertyEditorSetValueCallback& callback)
{
#define NUMERIC_EDITOR(type, imgui_type) \
    { \
        type v_min = std::numeric_limits<type>::min(), v_max = std::numeric_limits<type>::max(); \
        ImGui::SetNextItemWidth(-FLT_MIN); \
        type v = *static_cast<const type*>(obj); \
        if (ImGui::DragScalar("##Editor", imgui_type, &v, 1.0f, &v_min, &v_max)) \
        { \
            callback(&v); \
        } \
    }

    switch (type.GetTypeId())
    {
        case StaticTypeName<int8>::TypeId:      NUMERIC_EDITOR(int8, ImGuiDataType_S8) break;
        case StaticTypeName<uint8>::TypeId:     NUMERIC_EDITOR(uint8, ImGuiDataType_U8) break;
        case StaticTypeName<int16>::TypeId:     NUMERIC_EDITOR(int16, ImGuiDataType_S16) break;
        case StaticTypeName<uint16>::TypeId:    NUMERIC_EDITOR(uint16, ImGuiDataType_U16) break;
        case StaticTypeName<int32>::TypeId:     NUMERIC_EDITOR(int32, ImGuiDataType_S32) break;
        case StaticTypeName<uint32>::TypeId:    NUMERIC_EDITOR(uint32, ImGuiDataType_U32) break;
        case StaticTypeName<int64>::TypeId:     NUMERIC_EDITOR(int64, ImGuiDataType_S64) break;
        case StaticTypeName<uint64>::TypeId:    NUMERIC_EDITOR(uint64, ImGuiDataType_U64) break;
        case StaticTypeName<float>::TypeId:     NUMERIC_EDITOR(float, ImGuiDataType_Float) break;
        case StaticTypeName<double>::TypeId:    NUMERIC_EDITOR(double, ImGuiDataType_Double) break;
        case StaticTypeName<bool>::TypeId:
            {
                bool v = *static_cast<const bool*>(obj);
                if (ImGui::Checkbox("##Editor", &v))
                {
                    callback(&v);
                }
                break;
            }
        case StaticTypeName<std::string>::TypeId:
            {
                std::string v = *static_cast<const std::string*>(obj);

                char buff[MAX_PATH];
                strcpy_s(buff, MAX_PATH, v.data());

                if (ImGui::InputText("##Editor", buff, MAX_PATH))
                {
                    v = buff;
                    callback(&v);
                }
                break;
            }
        case StaticTypeName<FName>::TypeId:
            {
                FName v = *static_cast<const FName*>(obj);

                char buff[MAX_PATH];
                size_t len = 0;

                // TODO (jfarris): this won't work for release builds but whatever
                if (const char* string = FName::GetNameEntry(v))
                {
                    len = strcpy_s(buff, MAX_PATH, string);
                }
                else
                {
                    len = sprintf_s(buff, 32, "0x%08X", (hash32_t)v);
                }

                if (ImGui::InputText("##Editor", buff, len))
                {
                    v = FName(buff);
                    callback(&v);
                }

                break;
            }
        default: break;
    }

#undef NUMERIC_EDITOR

    if (type.IsTemplate("Phoenix::TFixed") && type.GetTypeName().GetNumTemplateArgs() == 2)
    {
        uint8 b = (uint8)std::stoi(type.GetTypeName().GetTemplateArgs()[0]);
        FName underlyingTypeId = FName(type.GetTypeName().GetTemplateArgs()[1]);

        double val = 0;
        float speed = 1.0f;
        double minVal = DBL_MIN;
        double maxVal = DBL_MAX;

        if (underlyingTypeId == StaticTypeName<int32>::TypeId)
        {
            int32 temp = *static_cast<const int32*>(obj);
            val = ConvertFromQ<int32, double>(temp, b);
            minVal = ConvertFromQ<int32, double>(std::numeric_limits<int32>::min(), b);
            maxVal = ConvertFromQ<int32, double>(std::numeric_limits<int32>::max(), b);
        }
        else if (underlyingTypeId == StaticTypeName<uint64>::TypeId)
        {
            int64 temp = *static_cast<const int64*>(obj);
            val = ConvertFromQ<int64, double>(temp, b);
            minVal = ConvertFromQ<int64, double>(std::numeric_limits<int64>::min(), b);
            maxVal = ConvertFromQ<int64, double>(std::numeric_limits<int64>::max(), b);
        }

        if (ImGui::DragScalar("##Editor", ImGuiDataType_Double, &val, speed, &minVal, &maxVal))
        {
            if (underlyingTypeId == StaticTypeName<int32>::TypeId)
            {
                int32 temp = ConvertToQ<double, int32>(val, b);
                callback(&temp);
            }
            else if (underlyingTypeId == StaticTypeName<uint64>::TypeId)
            {
                int64 temp = ConvertToQ<double, int64>(val, b);
                callback(&temp);
            }
        }
    }

    if (type.IsEnumFlags())
    {
        std::string previewValue = type.ToString(obj);

        if (ImGui::BeginCombo("##Editor", previewValue.c_str()))
        {
            const auto& enumValues = type.GetEnumValues();

            std::unordered_set<uint32> flagIndices;
            std::unordered_set<uint32> newFlagIndices;

            for (const auto& enumFlagValue : enumValues)
            {
                if (type.HasEnumFlag(obj, enumFlagValue.GetValue().GetData()))
                {
                    flagIndices.insert(enumFlagValue.GetIndex());
                }
            }

            for (const auto& enumFlagValue : enumValues)
            {
                bool checked = flagIndices.contains(enumFlagValue.GetIndex());
                if (ImGui::Checkbox(enumFlagValue.GetDisplayName().c_str(), &checked) && checked)
                {
                    newFlagIndices.insert(enumFlagValue.GetIndex());
                }
            }

            ImGui::EndCombo();

            if (newFlagIndices != flagIndices)
            {
                Variant newValue(*type.GetEnumUnderlyingType());
                for (uint32 flagIdx : newFlagIndices)
                {
                    type.SetEnumFlag(newValue.GetData(), enumValues[flagIdx].GetValue().GetData());
                }
                callback(newValue.GetData());
            }
        }
    }
    else if (type.IsEnum())
    {
        std::string previewValue = type.ToString(obj);

        if (ImGui::BeginCombo("##Editor", previewValue.c_str()))
        {
            TOptional<uint32> selectedIndex;
            TOptional<uint32> newSelectedIndex;

            if (auto enumValueDescriptor = type.GetEnumValueDescriptor(obj))
            {
                selectedIndex = enumValueDescriptor->GetIndex();
            }

            const auto& enumValues = type.GetEnumValues();
            for (uint32 i = 0; i < enumValues.size(); ++i)
            {
                const bool selected = selectedIndex.IsSet() && selectedIndex.Get() == i;

                if (ImGui::Selectable(enumValues[i].GetDisplayName().c_str(), selected))
                {
                    newSelectedIndex = i;
                }

                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();

            if (newSelectedIndex != selectedIndex &&
                newSelectedIndex.IsSet() &&
                newSelectedIndex.Get() < enumValues.size())
            {
                const EnumValueDescriptor& descriptor = enumValues[newSelectedIndex.Get()];
                callback(descriptor.GetValue().GetData());
            }
        }
    }
}

void DrawFieldDescriptor(void* obj, const FieldDescriptor& field, uint32 depth)
{
    if (!field.GetType())
    {
        return;
    }

    void* fieldObj;
    if (field.IsStatic())
    {
        fieldObj = nullptr;
    }
    else if (!field.GetPointer(obj, &fieldObj))
    {
        return;
    }

    auto setValue = [&](const void* newValue)
    {
        field.Set(obj, newValue);
    };

    ImGui::BeginDisabled(field.IsReadOnly());

    if (field.GetType()->IsComplex())
    {
        DrawClassEditor(fieldObj, *field.GetType(), &field, depth + 1);
    }
    else
    {
        DrawPropertyEditor(fieldObj, *field.GetType(), &field, setValue);
    }

    ImGui::EndDisabled();
}

void DrawPropertyDescriptor(void* obj, const PropertyDescriptor& property, uint32 depth)
{
    if (!property.GetType())
    {
        return;
    }

    std::vector<std::byte> tempObject;
    void* propertyObj;

    if (property.IsStatic())
    {
        propertyObj = nullptr;
    }
    else
    {
        tempObject.reserve(property.GetType()->GetSize());
        property.Get(obj, tempObject.data());
        propertyObj = tempObject.data();
    }

    auto setValue = [&](const void* newValue)
    {
        property.Set(obj, newValue);
    };

    ImGui::BeginDisabled(property.IsReadOnly());

    if (property.GetType()->IsComplex())
    {
        DrawClassEditor(propertyObj, *property.GetType(), &property, depth + 1);
    }
    else
    {
        DrawPropertyEditor(propertyObj, *property.GetType(), &property, setValue);
    }

    ImGui::EndDisabled();
}

void DrawMethodDescriptor(void* obj, const MethodDescriptor& method, uint32 depth)
{
    ImGui::BeginDisabled(!method.CanExecute(obj));

    if (ImGui::BeginTable("##method", 2))
    {
        ImGui::BeginDisabled(false);

        std::vector<Variant> variants;

        for (const auto& param : method.GetParams())
        {
            ImGui::LabelText(param.Name.c_str(), "%s", param.Type->GetDisplayName().c_str());
        }

        if (ImGui::Button("Execute"))
        {
            method.Execute(obj);
        }

        ImGui::EndDisabled();
        ImGui::EndTable();
    }

    ImGui::EndDisabled();
}

void DrawClassEditor(void* obj, const TypeDescriptor& type, const MemberDescriptor* parentDesc, uint32 depth)
{
    if (depth > 10)
    {
        return;
    }

    std::vector<const MemberDescriptor*> members;
    if (type.GetAllMembers(members) == 0)
    {
        return;
    }

    std::ranges::sort(members, [](const MemberDescriptor* a, const MemberDescriptor* b)
    {
        // Sort methods to the bottom
        if (HasAnyFlags(a->GetFlags(), EMemberDescriptorFlags::Method) &&
            !HasAnyFlags(b->GetFlags(), EMemberDescriptorFlags::Method))
        {
            return false;
        }
        // Sort by category, then sort order, then display name
        if (a->GetCategory() < b->GetCategory())
        {
            return true;
        }
        if (a->GetSortOrder() < b->GetSortOrder())
        {
            return true;
        }
        return a->GetDisplayName() < b->GetDisplayName();
    });

    if (ImGui::BeginTable("##members", 2))
    {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f); // Default twice larger

        for (const auto& member : members)
        {
            // Skip static members and methods
            if (member->IsStatic() || HasAnyFlags(member->GetFlags(), EMemberDescriptorFlags::Method))
            {
                continue;
            }

            ImGui::TableNextRow();
            ImGui::PushID(member->GetName().c_str());
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(member->GetDisplayName().c_str());
            ImGui::TableNextColumn();

            void* actualObj = member->IsStatic() ? nullptr : obj;

            if (HasAnyFlags(member->GetFlags(), EMemberDescriptorFlags::Field))
            {
                DrawFieldDescriptor(actualObj, *static_cast<const FieldDescriptor*>(member), depth + 1);
            }
            else if (HasAnyFlags(member->GetFlags(), EMemberDescriptorFlags::Property))
            {
                DrawPropertyDescriptor(actualObj, *static_cast<const PropertyDescriptor*>(member), depth + 1);
            }
            // else if (HasAnyFlags(member->GetFlags(), EMemberDescriptorFlags::Method))
            // {
            //     DrawMethodDescriptor(actualObj, *static_cast<const MethodDescriptor*>(member), depth + 1);
            // }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void DrawPropertyGrid(void* obj, const TypeDescriptor& descriptor)
{
    DrawClassEditor(obj, descriptor, nullptr, 0);
}

void DrawPropertyGrid(const void* obj, const TypeDescriptor& descriptor)
{
    ImGui::BeginDisabled(true);
    DrawClassEditor(const_cast<void*>(obj), descriptor, nullptr, 0);
    ImGui::EndDisabled();
}
