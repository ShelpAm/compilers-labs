#include <ranges>
#include <semantic/scope.h>

namespace semantic {

class Context {
  public:
    Scope *current_scope()
    {
        return current_scope_;
    }

    void push_scope()
    {
        scopes_.push_back(std::make_unique<Scope>(current_scope_));
        current_scope_ = scopes_.back().get();
    }

    void pop_scope()
    {
        if (current_scope_ == nullptr) {
            throw std::runtime_error("No scope to pop");
        }
        current_scope_ = current_scope_->parent();
    }

    void define_builtin_type(std::string const &name, BuiltinType const &type)
    {
        builtin_types_.insert({name, type});
    }

    Type *find_builtin_type(std::string const &name)
    {
        if (auto it = builtin_types_.find(name); it != builtin_types_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    Type *get_builtin_type(std::string const &name)
    {
        if (auto it = builtin_types_.find(name); it != builtin_types_.end()) {
            return &it->second;
        }
        throw std::runtime_error(
            std::format("Builtin type '{}' not found", name));
    }

    void dump(std::size_t indent) const
    {
        spdlog::debug("{}Context:", indent_string(indent));
        spdlog::debug("{}Builtin Types:", indent_string(indent + 1));
        for (auto const &[name, type] : builtin_types_) {
            spdlog::debug("{}    {}: size={}, typekind={}",
                          indent_string(indent + 1), name, type.size,
                          to_string(type.typekind));
        }
        spdlog::debug("{}Scopes:", indent_string(indent + 1));
        for (auto const &[i, scope] : std::views::enumerate(scopes_)) {
            scope->dump(indent + 2);
        }
    }

  private:
    Scope *current_scope_{};
    std::vector<std::unique_ptr<Scope>> scopes_;

    std::unordered_map<std::string, BuiltinType> builtin_types_;
};

} // namespace semantic
