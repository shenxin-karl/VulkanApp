#include "ConsoleViewport.h"
#include <glm/vec4.hpp>
#include "Foundation/ColorUtil.hpp"

static ImVec4 ToImVec4(const glm::vec4 &color) {
    return ImVec4{color.r, color.g, color.b, color.a};
}

ConsoleViewport::ConsoleViewport() : IViewport("Console") {
    gLogger->SetLogCallBack([&](Logger::DebugLevel level, const std::string &message) {
	    AddLog(level, message);
    });
}

ConsoleViewport::~ConsoleViewport() {
    gLogger->SetLogCallBack(nullptr);
}

void ConsoleViewport::Clear() {
    std::lock_guard lock(_mutex);
    _logItems.clear();
    _infoLogCount = 0;
    _debugLogCount = 0;
    _warningLogCount = 0;
    _errorLogCount = 0;
}

void ConsoleViewport::OnGUI(GameTimer &gameTimer) {
    if (!isShow) {
        return;
    }

    ImGui::Begin("Console", &isShow);
    if (ImGui::BeginPopup("Options")) {
        ImGui::Checkbox("Auto-Scroll", &_autoScroll);
        bool showInfo = HasFlag(_logLevel, Logger::DebugLevel::EL_Info);
        bool showDebug = HasFlag(_logLevel, Logger::DebugLevel::EL_Debug);
        bool showWarning = HasFlag(_logLevel, Logger::DebugLevel::EL_Warning);
        bool showError = HasFlag(_logLevel, Logger::DebugLevel::EL_Error);
        if (ImGui::Checkbox("Show Info Message", &showInfo)) {
            _logLevel = SetOrClearFlags(_logLevel, Logger::DebugLevel::EL_Info, showInfo);
        }
        if (ImGui::Checkbox("Show Debug Message", &showDebug)) {
            _logLevel = SetOrClearFlags(_logLevel, Logger::DebugLevel::EL_Debug, showDebug);
        }
        if (ImGui::Checkbox("Show Warning Message", &showWarning)) {
            _logLevel = SetOrClearFlags(_logLevel, Logger::DebugLevel::EL_Warning, showWarning);
        }
        if (ImGui::Checkbox("Show Error Message", &showError)) {
            _logLevel = SetOrClearFlags(_logLevel, Logger::DebugLevel::EL_Error, showError);
        }
        ImGui::EndPopup();
    }

    ImGui::Text("Log Count Info: %d, Debug: %d, Warning: %d, Error: %d",
        static_cast<int>(_infoLogCount),
        static_cast<int>(_debugLogCount),
        static_cast<int>(_warningLogCount),
        static_cast<int>(_errorLogCount));

    if (ImGui::Button("Options")) {
        ImGui::OpenPopup("Options");
    }
    ImGui::SameLine();
    bool clear = ImGui::Button("Clear");
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    _filter.Draw("Filter", -100.f);

    ImGui::Separator();
    ImVec4 white = ToImVec4(Colors::White);
    ImVec4 green = ToImVec4(Colors::Green);
    ImVec4 yellow = ToImVec4(Colors::Yellow);
    ImVec4 red = ToImVec4(Colors::Red);

    auto OutputMessage = [&](Logger::DebugLevel level, const std::string &message) {
        switch (level) {
        case Logger::EL_Info:
            ImGui::TextColored(white, "%s", message.c_str());
            break;
        case Logger::EL_Debug:
            ImGui::TextColored(green, "%s", message.c_str());
            break;
        case Logger::EL_Warning:
            ImGui::TextColored(yellow, "%s", message.c_str());
            break;
        case Logger::EL_Error:
            ImGui::TextColored(red, "%s", message.c_str());
            break;
        default:
            break;
        }
    };

    if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        if (clear) {
            Clear();
        }
        if (copy) {
            ImGui::LogToClipboard();
        }
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        std::lock_guard lock(_mutex);

        if (_filter.IsActive()) {
            for (LogItem &item : _logItems) {
                if (!HasFlag(_logLevel, item.level)) {
                    continue;
                }
                if (!_filter.PassFilter(item.message.data(), item.message.data() + item.message.length())) {
                    continue;
                }
                OutputMessage(item.level, item.message);
            }
        } else {
            size_t count = 0;
            if (HasFlag(_logLevel, Logger::EL_Info)) {
                count += _infoLogCount;
            }
            if (HasFlag(_logLevel, Logger::EL_Debug)) {
                count += _debugLogCount;
            }
            if (HasFlag(_logLevel, Logger::EL_Warning)) {
                count += _warningLogCount;
            }
            if (HasFlag(_logLevel, Logger::EL_Error)) {
                count += _errorLogCount;
            }

            ImGuiListClipper clipper;
            clipper.Begin(count, ImGui::GetTextLineHeightWithSpacing());

            int index = 0;
            while (clipper.Step() && index < _logItems.size()) {
                int lineCount = clipper.DisplayEnd - clipper.DisplayStart;
                while (lineCount > 0 && index < _logItems.size()) {
                    LogItem &item = _logItems[index++];
                    if (!HasFlag(_logLevel, item.level)) {
                        continue;
                    }
                    --lineCount;
                    OutputMessage(item.level, item.message);
                }
            }
            clipper.End();
        }

        ImGui::PopStyleVar();
        if (_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void ConsoleViewport::AddLog(Logger::DebugLevel level, const std::string &message) {
    std::lock_guard lock(_mutex);
    if (_logItems.size() > kMaxLogCount) {
        auto &front = _logItems.front();
        switch (front.level) {
        case Logger::DebugLevel::EL_Info:
            --_infoLogCount;
            break;
        case Logger::EL_Debug:
            --_debugLogCount;
            break;
        case Logger::EL_Warning:
            --_warningLogCount;
            break;
        case Logger::EL_Error:
            --_errorLogCount;
            break;
        default:
            break;
        }
        _logItems.pop_front();
    }

    _logItems.push_back({message, level});

    switch (level) {
    case Logger::DebugLevel::EL_Info:
        ++_infoLogCount;
        break;
    case Logger::EL_Debug:
        ++_debugLogCount;
        break;
    case Logger::EL_Warning:
        ++_warningLogCount;
        break;
    case Logger::EL_Error:
        ++_errorLogCount;
        break;
    default:
        break;
    }
}
