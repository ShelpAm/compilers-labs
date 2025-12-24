#include <semantic/semantic-analyzer.h>

#include <algorithm>
#include <diagnostics.h>
#include <ranges>
#include <semantic/context.h>

semantic::SemanticAnalyzer::SemanticAnalyzer(Context *ctx, Diagnostics *diags)
    : ctx_(ctx), diags_(diags)
{
    using enum TypeKind;
    auto p = std::make_unique<BasicType>(4, integer_type);
    ctx_->named_types_.insert({"int", std::move(p)});

    p = std::make_unique<BasicType>(4, float_type);
    ctx_->named_types_.insert({"float", std::move(p)});

    p = std::make_unique<BasicType>(4, string_type);
    ctx_->named_types_.insert({"string", std::move(p)});
}

void semantic::SemanticAnalyzer::visit(ast::Program &prog)
{
    for (auto const &decl : prog.decls_) {
        decl->accept(*this);
    }
}

void semantic::SemanticAnalyzer::summary() const
{
    std::println("Unnamed Types:");
    for (auto const &[i, type] : std::views::enumerate(ctx_->unnamed_types_)) {
        std::println("    {}:", i);
        std::println("        Size: {}", type->size);
        std::println("        TypeKind: {}", to_string(type->typekind));
        std::println("        Return Type: {}",
                     to_string(type->return_type->typekind));
        std::println("        Parameter Types:");
        for (auto const &p : type->parameter_types) {
            std::println("            {}", to_string(p->typekind));
        }
    }

    std::println("Named Types:");
    for (auto const &[name, type] : ctx_->named_types_) {
        std::println("    {}:", name);
        std::println("        Size: {}", type->size);
        std::println("        TypeKind: {}", to_string(type->typekind));
    }

    ctx_->symbol_table_.summary();
}

void semantic::SemanticAnalyzer::visit(ast::VariableDeclaration &vd)
{
    BasicType *type = find_type_by_name(vd.type().value);
    if (type == nullptr) {
        diags_->error("Unknown type '{}'", vd.type().value);
        return;
    }

    if (!ctx_->symbol_table_.define(
            vd.name(), Symbol{.name = vd.name(),
                              .type_ptr = type,
                              .symbolkind = SymbolKind::variable})) {
        diags_->error("Symbol already exists '{}'", vd.name());
    }

    vd.set_symbol(ctx_->symbol_table_.lookup(vd.name()));
}

void semantic::SemanticAnalyzer::visit(ast::FunctionDeclaration &fd)
{
    if (ctx_->symbol_table_.lookup(fd.name()) != nullptr) {
        diags_->error("Function re-declaration error: {}", fd.name());
        return;
    }

    // Constructs a function type instance
    std::vector<BasicType *> param_types;
    for (auto const &[paramtype, name] : fd.parameters()) {
        param_types.push_back(find_type_by_name(paramtype));
    }
    auto ft_ptr = std::make_unique<FunctionType>(
        BasicType{.size = 4, .typekind = TypeKind::function_type},
        find_type_by_name(fd.return_type().value), param_types, &fd);

    ctx_->unnamed_types_.push_back(std::move(ft_ptr));
    auto &ptr = ctx_->unnamed_types_.back();

    // auto it = std::ranges::find(ctx_->unnamed_types_, *ft_ptr,
    //                             [](auto const &p) { return *p; });
    // if (it == ctx_->unnamed_types_.end()) {
    //     ctx_->unnamed_types_.push_back(std::move(ft_ptr));
    //     it = std::prev(ctx_->unnamed_types_.end());
    // }
    ctx_->symbol_table_.define(fd.name(),
                               Symbol{.name = fd.name(),
                                      .type_ptr = ptr.get(),
                                      .symbolkind = SymbolKind::type});

    fd.set_symbol(ctx_->symbol_table_.lookup(fd.name()));

    fd.body()->accept(*this);
}

void semantic::SemanticAnalyzer::visit(ast::CompoundStatement &cs)
{
    for (auto const &stmt : cs.statments()) {
        stmt->accept(*this);
    }
}

void semantic::SemanticAnalyzer::visit(ast::DeclarationStatement &ds)
{
    ds.declaration()->accept(*this);
}

void semantic::SemanticAnalyzer::visit(ast::IntegerLiteralExpr &ie)
{
    auto *p = find_type_by_name("int");
    assert(p && "int should be found");
    ie.set_type(p);
}
void semantic::SemanticAnalyzer::visit(ast::FloatLiteralExpr &fe)
{
    auto *p = find_type_by_name("float");
    assert(p && "float should be found");
    fe.set_type(p);
}
void semantic::SemanticAnalyzer::visit(ast::StringLiteralExpr &se)
{
    auto *p = find_type_by_name("string");
    assert(p && "string should be found");
    se.set_type(p);
}

semantic::BasicType *
semantic::SemanticAnalyzer::find_type_by_name(std::string_view name)
{
    auto it = ctx_->named_types_.find(std::string{name});
    return it != ctx_->named_types_.end() ? it->second.get() : nullptr;
}
