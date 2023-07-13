#pragma once
#include <deque>
#include "IViewport.h"
#include "ImGUI/Libary/imgui.h"
#include "Foundation/Logger.h"

class ConsoleViewport : public IViewport {
public:
    ConsoleViewport();
    ~ConsoleViewport() override;
    void Clear();
    void OnGUI(GameTimer &gameTimer) override;
private:
    void AddLog(Logger::DebugLevel level, const std::string &message);
    struct LogItem {
        std::string message;
        Logger::DebugLevel level;
    };
private:
    ImGuiTextFilter _filter;
    bool _autoScroll = true;
    std::mutex _mutex;
    std::deque<LogItem> _logItems;
    Logger::DebugLevel _logLevel = Logger::EL_All;

    size_t _infoLogCount = 0;
    size_t _debugLogCount = 0;
    size_t _warningLogCount = 0;
    size_t _errorLogCount = 0;

    static constexpr size_t kMaxLogCount = 5000;
};
