#pragma once

#include <cstdint>
#include <imgui.h>
#include <memory>
#include <mutex>
#include <vector>

namespace Phoenix
{
    class ILogger;
}

class Console
{
public:

    Console();
    ~Console();

    void Init(const std::shared_ptr<Phoenix::ILogger>& logger);

    void ClearLog();

    void AddLog(const char* fmt, ...) IM_FMTARGS(2);

    void Draw();

    void ExecCommand(const char* command_line);

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

    int TextEditCallback(ImGuiInputTextCallbackData* data);

private:

    char                        InputBuf[256];
    ImVector<const char*>       Commands;
    ImVector<char*>             History;
    int                         HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter             Filter;
    bool                        AutoScroll;
    bool                        ScrollToBottom;

    char*                       Items[1024];
    uint32_t                    ItemsReadIdx = 0;
    uint32_t                    ItemsWriteIdx = 0;

    std::shared_ptr<Phoenix::ILogger>   Logger;
    std::mutex                          LogMutex;
    std::vector<std::string>            LogFlush;
};
