#include "Utils.h"

#include <cstring>
#include "Name.h"
#include "Parallel.h"

void Phoenix::ChunkedParallelCopy(uint8_t* dst, const uint8_t* src, size_t bufferSize, size_t chunkSize)
{
    static std::shared_ptr<TaskQueue> gTaskQueue;
    if (!gTaskQueue)
    {
        gTaskQueue = TaskQueue::CreateTaskQueue("ChunkedParallelCopy"_n);
    }

    if (chunkSize == 0)
    {
        return;
    }

    size_t numChunks = (bufferSize + chunkSize - 1) / chunkSize;
    std::atomic<size_t> chunkIndex(0);

    auto worker = [&]
    {
        while (true)
        {
            size_t idx = chunkIndex.fetch_add(1);
            if (idx >= numChunks)
            {
                break;
            }

            size_t offset = idx * chunkSize;
            size_t size = std::min(chunkSize, bufferSize - offset);

            // Compare chunk
            if (std::memcmp(src + offset, dst + offset, size) != 0)
            {
                // Copy chunk if different
                std::memcpy(dst + offset, src + offset, size);
            }
        }
    };

    gTaskQueue->BeginGroup(gTaskQueue->GetNumWorkers());
    for (uint32 i = 0; i < gTaskQueue->GetNumWorkers(); ++i)
    {
        gTaskQueue->Enqueue(worker);
    }
    gTaskQueue->EndGroup();

    gTaskQueue->Flush();
}
