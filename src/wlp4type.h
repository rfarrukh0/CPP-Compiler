#include "wlp4parse.h"

struct Variable
{
    string name;
    string type;

    Variable() {}

    Variable(Node *root)
    {
        Node *type = root->children.at(0);
        Node *name = root->children.at(1);
        this->name = name->token.lexeme;
        if (type->children.size() == 1)
        {
            this->type = "int";
        }
        else
        {
            this->type = "int*";
        }
    }
};

struct VariableTable
{
    map<string, Variable> instances;

    void add(Variable &var)
    {
        if (instances.find(var.name) == instances.end())
        {
            instances[var.name] = var;
        }
        else
        {
            throw runtime_error("duplicate variable declaration");
        }
    }

    Variable &get(string name)
    {
        if (instances.find(name) == instances.end())
        {
            throw runtime_error("use of undeclared variable");
        }
        else
        {
            return instances[name];
        }
    }
};

struct Procedure
{
    string name;
    vector<string> signatures;
    vector<string> vars;
    VariableTable table;

    Procedure() {}

    Procedure(Node *root)
    {
        if (root->data.lhs == "procedure")
        {
            name = root->getChild("ID")->token.lexeme;
            Node *temp = root->getChild("params");
            if (temp->children.size() > 0)
            {
                temp = temp->getChild("paramlist");
                while (temp->children.size() == 3)
                {
                    Variable var = Variable(temp->getChild("dcl"));
                    vars.push_back(var.name);
                    table.add(var);
                    signatures.push_back(var.type);
                    temp = temp->getChild("paramlist");
                }
                Variable var = Variable(temp->getChild("dcl"));
                vars.push_back(var.name);
                table.add(var);
                signatures.push_back(var.type);
            }
        }
        else if (root->data.lhs == "main")
        {
            name = "wain";
            Variable var1 = Variable(root->getChild("dcl"));
            vars.push_back(var1.name);
            signatures.push_back(var1.name);
            table.add(var1);

            Variable var2 = Variable(root->getChild("dcl", 2));
            vars.push_back(var2.name);
            signatures.push_back(var2.name);
            table.add(var2);
        }
        Node *dcls = root->getChild("dcls");
        Variable temp;
        while (dcls->children.size() > 0)
        {
            temp = Variable(dcls->getChild("dcl"));
            vars.push_back(temp.name);
            table.add(temp);
            dcls = dcls->getChild("dcls");
        }
    }

    int getOffset(const std::string &name)
    {
        for (int i = 0; i < vars.size(); ++i)
        {
            if (vars[i] == name)
            {
                return i - signatures.size();
            }
        }
        return -1;
    }
};

struct ProcedureTable
{
    map<string, Procedure> instances;

    ProcedureTable() {}

    void add(Procedure &var)
    {
        if (instances.find(var.name) == instances.end())
        {
            instances[var.name] = var;
        }
        else
        {
            throw runtime_error("duplicate procedure declaration");
        }
    }

    Procedure &get(string name)
    {
        if (instances.find(name) == instances.end())
        {
            throw runtime_error("use of undeclared procedure");
        }
        else
        {
            return instances[name];
        }
    }
};

void annotateTypes(Node *tree, ProcedureTable &procedureTable, Procedure &currentProcedure)
{
    for (Node *child : tree->children)
    {
        annotateTypes(child, procedureTable, currentProcedure);
    }
    if (tree->data.lhs == "expr")
    {
        if (tree->data.rhs.size() == 1)
            tree->type = tree->getChild("term")->type;
        if (tree->data.rhs.size() == 3)
        {
            string firstType = tree->getChild("expr")->type;
            string secondType = tree->getChild("term")->type;
            if (tree->children.at(1)->token.kind == "PLUS")
            {
                if (firstType == "int" and secondType == "int")
                {
                    tree->type = "int";
                }
                else if (firstType != secondType)
                {
                    tree->type = "int*";
                }
                else
                {

                    throw runtime_error("plus on two pointers");
                }
            }
            else
            {
                if (firstType == "int" and secondType == "int")
                {
                    tree->type = "int";
                }
                else if (firstType == "int*" and secondType == "int")
                {
                    tree->type = "int*";
                }
                else if (firstType == "int*" and secondType == "int*")
                {
                    tree->type = "int";
                }
                else
                {
                    throw runtime_error("minus on a pointer and int");
                }
            }
        }
    }
    else if (tree->data.lhs == "factor")
    {
        if (tree->data.rhs.size() == 1)
        {
            if (tree->data.rhs.at(0) == "NUM")
            {
                tree->type = "int";
            }
            else if (tree->data.rhs.at(0) == "NULL")
            {
                tree->type = "int*";
            }
            else if (tree->data.rhs.at(0) == "ID")
            {
                Node *idNode = tree->getChild("ID");
                idNode->type = currentProcedure.table.get(idNode->token.lexeme).type;
                tree->type = idNode->type;
            }
            else
            {
                throw runtime_error("wrong type following factor");
            }
        }
        else if (tree->data.rhs.size() == 2)
        {
            if (tree->data.rhs.at(0) == "AMP")
            {
                tree->type = "int*";
                if (tree->children.at(1)->type != "int")
                {
                    throw runtime_error("type of factor preceded by AMP not int");
                }
            }
            else if (tree->data.rhs.at(0) == "STAR")
            {
                tree->type = "int";
                if (tree->children.at(1)->type != "int*")
                {
                    throw runtime_error("type of factor preceded by STAR not int*");
                }
            }
        }
        else if (tree->data.rhs.size() == 3)
        {
            if (tree->data.rhs.at(0) == "LPAREN")
            {
                tree->type = tree->children.at(1)->type;
            }
            else if (tree->data.rhs.at(0) == "ID")
            {
                tree->type = "int";
                // cout << tree->children.at(0)->token.lexeme<<endl;
                // cout << procedureTable.get(tree->children.at(0)->token.lexeme).signatures.size()<<endl;
                bool local = currentProcedure.table.instances.find(tree->children.at(0)->token.lexeme) != currentProcedure.table.instances.end();
                if (local)
                {
                    throw runtime_error("procedure called in procedure with id already used");
                }
                if (!local)
                {
                    procedureTable.get(tree->children.at(0)->token.lexeme);
                }
                if (!procedureTable.get(tree->children.at(0)->token.lexeme).signatures.empty())
                {
                    throw runtime_error("signatures not empty");
                }
            }
        }
        else if (tree->data.rhs.size() == 4)
        {
            tree->type = "int";
            int numExprs = 0;
            Node *temp = tree->getChild("arglist");
            string idName = tree->getChild("ID")->token.lexeme;
            bool local = currentProcedure.table.instances.find(idName) != currentProcedure.table.instances.end();
            if (local)
            {
                throw runtime_error("procedure called in procedure with local id in use");
            }
            vector<string> &signs = procedureTable.get(idName).signatures;

            if (signs.empty())
            {
                throw runtime_error("argument provided; shouldn't be any");
            }

            while (temp->children.size() == 3)
            {
                if (numExprs >= signs.size())
                {
                    throw runtime_error("too many arguments");
                }
                if (temp->children.at(0)->type != signs.at(numExprs))
                {
                    throw runtime_error("id not in signature");
                }
                numExprs++;
                temp = temp->children.at(2);
            }
            if (numExprs >= signs.size())
            {
                throw runtime_error("too many arguments");
            }
            if (temp->children.at(0)->type != signs.at(numExprs))
            {
                throw runtime_error("id not in signature");
            }
            numExprs++;

            if (numExprs != signs.size())
            {
                throw runtime_error("sizes do not match");
            }
        }
        else if (tree->data.rhs.size() == 5)
        {
            tree->type = "int*";
            if (tree->children.at(3)->type != "int")
            {
                throw runtime_error("type of expr not int");
            }
        }
    }
    else if (tree->data.lhs == "lvalue")
    {
        if (tree->data.rhs.at(0) == "ID")
        {
            Node *idNode = tree->getChild("ID");
            idNode->type = currentProcedure.table.get(idNode->token.lexeme).type;
            tree->type = idNode->type;
        }
        else if (tree->data.rhs.at(0) == "STAR")
        {
            tree->type = "int";
            if (tree->children.at(1)->type != "int*")
            {
                throw runtime_error("type of lvalue preceded by STAR not int*");
            }
        }
        else if (tree->data.rhs.at(0) == "LPAREN")
        {
            tree->type = tree->children.at(1)->type;
        }
        else
        {
            throw runtime_error("lvalue smth");
        }
    }
    else if (tree->data.lhs == "term")
    {
        if (tree->data.rhs.at(0) == "factor" and tree->data.rhs.size() == 1)
        {
            tree->type = tree->children.at(0)->type;
        }
        else
        {
            tree->type = "int";
            if (tree->children.at(0)->type != "int" or tree->children.at(2)->type != "int")
            {
                throw runtime_error("term or factor not int");
            }
        }
    }
    else if (tree->data.lhs == "statement")
    {
        if (tree->data.rhs.at(0) == "lvalue" and tree->getChild("lvalue")->type != tree->getChild("expr")->type)
        {
            throw runtime_error("lvalue and expr following statement should be same type");
        }
        else if (tree->data.rhs.at(0) == "PRINTLN" and tree->getChild("expr")->type != "int")
        {
            throw runtime_error("expr in statement must be int");
        }
        else if (tree->data.rhs.at(0) == "DELETE" and tree->getChild("expr")->type != "int*")
        {
            throw runtime_error("expr in statement must be int*");
        }
    }
    else if (tree->data.lhs == "test")
    {
        if (tree->getChild("expr")->type != tree->getChild("expr", 2)->type)
        {
            throw runtime_error("expression not the same type in test");
        }
    }
    else if (tree->data.lhs == "dcls")
    {
        if (tree->data.rhs.size() > 0)
        {
            if (tree->children.at(3)->token.kind == "NUM" and currentProcedure.table.get(tree->getChild("dcl")->getChild("ID")->token.lexeme).type != "int")
            {
                throw runtime_error("type of dcl should be int");
            }
            else if (tree->children.at(3)->token.kind == "NULL" and currentProcedure.table.get(tree->getChild("dcl")->getChild("ID")->token.lexeme).type != "int*")
            {
                throw runtime_error("type of dcl should be int*");
            }
        }
    }
}

void collectProcedures(Node *root, ProcedureTable &table)
{
    Node *temp = root;
    Procedure tempProcedure;
    while (temp->children.size() == 2)
    {
        tempProcedure = Procedure(temp->children.at(0));
        table.add(tempProcedure);
        annotateTypes(temp->children.at(0), table, tempProcedure);
        if (temp->children.at(0)->getChild("expr")->type != "int")
        {
            throw runtime_error("expr following procedure not int");
        }
        temp = temp->children.at(1);
    }
    tempProcedure = Procedure(temp->children.at(0));
    table.add(tempProcedure);
    temp = temp->children[0];
    annotateTypes(temp, table, tempProcedure);
    if (temp->getChild("dcl", 2)->getChild("type")->children.size() != 1)
    {
        throw runtime_error("second type not int");
    }
    if (temp->getChild("expr")->type != "int")
    {
        throw runtime_error("expr in main not int");
    }
};

void printT(Node *root, string indentation)
{
    cout << (root->terminal ? root->token.kind : root->data.lhs) + " " + root->type << endl;

    if (root->children.size() == 0 or root->children.size() == 1 and root->children[0]->data.rhs.size() > 0 and root->children[0]->data.rhs[0] == ".EMPTY")
        return;

    for (int i = 0; i < (root->children.size()) - 1; i++)
    {
        cout << indentation << "├─";
        printT(root->children.at(i), indentation + "│ ");
    }

    cout << indentation << "╰─";
    printT(root->children.at((root->children.size()) - 1), indentation + "  ");
}
