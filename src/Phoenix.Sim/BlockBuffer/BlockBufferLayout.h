#pragma once

#include "Phoenix/Reflection/TypeName.h"

namespace Phoenix
{
    class PHOENIX_SIM_API BlockBufferLayout
    {
    public:

        BlockBufferLayout() = default;

        template <class T>
        static BlockBufferLayout For(uint32 allocSize = 0)
        {
            BlockBufferLayout layout;
            layout.Header<T>();
            layout.AllocSize = allocSize;
            return layout;
        }

        template <class T>
        BlockBufferLayout& Header()
        {
            TypeId = FName(StaticTypeName<T>::GetName());
            HeaderSize = sizeof(T);
            return *this;
        }

        BlockBufferLayout& Allocate(uint32 alloc)
        {
            AllocSize += alloc;
            return *this;
        }

        template <class T, class ...TConfigArgs>
        BlockBufferLayout& Container(TConfigArgs&& ...config)
        {
            Children.push_back(T::StaticLayout(typename T::Config{std::forward<TConfigArgs>(config)...}));
            auto& childLayout = Children.back();
            childLayout.AllocOffset = AllocSize;
            AllocSize += childLayout.AllocSize;
            return *this;
        }

        template <class T, class ...TConfigArgs>
        BlockBufferLayout& Container(const char* name, TConfigArgs&& ...config)
        {
            Children.push_back(T::StaticLayout(typename T::Config{std::forward<TConfigArgs>(config)...}));
            auto& childLayout = Children.back();
            childLayout.AllocOffset = AllocSize;
            AllocSize += childLayout.AllocSize;
            childLayout.Name = FName(name, strlen(name));
            return *this;
        }

        BlockBufferLayout& SetName(const std::string& name)
        {
            Name = name;
            return *this;
        }

        FName GetName() const
        {
            return Name;
        }

        uint32 GetHeaderSize() const
        {
            return HeaderSize;
        }

        uint32 GetAllocOffset() const
        {
            return AllocOffset;
        }

        uint32 GetAllocSize() const
        {
            return AllocSize;
        }

        uint32 GetTotalSize() const
        {
            return HeaderSize + AllocSize; 
        }

        const std::vector<BlockBufferLayout>& GetChildren() const
        {
            return Children;
        }

    private:

        FName TypeId;
        FName Name;
        uint32 HeaderSize = 0;
        uint32 AllocSize = 0;
        uint32 AllocOffset = 0;
        std::vector<BlockBufferLayout> Children;
    };
}
