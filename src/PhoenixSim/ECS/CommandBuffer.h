#pragma once

#include <mutex>

#include "PhoenixSim/Name.h"

namespace Phoenix::ECS
{
    class CommandBuffer
    {
    public:

        struct CommandEntry
        {
            FName Id;
            uint32 DataOffset;
            uint32 DataSize;
        };

        bool IsEmpty() const { return Events.empty(); }

        template <class T>
        void Append(FName id, const T& data)
        {
            std::scoped_lock lock(WriteMutex);
            const uint32 offset = static_cast<uint32>(EventData.size());
            EventData.insert(EventData.end(), sizeof(T), 0);
            *reinterpret_cast<T*>(EventData.data() + offset) = data;
            Events.emplace_back(id, offset, sizeof(T));
        }

        template <class T, class ...TArgs>
        void Append(FName id, const TArgs& ...args)
        {
            std::scoped_lock lock(WriteMutex);
            const uint32 offset = static_cast<uint32>(EventData.size());
            EventData.insert(EventData.end(), sizeof(T), 0);
            new (EventData.data() + offset) T(std::forward<TArgs>(args)...);
            Events.emplace_back(id, offset, sizeof(T));
        }

        template <class TCommand, class ...TArgs>
        void Append(TArgs&& ...args)
        {
            std::scoped_lock lock(WriteMutex);
            const uint32 offset = static_cast<uint32>(EventData.size());
            EventData.insert(EventData.end(), sizeof(TCommand), 0);
            new (EventData.data() + offset) TCommand(std::forward<TArgs>(args)...);
            Events.emplace_back(TCommand::StaticId, offset, sizeof(TCommand));
        }

        template <class TCommand>
        void Append(const TCommand& command)
        {
            this->Append(TCommand::StaticId, command);
        }

        void Reset()
        {
            std::scoped_lock lock(WriteMutex);
            Events.clear();
            EventData.clear();
        }

        struct Command
        {
            FName Id;
            const void* Data = nullptr;
            uint32 Size = 0;
        };

        struct ConstIter
        {
            ConstIter(const CommandBuffer* queue, size_t index)
                : Queue(queue), Index(index)
            {
            }

            Command operator*() const
            {
                const CommandEntry& evt = Queue->Events[Index];
                return { evt.Id, Queue->EventData.data() + evt.DataOffset, evt.DataSize };
            }

            ConstIter& operator++()
            {
                ++Index;
                return *this;
            }

            ConstIter operator++(int) const
            {
                return { Queue, Index + 1 };
            }

            bool operator==(const ConstIter&) const = default;
            bool operator!=(const ConstIter&) const = default;

            const CommandBuffer* Queue;
            size_t Index;
        };

        ConstIter begin() const { return { this, 0 }; }
        ConstIter end() const { return { this, Events.size() }; }

    private:

        std::mutex WriteMutex;
        std::vector<CommandEntry> Events;
        std::vector<uint8> EventData;
    };
}
