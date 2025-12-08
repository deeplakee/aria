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
        const char *prefix = isLast ? "└── " : "├── ";
        const char *connector = isLast ? ADDED_INDENT : "│   ";

        println("{}decl:", indent + ADDED_INDENT + prefix);
        (*it)->display(indent + ADDED_INDENT + connector + ADDED_INDENT);
    }
}

void FunDeclNode::accept(AstVisitor &visitor)
{
    visitor.visitFunDeclNode(this);
}

void FunDeclNode::display(String indent)
{
    println("{}FunDeclNode", indent);
    println("{}functionName:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", funNameToken.text);
    println("{}parameters:", indent + ADDED_INDENT + "├── ");
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
        println("{}{}", indent + ADDED_INDENT + "│       ", names);
    }
    println("{}body:", indent + ADDED_INDENT + "└── ");
    body->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void ClassDeclNode::accept(AstVisitor &visitor)
{
    visitor.visitClassDeclNode(this);
}

void ClassDeclNode::display(String indent)
{
    println("{}ClassDeclNode", indent);
    println("{}className:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", nameToken.text);
    if (nameToken.text != superNameToken.text) {
        println("{}superClassName:", indent + ADDED_INDENT + "├── ");
        println("{}{}", indent + ADDED_INDENT + "│       ", superNameToken.text);
    }
    if (methods.empty()) {
        println("{}methods:", indent + ADDED_INDENT + "└── ");
    }

    for (auto it = methods.begin(); it != methods.end(); ++it) {
        const bool isLast = (std::next(it) == methods.end());
        const char *prefix = isLast ? "└── " : "├── ";
        const char *connector = isLast ? ADDED_INDENT : "│   ";

        println("{}method:", indent + ADDED_INDENT + prefix);
        (*it)->display(indent + ADDED_INDENT + connector + ADDED_INDENT);
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
        const char *prefix = isLast ? "└── " : "├── ";
        const char *connector = isLast ? ADDED_INDENT : "│   ";

        println("{}{}:", indent + ADDED_INDENT + prefix, names[i].text);
        exprs[i]->display(indent + ADDED_INDENT + connector + ADDED_INDENT);
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
        const char *prefix = isLast ? "└── " : "├── ";
        const char *connector = isLast ? ADDED_INDENT : "│   ";

        println("{}decl:", indent + ADDED_INDENT + prefix);
        (*it)->display(indent + ADDED_INDENT + connector + ADDED_INDENT);
    }
}

void IfStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitIfStmtNode(this);
}

void IfStmtNode::display(String indent)
{
    println("{}IfStmtNode", indent);
    println("{}condition:", indent + ADDED_INDENT + "├── ");
    condition->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    if (elseBody == nullptr) {
        println("{}body:", indent + ADDED_INDENT + "└── ");
        body->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
    } else {
        println("{}body:", indent + ADDED_INDENT + "├── ");
        body->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
        println("{}elseBody:", indent + ADDED_INDENT + "└── ");
        elseBody->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
    }
}

void WhileStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitWhileStmtNode(this);
}

void WhileStmtNode::display(String indent)
{
    println("{}WhileStmtNode", indent);
    println("{}condition:", indent + ADDED_INDENT + "├── ");
    condition->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    println("{}body:", indent + ADDED_INDENT + "└── ");
    body->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void ForStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitForStmtNode(this);
}

void ForStmtNode::display(String indent)
{
    println("{}ForStmtNode", indent);

    println("{}varInit:", indent + ADDED_INDENT + "├── ");
    if (varInit == nullptr) {
        println("{}{}", indent + ADDED_INDENT + "│       ", "null");
    } else {
        varInit->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    }

    println("{}condition:", indent + ADDED_INDENT + "├── ");
    if (condition == nullptr) {
        println("{}{}", indent + ADDED_INDENT + "│       ", "null");
    } else {
        condition->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    }

    println("{}increment:", indent + ADDED_INDENT + "├── ");
    if (increment == nullptr) {
        println("{}{}", indent + ADDED_INDENT + "│       ", "null");
    } else {
        increment->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    }

    println("{}body:", indent + ADDED_INDENT + "└── ");
    body->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void ForInStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitForInStmtNode(this);
}

void ForInStmtNode::display(String indent)
{
    println("{}ForInStmtNode", indent);
    println("{}var:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", iterNameToken.text);
    println("{}expr:", indent + ADDED_INDENT + "├── ");
    expr->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    println("{}body:", indent + ADDED_INDENT + "└── ");
    body->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
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
    println("{}expr:", indent + ADDED_INDENT + "└── ");
    expr->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void ImportStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitImportStmtNode(this);
}

void ImportStmtNode::display(String indent)
{
    println("{}module:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", moduleToken.text);
    println("{}name:", indent + ADDED_INDENT + "└── ");
    println("{}{}", indent + ADDED_INDENT + "        ", nameToken.text);
}

void TryCatchStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitTryCatchStmtNode(this);
}

void TryCatchStmtNode::display(String indent)
{
    println("{}TryCatchStmtNode", indent);
    println("{}tryBody:", indent + ADDED_INDENT + "├── ");
    tryBody->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    println("{}exception:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", errToken.text);
    println("{}catchBody:", indent + ADDED_INDENT + "└── ");
    catchBody->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void ThrowStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitThrowStmtNode(this);
}

void ThrowStmtNode::display(String indent)
{
    println("{}ThrowStmtNode", indent);
    println("{}e:", indent + ADDED_INDENT + "└── ");
    e->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void PrintStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitPrintStmtNode(this);
}

void PrintStmtNode::display(String indent)
{
    println("{}PrintStmtNode", indent);
    println("{}expr:", indent + ADDED_INDENT + "└── ");
    expr->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void ExprStmtNode::accept(AstVisitor &visitor)
{
    visitor.visitExprStmtNode(this);
}

void ExprStmtNode::display(String indent)
{
    println("{}ExprStmtNode", indent);
    println("{}expr:", indent + ADDED_INDENT + "└── ");
    expr->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void AssignExprNode::accept(AstVisitor &visitor)
{
    visitor.visitAssignExprNode(this);
}

void AssignExprNode::display(String indent)
{
    println("{}AssignExprNode", indent);
    println("{}op:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", opToken.typeStr());
    println("{}lhs:", indent + ADDED_INDENT + "├── ");
    lhs->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    println("{}rhs:", indent + ADDED_INDENT + "└── ");
    rhs->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void IncExprNode::accept(AstVisitor &visitor)
{
    visitor.visitIncExprNode(this);
}

void IncExprNode::display(String indent)
{
    println("{}IncExprNode", indent);
    println("{}op:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", opToken.typeStr());
    println("{}operand:", indent + ADDED_INDENT + "└── ");
    operand->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void BinaryExprNode::accept(AstVisitor &visitor)
{
    visitor.visitBinaryExprNode(this);
}

void BinaryExprNode::display(String indent)
{
    println("{}BinaryExprNode", indent);
    println("{}op:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", opToken.typeStr());
    println("{}lhs:", indent + ADDED_INDENT + "├── ");
    lhs->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    println("{}rhs:", indent + ADDED_INDENT + "└── ");
    rhs->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void UnaryExprNode::accept(AstVisitor &visitor)
{
    visitor.visitUnaryExprNode(this);
}

void UnaryExprNode::display(String indent)
{
    println("{}UnaryExprNode", indent);
    println("{}op:", indent + ADDED_INDENT + "├── ");
    println("{}{}", indent + ADDED_INDENT + "│       ", opToken.typeStr());
    println("{}operand:", indent + ADDED_INDENT + "└── ");
    operand->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void CallExprNode::accept(AstVisitor &visitor)
{
    visitor.visitCallExprNode(this);
}

void CallExprNode::display(String indent)
{
    println("{}CallExprNode", indent);
    println("{}calledExpr:", indent + ADDED_INDENT + "├── ");
    callee->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    if (args.empty()) {
        println("{}arguments:", indent + ADDED_INDENT + "└── ");
    }
    for (auto it = args.begin(); it != args.end(); ++it) {
        const bool isLast = (std::next(it) == args.end());
        const char *prefix = isLast ? "└── " : "├── ";
        const char *connector = isLast ? ADDED_INDENT : "│   ";

        println("{}argument:", indent + ADDED_INDENT + prefix);
        (*it)->display(indent + ADDED_INDENT + connector + ADDED_INDENT);
    }
}

void FieldExprNode::accept(AstVisitor &visitor)
{
    visitor.visitFieldExprNode(this);
}

void FieldExprNode::display(String indent)
{
    println("{}FieldExprNode", indent);
    println("{}receiver:", indent + ADDED_INDENT + "├── ");
    receiver->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    println("{}fieldName:", indent + ADDED_INDENT + "└── ");
    println("{}{}", indent + ADDED_INDENT + ADDED_INDENT, fieldNameToken.text);
}

void IndexExprNode::accept(AstVisitor &visitor)
{
    visitor.visitIndexExprNode(this);
}

void IndexExprNode::display(String indent)
{
    println("{}IndexExprNode", indent);
    println("{}receiver:", indent + ADDED_INDENT + "├── ");
    receiver->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    println("{}index:", indent + ADDED_INDENT + "└── ");
    index->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
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
        const char *prefix = isLast ? "└── " : "├── ";
        const char *connector = isLast ? ADDED_INDENT : "│   ";

        println("{}val:", indent + ADDED_INDENT + prefix);
        (*it)->display(indent + ADDED_INDENT + connector + ADDED_INDENT);
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
        const char *prefix = isLast ? "└── " : "├── ";
        const char *connector = isLast ? ADDED_INDENT : "│   ";

        println("{}pair:", indent + ADDED_INDENT + prefix);
        (*it)->display(indent + ADDED_INDENT + connector + ADDED_INDENT);
    }
}

void PairNode::accept(AstVisitor &visitor)
{
    visitor.visitPairNode(this);
}

void PairNode::display(String indent)
{
    println("{}PairNode", indent);
    println("{}key:", indent + ADDED_INDENT + "├── ");
    key->display(indent + ADDED_INDENT + "│   " + ADDED_INDENT);
    println("{}value:", indent + ADDED_INDENT + "└── ");
    value->display(indent + ADDED_INDENT + ADDED_INDENT + ADDED_INDENT);
}

void VarNode::accept(AstVisitor &visitor)
{
    visitor.visitVarNode(this);
}

void VarNode::display(String indent)
{
    println("{}VarNode:{}", indent, varNameToken.text);
}

void NumberNode::accept(AstVisitor &visitor)
{
    visitor.visitNumberNode(this);
}

void NumberNode::display(String indent)
{
    println("{}NumberNode:{}", indent, numToken.text);
}

void StringNode::accept(AstVisitor &visitor)
{
    visitor.visitStringNode(this);
}

void StringNode::display(String indent)
{
    println("{}StringNode:{}", indent, strToken.text);
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
    println("{}ErrorNode:{}", indent, errToken.text);
}

} // namespace aria