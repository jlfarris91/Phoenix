#include "ParallelExecutor.h"

using namespace Phoenix;

static IParallelExecutor* s_parallelExecutor = nullptr;

bool Phoenix::HasParallelExecutor()
{
    return s_parallelExecutor != nullptr;
}

IParallelExecutor& Phoenix::GetParallelExecutor()
{
    assert(s_parallelExecutor && "No IParallelExecutor installed — call SetParallelExecutor first");
    return *s_parallelExecutor;
}

void Phoenix::SetParallelExecutor(IParallelExecutor* executor)
{
    s_parallelExecutor = executor;
}
