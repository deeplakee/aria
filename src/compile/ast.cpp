#include "compile/ast.h"
#include "compile/astVisitor.h"

namespace aria {

void ProgramNode::accept(AstVisitor &visitor)
{
    visitor.visitProgramNode(this);
}

void ProgramNode::display(String indent)
{
    println("{}ProgramNode", indent);
    for (auto it = decls.begin(); it != decls.end(); ++it) {
        const bool isLast = (std::next(it) == decls.end());
        const auto prefix = isLast ? "└── " : "├── ";
        const auto connector = isLast ? add_indent : "│   ";

        println("{}decl:", indent + add_indent + prefix);
        (*it)->display(indent + add_indent + connector + add_indent);
    }
}

void FunDeclNode::accept(AstVisitor &visitor)
{
    visitor.visitFunDeclNode(this);
}

void FunDeclNode::display(String indent)
{
    println("{}FunDeclNode", indent);
    println("{}functionName:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", token_name.text);
    println("{}parameters:", indent + add_indent + "├── ");
    if (!params.empty()) {
        String names;
        const String delimiter = ", ";
        for (auto i = 0; i < params.size(); i++) {
            if (i + 1 == params.size()) {
                if (acceptsVarargs) {
                    names += "...";
                }
                names += params[i].text;
            } else {
                names += params[i].text + delimiter;
            }
        }
        println("{}{}", indent + add_indent + "│       ", names);
    }
    println("{}body:", indent + add_indent + "└── ");
    body->display(indent + add_indent + add_indent + add_indent);
}

void ClassDeclNode::accept(AstVisitor &visitor)
{
    visitor.visitClassDeclNode(this);
}

void ClassDeclNode::display(String indent)
{
    println("{}ClassDeclNode", indent);
    println("{}className:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", token_name.text);
    if (token_name.text != token_super_name.text) {
        println("{}superClassName:", indent + add_indent + "├── ");
        println("{}{}", indent + add_indent + "│       ", token_super_name.text);
    }
    if (methods.empty()) {
        println("{}methods:", indent + add_indent + "└── ");
    }

    for (auto it = methods.begin(); it != methods.end(); ++it) {
        const bool isLast = (std::next(it) == methods.end());
        const auto prefix = isLast ? "└── " : "├── ";
        const auto connector = isLast ? add_indent : "│   ";

        println("{}method:", indent + add_indent + prefix);
        (*it)->display(indent + add_indent + connector + add_indent);
    }
}

void VarDeclNode::accept(AstVisitor &visitor)
{
    visitor.visitVarDeclNode(this);
}

void VarDeclNode::display(String indent)
{
    println("{}VarDeclNode", indent);
    for (auto i = 0; i < names.size(); i++) {
        const bool isLast = (i + 1 == names.size());
        const auto prefix = isLast ? "└── " : "├── ";
        const auto connector = isLast ? add_indent : "│   ";

        println("{}{}:", indent + add_indent + prefix, names[i].text);
        exprs[i]->display(indent + add_indent + connector + add_indent);
    }
}

void BlockNode::accept(AstVisitor &visitor)
{
    visitor.visitBlockNode(this);
}

void BlockNode::display(String indent)
{
    println("{}BlockNode", indent);
    for (auto it = decls.begin(); it != decls.end(); ++it) {
        const bool isLast = (std::next(it) == decls.end());
        const auto prefix = isLast ? "└── " : "├── ";
        const auto connector = isLast ? add_indent : "│   ";

        println("{}decl:", indent + add_indent + prefix);
        (*it)->display(indent + add_indent + connector + add_indent);
    }
}

void IfStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitIfStmtNode(this);
}

void IfStmtNode::display(String indent)
{
    println("{}IfStmtNode", indent);
    println("{}condition:", indent + add_indent + "├── ");
    condition->display(indent + add_indent + "│   " + add_indent);
    if (elseBody == nullptr) {
        println("{}body:", indent + add_indent + "└── ");
        body->display(indent + add_indent + add_indent + add_indent);
    } else {
        println("{}body:", indent + add_indent + "├── ");
        body->display(indent + add_indent + "│   " + add_indent);
        println("{}elseBody:", indent + add_indent + "└── ");
        elseBody->display(indent + add_indent + add_indent + add_indent);
    }
}

void WhileStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitWhileStmtNode(this);
}

void WhileStmtNode::display(String indent)
{
    println("{}WhileStmtNode", indent);
    println("{}condition:", indent + add_indent + "├── ");
    condition->display(indent + add_indent + "│   " + add_indent);
    println("{}body:", indent + add_indent + "└── ");
    body->display(indent + add_indent + add_indent + add_indent);
}

void ForStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitForStmtNode(this);
}

void ForStmtNode::display(String indent)
{
    println("{}ForStmtNode", indent);

    println("{}varInit:", indent + add_indent + "├── ");
    if (varInit == nullptr) {
        println("{}{}", indent + add_indent + "│       ", "null");
    } else {
        varInit->display(indent + add_indent + "│   " + add_indent);
    }

    println("{}condition:", indent + add_indent + "├── ");
    if (condition == nullptr) {
        println("{}{}", indent + add_indent + "│       ", "null");
    } else {
        condition->display(indent + add_indent + "│   " + add_indent);
    }

    println("{}increment:", indent + add_indent + "├── ");
    if (increment == nullptr) {
        println("{}{}", indent + add_indent + "│       ", "null");
    } else {
        increment->display(indent + add_indent + "│   " + add_indent);
    }

    println("{}body:", indent + add_indent + "└── ");
    body->display(indent + add_indent + add_indent + add_indent);
}

void ForInStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitForInStmtNode(this);
}

void ForInStmtNode::display(String indent)
{
    println("{}ForInStmtNode", indent);
    println("{}var:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", token_name.text);
    println("{}expr:", indent + add_indent + "├── ");
    expr->display(indent + add_indent + "│   " + add_indent);
    println("{}body:", indent + add_indent + "└── ");
    body->display(indent + add_indent + add_indent + add_indent);
}

void BreakStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitBreakStmtNode(this);
}

void BreakStmtNode::display(String indent)
{
    println("{}BreakStmtNode", indent);
}

void ContinueStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitContinueStmtNode(this);
}

void ContinueStmtNode::display(String indent)
{
    println("{}ContinueStmtNode", indent);
}

void ReturnStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitReturnStmtNode(this);
}

void ReturnStmtNode::display(String indent)
{
    println("{}ReturnStmtNode", indent);
    println("{}expr:", indent + add_indent + "└── ");
    expr->display(indent + add_indent + add_indent + add_indent);
}

void ImportStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitImportStmtNode(this);
}

void ImportStmtNode::display(String indent)
{
    println("{}module:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", token_module.text);
    println("{}name:", indent + add_indent + "└── ");
    println("{}{}", indent + add_indent + "        ", token_name.text);
}

void TryCatchStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitTryCatchStmtNode(this);
}

void TryCatchStmtNode::display(String indent)
{
    println("{}TryCatchStmtNode", indent);
    println("{}tryBody:", indent + add_indent + "├── ");
    tryBody->display(indent + add_indent + "│   " + add_indent);
    println("{}exception:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", token_err.text);
    println("{}catchBody:", indent + add_indent + "└── ");
    catchBody->display(indent + add_indent + add_indent + add_indent);
}

void ThrowStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitThrowStmtNode(this);
}

void ThrowStmtNode::display(String indent)
{
    println("{}ThrowStmtNode", indent);
    println("{}e:", indent + add_indent + "└── ");
    e->display(indent + add_indent + add_indent + add_indent);
}

void PrintStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitPrintStmtNode(this);
}

void PrintStmtNode::display(String indent)
{
    println("{}PrintStmtNode", indent);
    println("{}expr:", indent + add_indent + "└── ");
    expr->display(indent + add_indent + add_indent + add_indent);
}

void ExprStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitExprStmtNode(this);
}

void ExprStmtNode::display(String indent)
{
    println("{}ExprStmtNode", indent);
    println("{}expr:", indent + add_indent + "└── ");
    expr->display(indent + add_indent + add_indent + add_indent);
}

void AssignExprNode::accept(AstVisitor &visitor)
{
    visitor.visitAssignExprNode(this);
}

void AssignExprNode::display(String indent)
{
    println("{}AssignExprNode", indent);
    println("{}op:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", op.typeStr());
    println("{}lhs:", indent + add_indent + "├── ");
    lhs->display(indent + add_indent + "│   " + add_indent);
    println("{}rhs:", indent + add_indent + "└── ");
    rhs->display(indent + add_indent + add_indent + add_indent);
}

void IncExprNode::accept(AstVisitor &visitor)
{
    visitor.visitIncExprNode(this);
}

void IncExprNode::display(String indent)
{
    println("{}IncExprNode", indent);
    println("{}op:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", op.typeStr());
    println("{}operand:", indent + add_indent + "└── ");
    expr->display(indent + add_indent + add_indent + add_indent);
}

void BinaryExprNode::accept(AstVisitor &visitor)
{
    visitor.visitBinaryExprNode(this);
}

void BinaryExprNode::display(String indent)
{
    println("{}BinaryExprNode", indent);
    println("{}op:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", op.typeStr());
    println("{}lhs:", indent + add_indent + "├── ");
    lhs->display(indent + add_indent + "│   " + add_indent);
    println("{}rhs:", indent + add_indent + "└── ");
    rhs->display(indent + add_indent + add_indent + add_indent);
}

void UnaryExprNode::accept(AstVisitor &visitor)
{
    visitor.visitUnaryExprNode(this);
}

void UnaryExprNode::display(String indent)
{
    println("{}UnaryExprNode", indent);
    println("{}op:", indent + add_indent + "├── ");
    println("{}{}", indent + add_indent + "│       ", op.typeStr());
    println("{}operand:", indent + add_indent + "└── ");
    operand->display(indent + add_indent + add_indent + add_indent);
}

void CallExprNode::accept(AstVisitor &visitor)
{
    visitor.visitCallExprNode(this);
}

void CallExprNode::display(String indent)
{
    println("{}CallExprNode", indent);
    println("{}calledExpr:", indent + add_indent + "├── ");
    calledExpr->display(indent + add_indent + "│   " + add_indent);
    if (args.empty()) {
        println("{}arguments:", indent + add_indent + "└── ");
    }
    for (auto it = args.begin(); it != args.end(); ++it) {
        const bool isLast = (std::next(it) == args.end());
        const auto prefix = isLast ? "└── " : "├── ";
        const auto connector = isLast ? add_indent : "│   ";

        println("{}argument:", indent + add_indent + prefix);
        (*it)->display(indent + add_indent + connector + add_indent);
    }
}

void FieldExprNode::accept(AstVisitor &visitor)
{
    visitor.visitFieldExprNode(this);
}

void FieldExprNode::display(String indent)
{
    println("{}FieldExprNode", indent);
    println("{}receiver:", indent + add_indent + "├── ");
    receiver->display(indent + add_indent + "│   " + add_indent);
    println("{}fieldName:", indent + add_indent + "└── ");
    println("{}{}", indent + add_indent + add_indent, token_field_name.text);
}

void IndexExprNode::accept(AstVisitor &visitor)
{
    visitor.visitIndexExprNode(this);
}

void IndexExprNode::display(String indent)
{
    println("{}IndexExprNode", indent);
    println("{}receiver:", indent + add_indent + "├── ");
    receiver->display(indent + add_indent + "│   " + add_indent);
    println("{}index:", indent + add_indent + "└── ");
    index->display(indent + add_indent + add_indent + add_indent);
}

void ListExprNode::accept(AstVisitor &visitor)
{
    visitor.visitListExprNode(this);
}

void ListExprNode::display(String indent)
{
    println("{}ListExprNode", indent);
    for (auto it = list.begin(); it != list.end(); ++it) {
        const bool isLast = (std::next(it) == list.end());
        const auto prefix = isLast ? "└── " : "├── ";
        const auto connector = isLast ? add_indent : "│   ";

        println("{}val:", indent + add_indent + prefix);
        (*it)->display(indent + add_indent + connector + add_indent);
    }
}

void MapExprNode::accept(AstVisitor &visitor)
{
    visitor.visitMapExprNode(this);
}

void MapExprNode::display(String indent)
{
    println("{}MapExprNode", indent);
    for (auto it = pairs.begin(); it != pairs.end(); ++it) {
        const bool isLast = (std::next(it) == pairs.end());
        const auto prefix = isLast ? "└── " : "├── ";
        const auto connector = isLast ? add_indent : "│   ";

        println("{}pair:", indent + add_indent + prefix);
        (*it)->display(indent + add_indent + connector + add_indent);
    }
}

void PairNode::accept(AstVisitor &visitor)
{
    visitor.visitPairNode(this);
}

void PairNode::display(String indent)
{
    println("{}PairNode", indent);
    println("{}key:", indent + add_indent + "├── ");
    key->display(indent + add_indent + "│   " + add_indent);
    println("{}value:", indent + add_indent + "└── ");
    value->display(indent + add_indent + add_indent + add_indent);
}

void VarNode::accept(AstVisitor &visitor)
{
    visitor.visitVarNode(this);
}

void VarNode::display(String indent)
{
    println("{}VarNode:{}", indent, var.text);
}

void NumberNode::accept(AstVisitor &visitor)
{
    visitor.visitNumberNode(this);
}

void NumberNode::display(String indent)
{
    println("{}NumberNode:{}", indent, num.text);
}

void StringNode::accept(AstVisitor &visitor)
{
    visitor.visitStringNode(this);
}

void StringNode::display(String indent)
{
    println("{}StringNode:{}", indent, str.text);
}

void TrueNode::accept(AstVisitor &visitor)
{
    visitor.visitTrueNode(this);
}

void TrueNode::display(String indent)
{
    println("{}TrueNode", indent);
}

void FalseNode::accept(AstVisitor &visitor)
{
    visitor.visitFalseNode(this);
}

void FalseNode::display(String indent)
{
    println("{}FalseNode", indent);
}

void NilNode::accept(AstVisitor &visitor)
{
    visitor.visitNilNode(this);
}

void NilNode::display(String indent)
{
    println("{}NilNode", indent);
}

void ErrorNode::accept(AstVisitor &visitor)
{
    visitor.visitErrorNode(this);
}

void ErrorNode::display(String indent)
{
    println("{}ErrorNode:{}", indent, err.text);
}

} // namespace aria