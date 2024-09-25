#include "wlp4type.h"

void processExpr(Node *root, Procedure &proc);
void processTerm(Node *root, Procedure &proc);
void processFactor(Node *root, Procedure &proc);
void processStatement(Node *root, Procedure &proc);
void processLValueFactor(Node *root, Procedure &proc);
void processLValueStatement(Node *root, Procedure &proc);
void processTest(Node *root, Procedure &proc, string endLabel);
void processStatements(Node *root, Procedure &proc);

int loopNum = 0;
string beginLoop = "loopBegin";
string endLoop = "loopEnd";

string elseLabel = "else";
string endIf = "endif";

string nullHandling = "skipNullHandling";

string procedurePrefix = "procedure";

void Add(int d, int s, int t)
{
    std::cout << "add $" << d << ", $" << s << ", $" << t << "\n";
}

void Sub(int d, int s, int t)
{
    std::cout << "sub $" << d << ", $" << s << ", $" << t << "\n";
}

void Mult(int d, int s, int t)
{
    cout << "mult $" << s << ", $" << t << "\n";
    cout << "mflo $" << d << "\n";
}

void Div(int d, int s, int t)
{
    cout << "div $" << s << ", $" << t << "\n";
    cout << "mflo $" << d << "\n";
}

void Mod(int d, int s, int t)
{
    cout << "div $" << s << ", $" << t << "\n";
    cout << "mfhi $" << d << "\n";
}

void Beq(int s, int t, std::string label)
{
    std::cout << "beq $" << s << ", $" << t << ", " + label + "\n";
}

void Bne(int s, int t, std::string label)
{
    std::cout << "bne $" << s << ", $" << t << ", " + label + "\n";
}

void Jr(int s)
{
    std::cout << "jr $" << s << "\n";
}

void Word(int i)
{
    std::cout << ".word " << i << "\n";
}

void Word(std::string label)
{
    std::cout << ".word " + label + "\n";
}

void lisReg(int reg, int s)
{
    cout << "lis $" << reg << "\n";
    Word(s);
}

void lisReg(int reg, string s)
{
    cout << "lis $" << reg << "\n";
    Word(s);
}

void Label(std::string name)
{
    std::cout << name + ":\n";
}

void Sw(int t, int i, int s)
{
    cout << "sw $" << t << ", " << i << "($" << s << ")\n";
}

void Lw(int t, int i, int s)
{
    cout << "lw $" << t << ", " << i << "($" << s << ")\n";
}

void push(int reg)
{
    std::cout << "sw $" << reg << ", -4($30)\n";
    std::cout << "sub $30, $30, $4\n";
}

void pop(int d)
{
    std::cout << "add $30, $30, $4\n";
    std::cout << "lw $" << d << ", -4($30)\n";
}

void pop()
{
    std::cout << "add $30, $30, $4\n";
}

void pushImmediate(int value)
{
    lisReg(3, value);
    std::cout << "sw $3, -4($30)\n";
    std::cout << "sub $30, $30, $4\n";
}

void moveReg(int d, int s)
{
    cout << "add $" << d << ", $0, $" << s << "\n";
}

void getVar(int offset)
{
    cout << "lw $3, " << -(offset * 4) << "($29)\n";
}

void Slt(int s, int t)
{
    cout << "slt $3, $" << s << ", $" << t << "\n";
}

void Sltu(int s, int t)
{
    cout << "sltu $3, $" << s << ", $" << t << "\n";
}

void doImport(string import)
{
    cout << ".import " << import << "\n";
}

void Jalr(int r)
{
    cout << "jalr $" << r << "\n";
}

void processExpr(Node *root, Procedure &proc)
{
    if (root->children.size() == 3)
    {
        string type1 = root->getChild("expr")->type;
        string type2 = root->getChild("term")->type;

        processExpr(root->getChild("expr"), proc);
        moveReg(5, 3);
        push(5);
        processTerm(root->getChild("term"), proc);
        pop(5);
        if (root->children.at(1)->token.kind == "PLUS")
        {
            if (type1 == "int*" and type2 == "int")
            {
                Mult(3, 3, 4);
            }
            else if (type1 == "int" and type2 == "int*")
            {
                Mult(5, 5, 4);
            }
            Add(3, 5, 3);
        }
        else if (root->children.at(1)->token.kind == "MINUS")
        {
            if (type1 == "int*" and type2 == "int")
            {
                Mult(3, 3, 4);
            }
            Sub(3, 5, 3);
            if (type1 == "int*" and type2 == "int*")
            {
                Div(3, 3, 4);
            }
        }
    }
    else
    {
        processTerm(root->getChild("term"), proc);
    }
}

void processTerm(Node *root, Procedure &proc)
{
    if (root->children.size() == 3)
    {
        processTerm(root->getChild("term"), proc);
        moveReg(5, 3);
        push(5);
        processFactor(root->getChild("factor"), proc);
        pop(5);
        if (root->children.at(1)->token.kind == "STAR")
        {
            Mult(3, 5, 3);
        }
        else if (root->children.at(1)->token.kind == "SLASH")
        {
            Div(3, 5, 3);
        }
        else if (root->children.at(1)->token.kind == "PCT")
        {
            Mod(3, 5, 3);
        }
    }
    else
    {
        processFactor(root->getChild("factor"), proc);
    }
}

void processFactor(Node *root, Procedure &proc)
{
    if (root->children.size() == 1 and root->children.at(0)->token.kind == "ID")
    {
        getVar(proc.getOffset(root->children.at(0)->token.lexeme));
    }
    else if (root->children.size() == 1 and root->children.at(0)->token.kind == "NUM")
    {
        lisReg(3, stoi(root->children.at(0)->token.lexeme));
    }
    else if (root->children.size() == 1 and root->children.at(0)->token.kind == "NULL")
    {
        lisReg(3, 1);
    }
    else if (root->children.size() == 3 and root->children.at(0)->token.kind == "LPAREN")
    {
        processExpr(root->getChild("expr"), proc);
    }
    else if (root->children.size() == 2 and root->children.at(0)->token.kind == "AMP")
    {
        processLValueFactor(root->getChild("lvalue"), proc);
    }
    else if (root->children.size() == 2 and root->children.at(0)->token.kind == "STAR")
    {
        processFactor(root->getChild("factor"), proc);
        Lw(3, 0, 3);
    }
    else if (root->children.size() == 5 and root->children.at(0)->token.kind == "NEW")
    {
        processExpr(root->getChild("expr"), proc);
        moveReg(1, 3);
        push(31);
        lisReg(5, "new");
        Jalr(5);
        pop(31);
        Bne(3, 0, nullHandling + to_string(loopNum));
        Add(3, 3, 11);
        Label(nullHandling + to_string(loopNum));
        loopNum++;
    }
    else if (root->children.size() == 3 and root->children.at(0)->token.kind == "ID")
    {
        push(31);
        push(29);
        lisReg(5, procedurePrefix + root->getChild("ID")->token.lexeme);
        Jalr(5);
        pop(29);
        pop(31);
    }
    else if (root->children.size() == 4 and root->children.at(0)->token.kind == "ID")
    {
        push(31);
        push(29);
        int count = 0;
        Node *arglist = root->getChild("arglist");
        while (arglist->children.size() == 3)
        {
            processExpr(arglist->getChild("expr"), proc);
            push(3);
            arglist = arglist->getChild("arglist");
            count++;
        }
        processExpr(arglist->getChild("expr"), proc);
        push(3);
        count++;
        lisReg(5, procedurePrefix + root->getChild("ID")->token.lexeme);
        Jalr(5);
        for (int i = 0; i < count; i++)
        {
            pop();
        }
        pop(29);
        pop(31);
    }
}

void processStatements(Node *root, Procedure &proc)
{
    if (root->children.size() > 0)
    {
        processStatements(root->getChild("statements"), proc);
        processStatement(root->getChild("statement"), proc);
    }
}

void processStatement(Node *root, Procedure &proc)
{
    Node *temp;
    if (root->children.size() == 4)
    {
        temp = root->getChild("lvalue");
        while (temp->children.size() == 3)
        {
            temp = temp->getChild("lvalue");
        }
    }
    if (root->children.size() == 4 and temp->children.size() == 1)
    {
        processExpr(root->getChild("expr"), proc);
        processLValueStatement(temp, proc);
    }
    else if (root->children.size() == 4 and temp->children.size() == 2)
    {
        processLValueStatement(temp, proc);
        push(3);
        processExpr(root->getChild("expr"), proc);
        pop(5);
        Sw(3, 0, 5);
    }
    else if (root->children.size() == 7 and root->children.at(0)->token.kind == "WHILE")
    {
        string endOfLoop = endLoop + to_string(loopNum);
        string startOfLoop = beginLoop + to_string(loopNum);
        loopNum++;
        Label(startOfLoop);
        push(3);
        processTest(root->getChild("test"), proc, endOfLoop);
        pop(3);
        processStatements(root->getChild("statements"), proc);
        Beq(0, 0, startOfLoop);
        Label(endOfLoop);
        pop(3);
    }
    else if (root->children.size() == 11)
    {
        string midIf = elseLabel + to_string(loopNum);
        string terminateIf = endIf + to_string(loopNum);
        loopNum++;
        processTest(root->getChild("test"), proc, midIf);
        processStatements(root->getChild("statements"), proc);
        Beq(0, 0, terminateIf);
        Label(midIf);
        processStatements(root->getChild("statements", 2), proc);
        Label(terminateIf);
    }
    else if (root->children.size() == 5 and root->children.at(0)->token.kind == "PRINTLN")
    {
        processExpr(root->getChild("expr"), proc);
        push(1);
        moveReg(1, 3);
        lisReg(5, "print");
        push(31);
        Jalr(5);
        pop(31);
        pop(1);
    }
    else if (root->children.size() == 5 and root->children.at(0)->token.kind == "DELETE")
    {
        processExpr(root->getChild("expr"), proc);
        moveReg(1, 3);
        Beq(1, 11, nullHandling + to_string(loopNum));
        lisReg(5, "delete");
        push(31);
        Jalr(5);
        pop(31);
        Label(nullHandling + to_string(loopNum));
        loopNum++;
    }
}

void processTest(Node *root, Procedure &proc, string endLabel)
{
    processExpr(root->getChild("expr"), proc);
    moveReg(5, 3);
    push(5);
    processExpr(root->getChild("expr", 2), proc);
    pop(5);
    if (root->children.at(1)->token.kind == "LT")
    {
        if (root->getChild("expr")->type == "int*")
        {
            Sltu(5, 3);
        }
        else
        {
            Slt(5, 3);
        }

        Bne(3, 11, endLabel);
    }
    else if (root->children.at(1)->token.kind == "GT")
    {
        if (root->getChild("expr")->type == "int*")
        {
            Sltu(3, 5);
        }
        else
        {
            Slt(3, 5);
        }
        Bne(3, 11, endLabel);
    }
    else if (root->children.at(1)->token.kind == "EQ")
    {
        Bne(3, 5, endLabel);
    }
    else if (root->children.at(1)->token.kind == "NE")
    {
        Beq(3, 5, endLabel);
    }
    else if (root->children.at(1)->token.kind == "LE")
    {
        Add(3, 3, 11);
        if (root->getChild("expr")->type == "int*")
        {
            Sltu(5, 3);
        }
        else
        {
            Slt(5, 3);
        }
        Bne(3, 11, endLabel);
    }
    else if (root->children.at(1)->token.kind == "GE")
    {
        Add(5, 5, 11);
        if (root->getChild("expr")->type == "int*")
        {
            Sltu(3, 5);
        }
        else
        {
            Slt(3, 5);
        }
        Bne(3, 11, endLabel);
    }
}

void processLValueFactor(Node *root, Procedure &proc)
{
    if (root->children.size() == 1)
    {
        lisReg(3, -proc.getOffset(root->getChild("ID")->token.lexeme) * 4);
        Add(3, 29, 3);
    }
    else if (root->children.size() == 2)
    {
        processFactor(root->getChild("factor"), proc);
    }
    else if (root->children.size() == 3)
    {
        processLValueFactor(root->getChild("lvalue"), proc);
    }
}

void processLValueStatement(Node *root, Procedure &proc)
{
    if (root->children.size() == 1)
    {
        Sw(3, -proc.getOffset(root->getChild("ID")->token.lexeme) * 4, 29);
    }
    else if (root->children.size() == 2)
    {
        processFactor(root->getChild("factor"), proc);
    }
    else if (root->children.size() == 3)
    {
        processLValueStatement(root->getChild("lvalue"), proc);
    }
}

void genProcs(Node *root, ProcedureTable &table)
{
    if (root->children.size() == 1)
    {
        Node *node = root->getChild("main");
        Label(procedurePrefix + "wain");
        // Instantiate constants
        lisReg(4, 4);
        lisReg(11, 1);

        // Push main arguments
        push(1);
        push(2);
        Sub(29, 30, 4);

        // Keep track of pushes
        int pushes = 2;

        // Process each declaration
        Node *dcls = node->getChild("dcls");
        while (dcls->children.size() > 0)
        {
            if (dcls->children.at(3)->token.kind == "NUM")
            {
                pushImmediate(stoi(dcls->children.at(3)->token.lexeme));
            }
            else
            {
                pushImmediate(1);
            }
            pushes++;
            dcls = dcls->getChild("dcls");
        }

        push(31);
        lisReg(5, "init");
        if (node->getChild("dcl")->getChild("type")->children.size() == 1)
        {
            lisReg(2, 0);
        }
        Jalr(5);
        pop(31);

        processStatements(node->getChild("statements"), table.get("wain"));
        processExpr(node->getChild("expr"), table.get("wain"));

        for (int i = 0; i < pushes; i++)
        {
            pop();
        }
        Jr(31);
        return;
    }
    else
    {
        genProcs(root->getChild("procedures"), table);
        Node *node = root->getChild("procedure");
        Label(procedurePrefix + node->getChild("ID")->token.lexeme);
        Sub(29, 30, 4);

        int pushes = 0;

        // Process each declaration
        Node *dcls = node->getChild("dcls");
        while (dcls->children.size() > 0)
        {
            if (dcls->children.at(3)->token.kind == "NUM")
            {
                pushImmediate(stoi(dcls->children.at(3)->token.lexeme));
            }
            else
            {
                pushImmediate(1);
            }
            pushes++;
            dcls = dcls->getChild("dcls");
        }

        processStatements(node->getChild("statements"), table.get(node->getChild("ID")->token.lexeme));
        processExpr(node->getChild("expr"), table.get(node->getChild("ID")->token.lexeme));

        for (int i = 0; i < pushes; i++)
        {
            pop();
        }

        Jr(31);
    }
}

void process(Node *root, ProcedureTable &table)
{
    doImport("print");
    doImport("init");
    doImport("new");
    doImport("delete");

    genProcs(root->getChild("procedures"), table);
}

int main()
{
    vector<int> *stateStack = new vector<int>;
    vector<Node *> *treeStack = new vector<Node *>;
    vector<rule> *cfg = activateCFG();
    tr = activateSLR();
    ProcedureTable table = ProcedureTable();
    try
    {
        std::stringstream s(DFAstring);
        std::stringstream t(WLP4_DFA);
        deque<Token> tokens = smm(erectDFA(s));
        tokens.push_front(Token("BOF", "BOF"));
        tokens.push_back(Token("EOF", "EOF"));
        if (tokens.size() < 3)
        {
            throw runtime_error("empty file");
        }
        stateStack->push_back(0);
        parse(stateStack, tokens, cfg, treeStack);
        collectProcedures(treeStack->at(0)->children.at(1), table);
        process(treeStack->at(0), table);
        // printT(treeStack->at(0), "");
        // printTree(treeStack->at(0));
        for (Node *node : *treeStack)
        {
            delete node;
        }
        delete treeStack;
        delete stateStack;
        delete cfg;
        delete tr;
    }
    catch (std::runtime_error &e)
    {
        for (Node *node : *treeStack)
        {
            delete node;
        }
        delete treeStack;
        delete stateStack;
        delete cfg;
        delete tr;
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}