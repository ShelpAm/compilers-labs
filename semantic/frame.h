#pragma once
#include <ast/node.h>
#include <iostream>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>

namespace semantic {

class Frame {
  public:
    explicit Frame(std::unique_ptr<Frame> parent) : parent_(std::move(parent))
    {
    }

    std::optional<std::string> lookup_rvalue(std::string const &var)
    {
        auto *p = this;
        while (p != nullptr) {
            auto it = std::ranges::find(p->memory_, var,
                                        [](auto const &p) { return p.first; });
            if (it != p->memory_.end()) {
                return it->second;
            }
            p = p->parent_.get();
        }
        return std::nullopt;
    }

    // 暂不支持 uninitialized variable
    std::string *lookup_lvalue(std::string const &var)
    {
        auto *p = this;
        while (p != nullptr) {
            auto it = std::ranges::find(memory_, var,
                                        [](auto const &p) { return p.first; });
            if (it != memory_.end()) {
                return &it->second;
            }
            p = p->parent_.get();
        }
        return nullptr;
    }

    void new_variable(std::string var, std::string value)
    {
        memory_.push_back({std::move(var), std::move(value)});
    }

    [[nodiscard]] std::unique_ptr<Frame> const &parent() const
    {
        return parent_;
    }

    [[nodiscard]] std::unique_ptr<Frame> &parent()
    {
        return parent_;
    }

    void dump(std::ostream &os, int indent = 0)
    {
        if (indent > 1024) {
            spdlog::error("Stack depth exceeds maximum, 1024. Exiting.");
            std::exit(1);
        }
        make_indent(os, indent);
        std::println(os, "Frame:");
        make_indent(os, indent + 1);
        std::println(os, "Memory:");
        for (auto const &[name, content] : memory_) {
            make_indent(os, indent + 2);
            std::println(os, "{}: {}", name, content);
        }
    }

  private:
    std::vector<std::pair<std::string, std::string>> memory_;
    std::unique_ptr<Frame> parent_;
};

} // namespace semantic
