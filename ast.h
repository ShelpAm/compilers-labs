#pragma once
#include <diagnostics.h>
#include <lexer.h>
#include <memory>
#include <vector>

class Parser;
class Lexer;

namespace ast {

class Node {
  public:
    Node() = default;
    Node(Node const &) = delete;
    Node(Node &&) = delete;
    Node &operator=(Node const &) = delete;
    Node &operator=(Node &&) = delete;
    virtual ~Node() = default;

    virtual void dump(std::ostream &os, int indent = 0) const = 0;

  protected:
    static void make_indent(std::ostream &os, int n)
    {
        for (int i = 0; i < n; ++i)
            os << "    ";
    }
};

class Statement : public Node {};
class Expression : public Node {};
class Declaration : public Node {};

class Program : public Node {
    friend class ::Parser;

  public:
    Program();

    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::vector<std::unique_ptr<Declaration>> decls_;
};

class CompoundStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::vector<std::unique_ptr<Statement>> stmts_;
};

class ReturnStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::unique_ptr<Expression> value_;
};

class DeclarationStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::unique_ptr<Declaration> decl_;
};

class ExpressionStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::unique_ptr<Expression> expr_;
};

class IdentifierExpr : public Expression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "IdentifierExpr(" << name_ << ")\n";
    }

  private:
    std::string name_;
};

class IntegerLiteralExpr : public Expression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "IntegerLiteral(" << value_ << ")\n";
    }

  private:
    std::string value_;
};

class FloatLiteralExpr : public Expression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "FloatLiteral(" << value_ << ")\n";
    }

  private:
    std::string value_;
};

class StringLiteralExpr : public Expression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "StringLiteral(" << value_ << ")\n";
    }

  private:
    std::string value_;
};

class VariableDeclaration : public Declaration {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "VariableDeclaration\n";

        make_indent(os, indent + 1);
        os << "Type: " << to_string(type_) << "\n";

        make_indent(os, indent + 1);
        os << "Name: " << name_ << "\n";

        if (init_) {
            make_indent(os, indent + 1);
            os << "Init:\n";
            init_->dump(os, indent + 2);
        }
    }

  private:
    TokenKind type_;
    std::string name_;
    std::unique_ptr<Expression> init_;
};

class FunctionDeclaration : public Declaration {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "FunctionDeclaration\n";

        make_indent(os, indent);
        os << "ReturnType: " << to_string(return_type_) << "\n";

        make_indent(os, indent);
        os << "Name: " << name_ << "\n";

        if (body_) {
            make_indent(os, indent + 1);
            os << "Body:\n";
            body_->dump(os, indent + 2);
        }
    }

  private:
    TokenKind return_type_;
    std::string name_;
    std::unique_ptr<CompoundStatement> body_;
};

} // namespace ast
