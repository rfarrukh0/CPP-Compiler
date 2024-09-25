#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <deque>
#include <set>
using namespace std;
#include "wlp4data.h"
#include "dfa.h"

//// These helper functions are defined at the bottom of the file:
// Check if a string is a single character.
bool isChar(std::string s);
// Check if a string represents a character range.
bool isRange(std::string s);
// Remove leading and trailing whitespace from a string, and
// squish intermediate whitespace sequences down to one space each.
std::string squish(std::string s);
// Convert hex digit character to corresponding number.
int hexToNum(char c);
// Convert number to corresponding hex digit character.
char numToHex(int d);
// Replace all escape sequences in a string with corresponding characters.
std::string escape(std::string s);
// Convert non-printing characters or spaces in a string into escape sequences.
std::string unescape(std::string s);

const std::string STATES = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT = ".INPUT";

struct dfa
{
    vector<string> states;
    map<string, bool> accept;
    map<string, map<char, string>> ns;

    string gns(string state, char c)
    {
        if (ns.find(state) != ns.end() and ns[state].find(c) != ns[state].end())
        {
            return ns[state][c];
        }
        return "ns not found";
    }
};

struct Token
{
    string kind;
    string lexeme;
    Token() {}
    Token(string kind, string lexeme) : kind(kind), lexeme(lexeme) {}
};

vector<string> split(string line)
{
    vector<string> tokens;
    if (int(line.length()) == 0)
        return tokens;
    string cur = "";
    for (char c : line)
    {
        if (c == *" ")
        {
            if (!cur.empty())
                tokens.push_back(cur);
            cur = "";
        }
        else
        {
            cur += c;
        }
    }
    if (!cur.empty())
        tokens.push_back(cur);
    return tokens;
}

vector<string> splitString(string input)
{
    vector<string> lines;
    istringstream iss(input);
    string line;

    while (std::getline(iss, line, '\n'))
    {
        lines.push_back(line);
    }

    return lines;
}

bool validateToken(Token token)
{
    if (token.kind == "NUM")
    {
        if (int(token.lexeme.length()) == 0)
        {
            return false;
        }
        if (int(token.lexeme.length()) > 1 and token.lexeme.at(0) == '0')
        {
            return false;
        }
        // try
        // {
        string upperBound = "2147483647";
        if (int(token.lexeme.length()) > 10)
            return false;
        if (int(token.lexeme.length()) < 10)
            return true;
        for (int i = 0; i < 10; i++)
        {
            if (int(token.lexeme.at(i)) > int(upperBound.at(i)))
            {
                return false;
            }
            else if (int(token.lexeme.at(i)) < int(upperBound.at(i)))
            {
                return true;
            }
        }
        // }
        // catch (exception e)
        // {
        //   throw runtime_error("failed");
        // }
    }
    return true;
}

string assignId(string lexeme)
{
    if (lexeme == "int")
    {
        return "INT";
    }
    else if (lexeme == "wain")
    {
        return "WAIN";
    }
    else if (lexeme == "if")
    {
        return "IF";
    }
    else if (lexeme == "else")
    {
        return "ELSE";
    }
    else if (lexeme == "while")
    {
        return "WHILE";
    }
    else if (lexeme == "println")
    {
        return "PRINTLN";
    }
    else if (lexeme == "return")
    {
        return "RETURN";
    }
    else if (lexeme == "new")
    {
        return "NEW";
    }
    else if (lexeme == "delete")
    {
        return "DELETE";
    }
    else if (lexeme == "NULL")
    {
        return "NULL";
    }
    else
    {
        return "ID";
    }
}

deque<Token> smm(dfa *rules)
{
    deque<Token> tokens;
    string line;
    string state = "start";
    string tok = "";
    while (getline(cin, line))
    {
        while (int(line.length()) > 0)
        {
            char fc = line.at(0);
            if (rules->gns(state, fc) != "ns not found")
            {
                tok += fc;
                line = line.substr(1);
                state = rules->gns(state, fc);
            }
            else
            {
                if (rules->accept[state])
                {
                    if (!validateToken(Token(state, tok)))
                    {
                        delete rules;
                        throw runtime_error("Invalid Value");
                    }
                    if (state[0] != '?')
                    {
                        if (state == "ID")
                            state = assignId(tok);
                        if (state == "ZERO")
                            state = "NUM";
                        tokens.push_back(Token(state, tok));
                    }
                    state = "start";
                    tok = "";
                }
                else
                {
                    delete rules;
                    cout << state;
                    throw runtime_error("Invalid state");
                }
            }
        }
        if (rules->accept[state])
        {
            if (!validateToken(Token(state, tok)))
            {
                delete rules;
                throw runtime_error("Invalid Value");
            }
            if (state[0] != '?')
            {
                if (state == "ID")
                    state = assignId(tok);
                if (state == "ZERO")
                    state = "NUM";
                tokens.push_back(Token(state, tok));
            }
            state = "start";
            tok = "";
        }
        else if (state != "start")
        {
            delete rules;
            throw runtime_error("Invalid state");
        }
    }
    delete rules;

    return tokens;
}

dfa *erectDFA(istream &in)
{
    dfa *dfaPtr = new dfa;
    std::string s;
    // Skip blank lines at the start of the file
    while (true)
    {
        if (!(std::getline(in, s)))
        {
            throw std::runtime_error("Expected " + STATES + ", but found end of input.");
        }
        s = squish(s);
        if (s == STATES)
        {
            break;
        }
        if (!s.empty())
        {
            throw std::runtime_error("Expected " + STATES + ", but found: " + s);
        }
    }
    // Print states
    while (true)
    {
        if (!(in >> s))
        {
            throw std::runtime_error("Unexpected end of input while reading state set: " + TRANSITIONS + "not found.");
        }
        if (s == TRANSITIONS)
        {
            break;
        }
        // Process an individual state
        bool accepting = false;
        if (s.back() == '!' && int(s.length()) > 1)
        {
            accepting = true;
            s.pop_back();
        }
        dfaPtr->states.push_back(s);
        //   cout<<s<<endl;
        dfaPtr->accept[s] = accepting;
        //   cout<<s + " " + (accepting ? "true" : "false") << endl;
    }
    // Print transitions
    std::getline(in, s); // Skip .TRANSITIONS header
    while (true)
    {
        if (!(std::getline(in, s)))
        {
            // We reached the end of the file
            break;
        }
        s = squish(s);
        if (s == INPUT)
        {
            break;
        }
        // Split the line into parts
        std::string lineStr = s;
        std::stringstream line(lineStr);
        std::vector<std::string> lineVec;
        while (line >> s)
        {
            lineVec.push_back(s);
        }
        if (lineVec.empty())
        {
            // Skip blank lines
            continue;
        }
        if (lineVec.size() < 3)
        {
            throw std::runtime_error("Incomplete transition line: " + lineStr);
        }
        // Extract state information from the line
        std::string fromState = lineVec.front();
        std::string toState = lineVec.back();
        // Extract character and range information from the line
        std::vector<char> charVec;
        for (int i = 1; i < int(lineVec.size()) - 1; ++i)
        {
            std::string charOrRange = escape(lineVec[i]);
            if (isChar(charOrRange))
            {
                char c = charOrRange[0];
                if (c < 0 || c > 127)
                {
                    throw std::runtime_error("Invalid (non-ASCII) character in transition line: " + lineStr + "\n" + "Character " + unescape(std::string(1, c)) + " is outside ASCII range");
                }
                dfaPtr->ns[fromState][c] = toState;
                //   cout<< fromState + " " + c + " " + dfa.ns[fromState][c]<<endl;
            }
            else if (isRange(charOrRange))
            {
                for (char c = charOrRange[0]; charOrRange[0] <= c && c <= charOrRange[2]; ++c)
                {
                    dfaPtr->ns[fromState][c] = toState;
                    // cout<< fromState + " " + c + " " + dfa.ns[fromState][c]<<endl;
                }
            }
            else
            {
                throw std::runtime_error("Expected character or range, but found " + charOrRange + " in transition line: " + lineStr);
            }
        }
    }
    // We ignore .INPUT sections, so we're done

    return dfaPtr;
}

//// Helper functions

bool isChar(std::string s)
{
    return int(s.length()) == 1;
}

bool isRange(std::string s)
{
    return int(s.length()) == 3 && s[1] == '-';
}

std::string squish(std::string s)
{
    std::stringstream ss(s);
    std::string token;
    std::string result;
    std::string space = "";
    while (ss >> token)
    {
        result += space;
        result += token;
        space = " ";
    }
    return result;
}

int hexToNum(char c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    else if ('a' <= c && c <= 'f')
    {
        return 10 + (c - 'a');
    }
    else if ('A' <= c && c <= 'F')
    {
        return 10 + (c - 'A');
    }
    // This should never happen....
    throw std::runtime_error("Invalid hex digit!");
}

char numToHex(int d)
{
    return (d < 10 ? d + '0' : d - 10 + 'A');
}

std::string escape(std::string s)
{
    std::string p;
    for (int i = 0; i < int(s.length()); ++i)
    {
        if (s[i] == '\\' && i + 1 < int(s.length()))
        {
            char c = s[i + 1];
            i = i + 1;
            if (c == 's')
            {
                p += ' ';
            }
            else if (c == 'n')
            {
                p += '\n';
            }
            else if (c == 'r')
            {
                p += '\r';
            }
            else if (c == 't')
            {
                p += '\t';
            }
            else if (c == 'x')
            {
                if (i + 2 < int(s.length()) && isxdigit(s[i + 1]) && isxdigit(s[i + 2]))
                {
                    if (hexToNum(s[i + 1]) > 8)
                    {
                        throw std::runtime_error(
                            "Invalid escape sequence \\x" + std::string(1, s[i + 1]) + std::string(1, s[i + 2]) + ": not in ASCII range (0x00 to 0x7F)");
                    }
                    char code = hexToNum(s[i + 1]) * 16 + hexToNum(s[i + 2]);
                    p += code;
                    i = i + 2;
                }
                else
                {
                    p += c;
                }
            }
            else if (isgraph(c))
            {
                p += c;
            }
            else
            {
                p += s[i];
            }
        }
        else
        {
            p += s[i];
        }
    }
    return p;
}

std::string unescape(std::string s)
{
    std::string p;
    for (int i = 0; i < int(s.length()); ++i)
    {
        char c = s[i];
        if (c == ' ')
        {
            p += "\\s";
        }
        else if (c == '\n')
        {
            p += "\\n";
        }
        else if (c == '\r')
        {
            p += "\\r";
        }
        else if (c == '\t')
        {
            p += "\\t";
        }
        else if (!isgraph(c))
        {
            std::string hex = "\\x";
            p += hex + numToHex((unsigned char)c / 16) + numToHex((unsigned char)c % 16);
        }
        else
        {
            p += c;
        }
    }
    return p;
}

struct rule
{
    string lhs;
    vector<string> rhs;
};

struct slr
{
    map<pair<int, string>, int> transitions;
    map<pair<int, string>, int> reductions;
};

slr *tr;

struct Node
{
    // data stored at the node
    rule data;
    Token token;
    bool terminal = false;
    string type;
    // vector of pointers to child subtrees; could also use smart pointers
    std::vector<Node *> children;
    // constructor for a leaf node
    Node(rule data) : data(data) {}
    Node(Token token) : token(token) {}
    Node() : data(), token(), children() {}

    Node *getChild(const std::string &criteria, int n = 1)
    {
        int count = 0;
        for (int i = 0; i < data.rhs.size(); i++)
        {
            if (data.rhs[i] == criteria)
            {
                count++;
            }
            if (count == n)
            {
                return children[i];
            }
        }
        throw runtime_error("child not found");
    }

    // explicit destructor; could also use smart pointers
    ~Node()
    {
        for (auto &c : children)
        {
            delete c;
        }
    }
    void print(std::string prefix, std::ostream &out = std::cout)
    {
        // prints a representation of the tree, with each line indented by the given prefix
    }
};

vector<rule> *activateCFG()
{
    vector<rule> *cfg = new vector<rule>;
    vector<string> splittedRules = splitString(WLP4_CFG);
    splittedRules.erase(splittedRules.begin());
    for (string line : splittedRules)
    {
        vector<string> tokens = split(line);
        rule r;
        r.lhs = tokens.at(0);
        tokens.erase(tokens.begin());
        if (!(tokens.size() == 1 and tokens.at(0) == ".EMPTY"))
        {
            r.rhs = tokens;
        }
        cfg->push_back(r);
    }
    return cfg;
}

slr *activateSLR()
{
    slr *s = new slr();
    vector<string> splittedTransitions = splitString(WLP4_TRANSITIONS);
    vector<string> splittedReductions = splitString(WLP4_REDUCTIONS);
    splittedTransitions.erase(splittedTransitions.begin());
    for (string line : splittedTransitions)
    {
        vector<string> tokens = split(line);
        if (tokens.size() != 3)
        {
            throw runtime_error("epic fail");
        }
        s->transitions[make_pair(stoi(tokens.at(0)), tokens.at(1))] = stoi(tokens.at(2));
    }

    splittedReductions.erase(splittedReductions.begin());
    for (string line : splittedReductions)
    {
        vector<string> tokens = split(line);
        if (tokens.size() != 3)
        {
            throw runtime_error("epic fail 2");
        }
        s->reductions[make_pair(stoi(tokens.at(0)), tokens.at(2))] = stoi(tokens.at(1));
    }
    return s;
}

void reduceTrees(rule rule, vector<Node *> *treeStack)
{
    Node *node = new Node(rule);
    int len = rule.rhs.size();
    for (int i = len; i > 0; --i)
    {
        node->children.push_back(treeStack->at(treeStack->size() - i));
    }
    for (int i = len; i > 0; --i)
    {
        treeStack->pop_back();
    }
    treeStack->push_back(node);
}

void reduceStates(rule rule, vector<int> *stateStack)
{
    int len = rule.rhs.size();
    for (int i = 0; i < len; i++)
    {
        stateStack->pop_back();
    }
    auto it = tr->transitions.find(make_pair(stateStack->at(stateStack->size() - 1), rule.lhs));
    if (tr->transitions.end() == it)
    {
        throw runtime_error("epic epic fail");
    }
    else
    {
        stateStack->push_back(tr->transitions[make_pair(stateStack->at(stateStack->size() - 1), rule.lhs)]);
    }
}

void shift(deque<Token> &token, vector<int> *stateStack, vector<Node *> *treeStack, const vector<rule> *cfg)
{
    if (token.empty())
        throw runtime_error("you're in trouble");
    Node *node = new Node(token.front());
    node->terminal = true;
    treeStack->push_back(node);
    int top = stateStack->back();
    auto it = tr->transitions.find(make_pair(top, token.front().kind));
    if (it != tr->transitions.end())
    {
        stateStack->push_back(tr->transitions[make_pair(top, token.front().kind)]);
    }
    else
    {
        throw runtime_error("epic fail 3");
    }
    token.pop_front();
}

void parse(vector<int> *stateStack, deque<Token> &tokens, const vector<rule> *cfg, vector<Node *> *treeStack)
{
    while (!tokens.empty())
    {
        int curState = stateStack->back();
        string tokenKind = tokens.front().kind;
        auto it = tr->reductions.find({curState, tokenKind});

        if (it != tr->reductions.end())
        {
            const rule &r = cfg->at(it->second);
            reduceTrees(r, treeStack);
            reduceStates(r, stateStack);
        }
        else
        {
            shift(tokens, stateStack, treeStack, cfg);
        }
    }

    reduceTrees(cfg->at(0), treeStack);
}

void printTree(Node *tree)
{
    if (tree->terminal)
    {
        cout << tree->token.kind << " " << tree->token.lexeme << endl;
    }
    else
    {
        rule r = tree->data;
        if (r.rhs.size() == 0)
        {
            cout << r.lhs << " .EMPTY";
        }
        else
        {
            cout << r.lhs;
            for (string s : r.rhs)
            {
                cout << " " << s;
            }
        }
        cout << endl;
        for (Node *node : tree->children)
        {
            printTree(node);
        }
    }
}
