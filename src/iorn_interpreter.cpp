#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <sstream>
#include <map>
#include <cmath>
#include <algorithm>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

class IornInterpreter {
private:
    std::map<std::string, std::string> variables;
    
    void setRedColor() {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        }
#else
        std::cerr << "\033[31m";
#endif
    }
    
    void resetColor() {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
#else
        std::cerr << "\033[0m";
#endif
    }
    
    int getLineNumber(const std::string& code, size_t pos) {
        return static_cast<int>(std::count(code.begin(), code.begin() + pos, '\n') + 1);
    }
    
    int getCharNumber(const std::string& code, size_t pos) {
        size_t lastNewline = code.rfind('\n', pos);
        return static_cast<int>((lastNewline == std::string::npos) ? pos + 1 : pos - lastNewline);
    }
    
    bool checkSyntax(const std::string& code) {
        std::istringstream stream(code);
        std::string line;
        int lineNum = 0;
        
        while (std::getline(stream, line)) {
            lineNum++;
            
            if (line.empty() || line.find("@rem") != std::string::npos || 
                line.find("##") != std::string::npos || line.find("\"\"\"") != std::string::npos ||
                line.find("@remLine") != std::string::npos) {
                continue;
            }
            
            std::regex functionCall("\\w+\\s*\\(");
            std::regex nullDeclaration("new variable\\s+\\w+\\s+NULL;");
            bool isIfRelated = (line.find("if (") != std::string::npos || line.find("else") != std::string::npos || line.find("endif") != std::string::npos);
            bool isIgnore = (line.find("ignore") != std::string::npos);
            
            if ((std::regex_search(line, functionCall) || line.find("import ") != std::string::npos || 
                 line.find("new variable") != std::string::npos || line.find("rename variable") != std::string::npos) && 
                !std::regex_match(line, nullDeclaration) && !isIfRelated && !isIgnore) {
                
                std::string trimmed = line;
                trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
                
                if (!trimmed.empty() && trimmed.back() != ';' && trimmed.back() != ':') {
                    setRedColor();
                    std::cerr << "There is no ending on " << lineNum 
                              << " page " << trimmed.length() + 1 << " character!" << std::endl;
                    resetColor();
                    return false;
                }
            }
        }
        return true;
    }
    
    std::string removeComments(const std::string& code) {
        std::string result = code;
        
        size_t pos = 0;
        while ((pos = result.find("@remLine(", pos)) != std::string::npos) {
            size_t start = pos;
            size_t end = pos + 9;
            int brackets = 1;
            
            while (end < result.length() && brackets > 0) {
                if (result[end] == '(') brackets++;
                else if (result[end] == ')') brackets--;
                end++;
            }
            
            if (brackets == 0) {
                result.erase(start, end - start);
                pos = start;
            } else {
                pos++;
            }
        }
        
        result = std::regex_replace(result, std::regex("@rem.*"), "");
        result = std::regex_replace(result, std::regex("##.*"), "");
        result = std::regex_replace(result, std::regex("\"\"\"[\\s\\S]*?\"\"\""), "");
        
        return result;
    }
    
    bool validateVariable(const std::string& varName, const std::string& varType, const std::string& varValue, int lineNum) {
        if (varName.empty() || !std::isalpha(varName[0])) {
            setRedColor();
            std::cerr << "Variable name '" << varName << "' is invalid on line " << lineNum << ". Must start with a letter." << std::endl;
            resetColor();
            return false;
        }
        
        if (variables.find(varName) != variables.end()) {
            setRedColor();
            std::cerr << "Variable '" << varName << "' is already declared on line " << lineNum << "." << std::endl;
            resetColor();
            return false;
        }
        
        std::string trimmedValue = varValue;
        trimmedValue.erase(0, trimmedValue.find_first_not_of(" \t"));
        trimmedValue.erase(trimmedValue.find_last_not_of(" \t") + 1);
        
        if (varType == "string") {
            if (trimmedValue.length() < 2 || trimmedValue.front() != '"' || trimmedValue.back() != '"') {
                setRedColor();
                std::cerr << "String value must be enclosed in quotes on line " << lineNum << "." << std::endl;
                resetColor();
                return false;
            }
        } else if (varType == "numeric") {
            if (!std::regex_match(trimmedValue, std::regex("^-?\\d+$"))) {
                setRedColor();
                std::cerr << "Invalid numeric value '" << trimmedValue << "' on line " << lineNum << "." << std::endl;
                resetColor();
                return false;
            }
        } else if (varType == "floating") {
            if (!std::regex_match(trimmedValue, std::regex("^-?\\d+\\.\\d+$"))) {
                setRedColor();
                std::cerr << "Invalid floating value '" << trimmedValue << "' on line " << lineNum << ". Must contain decimal point." << std::endl;
                resetColor();
                return false;
            }
        } else if (varType == "boolean") {
            if (trimmedValue != "TRUE" && trimmedValue != "FALSE") {
                setRedColor();
                std::cerr << "Boolean value must be TRUE or FALSE on line " << lineNum << "." << std::endl;
                resetColor();
                return false;
            }
        }
        return true;
    }
    
    std::string replaceVariables(const std::string& expr) {
        std::string result = expr;
        for (const auto& var : variables) {
            size_t pos = 0;
            while ((pos = result.find(var.first, pos)) != std::string::npos) {
                bool isFullWord = (pos == 0 || !std::isalnum(result[pos-1])) && 
                                 (pos + var.first.length() == result.length() || !std::isalnum(result[pos + var.first.length()]));
                if (isFullWord) {
                    result.replace(pos, var.first.length(), var.second);
                    pos += var.second.length();
                } else {
                    pos++;
                }
            }
        }
        return result;
    }
    
    double evaluateExpression(const std::string& expr);
    double evaluateWithPrecedence(const std::string& expr);
    size_t findNumberStart(const std::string& expr, size_t pos);
    size_t findNumberEnd(const std::string& expr, size_t pos);
    
    bool evaluateCondition(const std::string& condition) {
        std::string cleanCond = condition;
        cleanCond.erase(std::remove(cleanCond.begin(), cleanCond.end(), ' '), cleanCond.end());
        
        std::vector<std::string> operators = {"==", "!=", ">=", "<=", ">", "<"};
        
        for (const std::string& op : operators) {
            size_t pos = cleanCond.find(op);
            if (pos != std::string::npos) {
                std::string left = cleanCond.substr(0, pos);
                std::string right = cleanCond.substr(pos + op.length());
                
                bool leftIsStringLiteral = (left.length() >= 2 && left.front() == '"' && left.back() == '"');
                bool rightIsStringLiteral = (right.length() >= 2 && right.front() == '"' && right.back() == '"');
                
                std::string leftOriginal = left;
                std::string rightOriginal = right;
                left = replaceVariables(left);
                right = replaceVariables(right);
                
                bool leftWasVariable = (leftOriginal != left && !leftIsStringLiteral);
                bool rightWasVariable = (rightOriginal != right && !rightIsStringLiteral);
                
                if (leftIsStringLiteral || rightIsStringLiteral || leftWasVariable || rightWasVariable) {
                    std::string leftStr = leftIsStringLiteral ? left.substr(1, left.length() - 2) : left;
                    std::string rightStr = rightIsStringLiteral ? right.substr(1, right.length() - 2) : right;
                    
                    // Trim whitespace for comparison
                    leftStr.erase(0, leftStr.find_first_not_of(" \t\r\n"));
                    leftStr.erase(leftStr.find_last_not_of(" \t\r\n") + 1);
                    rightStr.erase(0, rightStr.find_first_not_of(" \t\r\n"));
                    rightStr.erase(rightStr.find_last_not_of(" \t\r\n") + 1);
                    
                    // Try numeric comparison first if both look like numbers
                    bool leftIsNumeric = std::regex_match(leftStr, std::regex("^-?\\d+(\\.\\d+)?$"));
                    bool rightIsNumeric = std::regex_match(rightStr, std::regex("^-?\\d+(\\.\\d+)?$"));
                    
                    if (leftIsNumeric && rightIsNumeric) {
                        try {
                            double leftVal = std::stod(leftStr);
                            double rightVal = std::stod(rightStr);
                            
                            if (op == "==") return leftVal == rightVal;
                            else if (op == "!=") return leftVal != rightVal;
                            else if (op == ">=") return leftVal >= rightVal;
                            else if (op == "<=") return leftVal <= rightVal;
                            else if (op == ">") return leftVal > rightVal;
                            else if (op == "<") return leftVal < rightVal;
                        } catch (...) {
                            // Fall back to string comparison
                        }
                    }
                    
                    // String comparison
                    if (op == "==") return leftStr == rightStr;
                    else if (op == "!=") return leftStr != rightStr;
                    else if (op == ">=") return leftStr >= rightStr;
                    else if (op == "<=") return leftStr <= rightStr;
                    else if (op == ">") return leftStr > rightStr;
                    else if (op == "<") return leftStr < rightStr;
                } else {
                    try {
                        double leftVal = evaluateExpression(left);
                        double rightVal = evaluateExpression(right);
                        
                        if (op == "==") return leftVal == rightVal;
                        else if (op == "!=") return leftVal != rightVal;
                        else if (op == ">=") return leftVal >= rightVal;
                        else if (op == "<=") return leftVal <= rightVal;
                        else if (op == ">") return leftVal > rightVal;
                        else if (op == "<") return leftVal < rightVal;
                    } catch (...) {
                        // Trim whitespace for string comparison fallback
                        std::string leftTrimmed = left;
                        std::string rightTrimmed = right;
                        leftTrimmed.erase(0, leftTrimmed.find_first_not_of(" \t\r\n"));
                        leftTrimmed.erase(leftTrimmed.find_last_not_of(" \t\r\n") + 1);
                        rightTrimmed.erase(0, rightTrimmed.find_first_not_of(" \t\r\n"));
                        rightTrimmed.erase(rightTrimmed.find_last_not_of(" \t\r\n") + 1);
                        
                        if (op == "==") return leftTrimmed == rightTrimmed;
                        else if (op == "!=") return leftTrimmed != rightTrimmed;
                        else if (op == ">=") return leftTrimmed >= rightTrimmed;
                        else if (op == "<=") return leftTrimmed <= rightTrimmed;
                        else if (op == ">") return leftTrimmed > rightTrimmed;
                        else if (op == "<") return leftTrimmed < rightTrimmed;
                    }
                }
            }
        }
        
        std::string varName = cleanCond;
        if (variables.find(varName) != variables.end()) {
            return variables[varName] == "TRUE";
        }
        
        return false;
    }
    
    bool isInComment(const std::string& originalCode, size_t pos) {
        size_t searchPos = 0;
        while (true) {
            size_t start = originalCode.find("\"\"\"", searchPos);
            if (start == std::string::npos || start > pos) break;
            
            size_t end = originalCode.find("\"\"\"", start + 3);
            if (end != std::string::npos && pos >= start && pos <= end) {
                return true;
            }
            searchPos = (end != std::string::npos) ? end + 3 : start + 3;
        }
        
        searchPos = 0;
        while (true) {
            size_t start = originalCode.find("@remLine(", searchPos);
            if (start == std::string::npos || start > pos) break;
            
            size_t end = start + 9;
            int brackets = 1;
            while (end < originalCode.length() && brackets > 0) {
                if (originalCode[end] == '(') brackets++;
                else if (originalCode[end] == ')') brackets--;
                end++;
            }
            
            if (brackets == 0 && pos >= start && pos <= end) {
                return true;
            }
            searchPos = start + 1;
        }
        
        return false;
    }
    
    void executeBlock(const std::string& block);
    void processIfStatement(const std::string& cleanCode);
    std::string executeInput(const std::string& prompt = "", const std::string& expectedType = "string");
    void processVariables(const std::string& cleanCode);
    std::string interpolateVariables(const std::string& text);
    void executePrint(const std::string& text);
    void executeSequentially(const std::string& cleanCode);
    void processVariableLine(const std::string& line, int lineNum);
    
public:
    IornInterpreter() {
#ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
#endif
    }
    
    void interpret(const std::string& code) {
        std::string cleanCode = removeComments(code);
        
        if (!checkSyntax(cleanCode)) {
            return;
        }
        
        std::regex importDeclaration("import\\s+([\\w\\.]+\\*?);?");
        bool printDeclared = false;
        bool inputDeclared = false;
        
        std::sregex_iterator iter(cleanCode.begin(), cleanCode.end(), importDeclaration);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            std::string importPath = (*iter)[1].str();
            if (importPath == "terminal.Print" || 
                importPath == "terminal.*" || 
                (importPath.find("terminal.") == 0 && importPath.back() == '*')) {
                printDeclared = true;
            }
            if (importPath == "terminal.input" || 
                importPath == "terminal.*" || 
                (importPath.find("terminal.") == 0 && importPath.back() == '*')) {
                inputDeclared = true;
            }
        }
        
        if (!printDeclared) {
            setRedColor();
            std::cerr << "I do not know what Print is! Write its import" << std::endl;
            resetColor();
            return;
        }
        
        std::regex inputCallAny("input\\(");
        if (std::regex_search(cleanCode, inputCallAny) && !inputDeclared) {
            setRedColor();
            std::cerr << "I do not know what input is! Write its import" << std::endl;
            resetColor();
            return;
        }
        
        // Execute code line by line
        executeSequentially(cleanCode);
        
        // Validate function calls (exclude numbers)
        std::regex anyFunctionCall("([a-zA-Z_]\\w*)\\(([^)]*)\\);");
        std::string::const_iterator start2 = cleanCode.cbegin();
        std::smatch match2;
        
        while (std::regex_search(start2, cleanCode.cend(), match2, anyFunctionCall)) {
            std::string funcName = match2[1].str();
            if (funcName != "Print" && funcName != "input") {
                setRedColor();
                std::cerr << "Unknown function '" << funcName << "()'. Function is not defined or imported." << std::endl;
                resetColor();
                return;
            }
            start2 = match2.suffix().first;
        }
    }
    
    void interpretFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            setRedColor();
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            resetColor();
            return;
        }
        
        std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        interpret(code);
    }
};

// Implementation of forward-declared methods
double IornInterpreter::evaluateExpression(const std::string& expr) {
    std::string cleanExpr = expr;
    cleanExpr.erase(std::remove(cleanExpr.begin(), cleanExpr.end(), ' '), cleanExpr.end());
    
    size_t pos = 0;
    while ((pos = cleanExpr.find("**", pos)) != std::string::npos) {
        cleanExpr.replace(pos, 2, "^");
        pos += 1;
    }
    
    return evaluateWithPrecedence(cleanExpr);
}

double IornInterpreter::evaluateWithPrecedence(const std::string& expr) {
    if (expr.empty()) return 0;
    
    std::string workExpr = expr;
    while (workExpr.find('(') != std::string::npos) {
        size_t start = workExpr.find_last_of('(');
        size_t end = workExpr.find(')', start);
        if (end == std::string::npos) break;
        
        std::string subExpr = workExpr.substr(start + 1, end - start - 1);
        double result = evaluateWithPrecedence(subExpr);
        
        workExpr = workExpr.substr(0, start) + std::to_string(result) + workExpr.substr(end + 1);
    }
    
    if (workExpr.find_first_of("+-*/%^") == std::string::npos) {
        return std::stod(workExpr);
    }
    
    while (workExpr.find('^') != std::string::npos) {
        size_t opPos = workExpr.find('^');
        size_t leftStart = findNumberStart(workExpr, opPos - 1);
        size_t rightEnd = findNumberEnd(workExpr, opPos + 1);
        
        double left = std::stod(workExpr.substr(leftStart, opPos - leftStart));
        double right = std::stod(workExpr.substr(opPos + 1, rightEnd - opPos - 1));
        double result = std::pow(left, right);
        
        workExpr = workExpr.substr(0, leftStart) + std::to_string(result) + workExpr.substr(rightEnd);
    }
    
    size_t pos = 0;
    while ((pos = workExpr.find_first_of("*/%", pos)) != std::string::npos) {
        char op = workExpr[pos];
        size_t leftStart = findNumberStart(workExpr, pos - 1);
        size_t rightEnd = findNumberEnd(workExpr, pos + 1);
        
        double left = std::stod(workExpr.substr(leftStart, pos - leftStart));
        double right = std::stod(workExpr.substr(pos + 1, rightEnd - pos - 1));
        double result = 0;
        
        if (op == '*') result = left * right;
        else if (op == '/') result = (right != 0) ? left / right : 0;
        else if (op == '%') result = std::fmod(left, right);
        
        workExpr = workExpr.substr(0, leftStart) + std::to_string(result) + workExpr.substr(rightEnd);
        pos = leftStart;
    }
    
    pos = 0;
    while ((pos = workExpr.find_first_of("+-", pos)) != std::string::npos) {
        if (pos == 0) { pos++; continue; }
        
        char op = workExpr[pos];
        size_t leftStart = findNumberStart(workExpr, pos - 1);
        size_t rightEnd = findNumberEnd(workExpr, pos + 1);
        
        double left = std::stod(workExpr.substr(leftStart, pos - leftStart));
        double right = std::stod(workExpr.substr(pos + 1, rightEnd - pos - 1));
        double result = (op == '+') ? left + right : left - right;
        
        workExpr = workExpr.substr(0, leftStart) + std::to_string(result) + workExpr.substr(rightEnd);
        pos = leftStart;
    }
    
    return std::stod(workExpr);
}

size_t IornInterpreter::findNumberStart(const std::string& expr, size_t pos) {
    while (pos > 0 && (std::isdigit(expr[pos]) || expr[pos] == '.' || expr[pos] == '-')) {
        pos--;
    }
    if (pos > 0 && !std::isdigit(expr[pos]) && expr[pos] != '.' && expr[pos] != '-') pos++;
    return pos;
}

size_t IornInterpreter::findNumberEnd(const std::string& expr, size_t pos) {
    while (pos < expr.length() && (std::isdigit(expr[pos]) || expr[pos] == '.' || (pos == 0 && expr[pos] == '-'))) {
        pos++;
    }
    return pos;
}

std::string IornInterpreter::executeInput(const std::string& prompt, const std::string& expectedType) {
    std::string input;
    while (true) {
        if (!prompt.empty()) {
            std::cout << prompt << std::flush;
        }
        
#ifdef _WIN32
        // Для Windows читаем через широкие символы
        wchar_t wbuffer[1024];
        DWORD charsRead;
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        if (ReadConsoleW(hStdin, wbuffer, sizeof(wbuffer)/sizeof(wchar_t) - 1, &charsRead, NULL)) {
            wbuffer[charsRead] = L'\0';
            // Убираем \r\n в конце
            std::wstring winput(wbuffer);
            while (!winput.empty() && (winput.back() == L'\r' || winput.back() == L'\n')) {
                winput.pop_back();
            }
            // Преобразуем в UTF-8
            if (!winput.empty()) {
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, winput.c_str(), (int)winput.size(), NULL, 0, NULL, NULL);
                input.resize(size_needed);
                WideCharToMultiByte(CP_UTF8, 0, winput.c_str(), (int)winput.size(), &input[0], size_needed, NULL, NULL);
            } else {
                input = "";
            }
        } else {
            input = "";
        }
#else
        if (!std::getline(std::cin, input)) {
            input = "";
            break;
        }
#endif
        
        // Убираем возможные символы \r в конце строки
        if (!input.empty() && input.back() == '\r') {
            input.pop_back();
        }
        
        // Для строкового типа принимаем любой ввод
        if (expectedType == "string") {
            break;
        }
        
        if (expectedType == "numeric") {
            if (std::regex_match(input, std::regex("^-?\\d+$"))) {
                break;
            } else {
                setRedColor();
                std::cerr << "Error: Please enter a valid integer number." << std::endl;
                resetColor();
            }
        } else if (expectedType == "floating") {
            if (std::regex_match(input, std::regex("^-?\\d+\\.\\d+$"))) {
                break;
            } else {
                setRedColor();
                std::cerr << "Error: Please enter a valid floating point number (e.g., 1.5)." << std::endl;
                resetColor();
            }
        } else if (expectedType == "boolean") {
            if (input == "TRUE" || input == "FALSE" || input == "true" || input == "false") {
                if (input == "true") input = "TRUE";
                if (input == "false") input = "FALSE";
                break;
            } else {
                setRedColor();
                std::cerr << "Error: Please enter TRUE/FALSE or true/false." << std::endl;
                resetColor();
            }
        } else {
            // Для всех остальных типов тоже принимаем ввод
            break;
        }
    }
    return input;
}

void IornInterpreter::processVariables(const std::string& cleanCode) {
    std::istringstream stream(cleanCode);
    std::string line;
    int lineNum = 0;
    
    while (std::getline(stream, line)) {
        lineNum++;
        
        if (line.find("new variable") != std::string::npos) {
            std::regex nullDeclaration("new variable\\s+(\\w+)\\s+NULL;");
            std::smatch nullMatch;
            if (std::regex_search(line, nullMatch, nullDeclaration)) {
                std::string varName = nullMatch[1].str();
                variables[varName] = "null";
                continue;
            }
            
            std::regex varDeclaration("new variable\\s+(\\w+)\\s+(string|numeric|floating|boolean)\\s*=\\s*([^;]+);");
            std::smatch match;
            
            if (std::regex_search(line, match, varDeclaration)) {
                std::string varName = match[1].str();
                std::string varType = match[2].str();
                std::string varValue = match[3].str();
                
                if (varValue.find_first_of("+-*/%^") != std::string::npos || varValue.find("**") != std::string::npos) {
                    if (varType == "numeric" || varType == "floating") {
                        try {
                            double result = evaluateExpression(varValue);
                            if (varType == "numeric") {
                                varValue = std::to_string(static_cast<int>(result));
                            } else {
                                varValue = std::to_string(result);
                            }
                        } catch (...) {
                            setRedColor();
                            std::cerr << "Invalid mathematical expression on line " << lineNum << "." << std::endl;
                            resetColor();
                            continue;
                        }
                    }
                }
                
                bool isInputCall = varValue.find("input(") != std::string::npos;
                if (isInputCall) {
                    std::regex inputCall("input\\((\"[^\"]*\"|[^)]*)\\)");
                    std::smatch inputMatch;
                    if (std::regex_search(varValue, inputMatch, inputCall)) {
                        std::string prompt = inputMatch[1].str();
                        if (!prompt.empty() && prompt.front() == '"' && prompt.back() == '"') {
                            prompt = prompt.substr(1, prompt.length() - 2);
                        }
                        std::string inputValue = executeInput(prompt, varType);
                        variables[varName] = inputValue;
                    }
                } else {
                    if (!validateVariable(varName, varType, varValue, lineNum)) {
                        continue;
                    }
                    
                    std::string trimmedValue = varValue;
                    trimmedValue.erase(0, trimmedValue.find_first_not_of(" \t"));
                    trimmedValue.erase(trimmedValue.find_last_not_of(" \t") + 1);
                    
                    if (varType == "string" && trimmedValue.front() == '"' && trimmedValue.back() == '"') {
                        trimmedValue = trimmedValue.substr(1, trimmedValue.length() - 2);
                    }
                    
                    variables[varName] = trimmedValue;
                }
            }
        }
        
        if (line.find("rename variable") != std::string::npos) {
            std::regex renameDeclaration("rename variable\\s+(\\w+)\\s+(string|numeric|floating|boolean)\\s*=\\s*([^;]+);");
            std::smatch match;
            
            if (std::regex_search(line, match, renameDeclaration)) {
                std::string varName = match[1].str();
                std::string varType = match[2].str();
                std::string varValue = match[3].str();
                
                if (variables.find(varName) == variables.end()) {
                    setRedColor();
                    std::cerr << "Variable '" << varName << "' does not exist for rename on line " << lineNum << "." << std::endl;
                    resetColor();
                    continue;
                }
                
                if (varValue.find("input(") != std::string::npos) {
                    std::regex inputCall("input\\((\"[^\"]*\"|[^)]*)\\)");
                    std::smatch inputMatch;
                    if (std::regex_search(varValue, inputMatch, inputCall)) {
                        std::string prompt = inputMatch[1].str();
                        if (!prompt.empty() && prompt.front() == '"' && prompt.back() == '"') {
                            prompt = prompt.substr(1, prompt.length() - 2);
                        }
                        std::string inputValue = executeInput(prompt, varType);
                        variables[varName] = inputValue;
                    }
                } else {
                    std::string trimmedValue = varValue;
                    trimmedValue.erase(0, trimmedValue.find_first_not_of(" \t"));
                    trimmedValue.erase(trimmedValue.find_last_not_of(" \t") + 1);
                    
                    if (varType == "string" && trimmedValue.front() == '"' && trimmedValue.back() == '"') {
                        trimmedValue = trimmedValue.substr(1, trimmedValue.length() - 2);
                    }
                    
                    variables[varName] = trimmedValue;
                }
            }
        }
    }
}

std::string IornInterpreter::interpolateVariables(const std::string& text) {
    std::string result = text;
    std::regex varPattern("\\$\\[(\\w+)\\]");
    std::smatch match;
    
    while (std::regex_search(result, match, varPattern)) {
        std::string varName = match[1].str();
        if (variables.find(varName) != variables.end()) {
            result.replace(match.position(), match.length(), variables[varName]);
        }
    }
    
    return result;
}

void IornInterpreter::executePrint(const std::string& text) {
    std::string output = text.substr(1, text.length() - 2);
    
    if (text.length() > 2 && text.substr(0, 2) == "f\"") {
        output = text.substr(2, text.length() - 3);
        output = interpolateVariables(output);
    }
    
    std::cout << output << std::endl;
}

void IornInterpreter::executeBlock(const std::string& block) {
    std::string trimmedBlock = block;
    trimmedBlock.erase(0, trimmedBlock.find_first_not_of(" \t\n\r"));
    trimmedBlock.erase(trimmedBlock.find_last_not_of(" \t\n\r") + 1);
    
    if (trimmedBlock == "ignore" || trimmedBlock.empty() || trimmedBlock == "ignore;") {
        return;
    }
    
    std::regex printCall("Print\\((f?\"[^\"]*\")\\);");
    std::sregex_iterator iter(block.begin(), block.end(), printCall);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::string text = (*iter)[1].str();
        executePrint(text);
    }
}

void IornInterpreter::executeSequentially(const std::string& cleanCode) {
    std::istringstream stream(cleanCode);
    std::string line;
    std::vector<std::string> lines;
    
    // Collect all lines
    while (std::getline(stream, line)) {
        if (!line.empty() && line.find("import ") == std::string::npos) {
            lines.push_back(line);
        }
    }
    
    // Execute lines sequentially
    for (size_t i = 0; i < lines.size(); i++) {
        std::string currentLine = lines[i];
        
        // Skip empty lines and comments
        if (currentLine.empty() || currentLine.find("@rem") != std::string::npos || 
            currentLine.find("##") != std::string::npos) {
            continue;
        }
        
        // Handle variable declarations
        if (currentLine.find("new variable") != std::string::npos || 
            currentLine.find("rename variable") != std::string::npos) {
            processVariableLine(currentLine, i + 1);
        }
        // Handle Print statements
        else if (currentLine.find("Print(") != std::string::npos) {
            std::regex printCall("Print\\((f?\"[^\"]*\")\\);");
            std::smatch match;
            if (std::regex_search(currentLine, match, printCall)) {
                std::string text = match[1].str();
                executePrint(text);
            }
        }
        // Handle if statements
        else if (currentLine.find("if (") != std::string::npos) {
            // Find the complete if block
            std::string ifBlock = currentLine;
            size_t j = i + 1;
            while (j < lines.size() && lines[j].find("endif;") == std::string::npos) {
                ifBlock += "\n" + lines[j];
                j++;
            }
            if (j < lines.size()) {
                ifBlock += "\n" + lines[j]; // Add endif line
            }
            processIfStatement(ifBlock);
            i = j; // Skip processed lines
        }
    }
}

void IornInterpreter::processVariableLine(const std::string& line, int lineNum) {
    if (line.find("new variable") != std::string::npos) {
        std::regex nullDeclaration("new variable\\s+(\\w+)\\s+NULL;");
        std::smatch nullMatch;
        if (std::regex_search(line, nullMatch, nullDeclaration)) {
            std::string varName = nullMatch[1].str();
            variables[varName] = "null";
            return;
        }
        
        std::regex varDeclaration("new variable\\s+(\\w+)\\s+(string|numeric|floating|boolean)\\s*=\\s*([^;]+);");
        std::smatch match;
        
        if (std::regex_search(line, match, varDeclaration)) {
            std::string varName = match[1].str();
            std::string varType = match[2].str();
            std::string varValue = match[3].str();
            
            bool isInputCall = varValue.find("input(") != std::string::npos;
            if (isInputCall) {
                std::regex inputCall("input\\((\"[^\"]*\"|[^)]*)\\)");
                std::smatch inputMatch;
                if (std::regex_search(varValue, inputMatch, inputCall)) {
                    std::string prompt = inputMatch[1].str();
                    if (!prompt.empty() && prompt.front() == '"' && prompt.back() == '"') {
                        prompt = prompt.substr(1, prompt.length() - 2);
                    }
                    std::string inputValue = executeInput(prompt, varType);
                    variables[varName] = inputValue;
                }
            } else {
                if (!validateVariable(varName, varType, varValue, lineNum)) {
                    return;
                }
                
                std::string trimmedValue = varValue;
                trimmedValue.erase(0, trimmedValue.find_first_not_of(" \t"));
                trimmedValue.erase(trimmedValue.find_last_not_of(" \t") + 1);
                
                if (varType == "string" && trimmedValue.front() == '"' && trimmedValue.back() == '"') {
                    trimmedValue = trimmedValue.substr(1, trimmedValue.length() - 2);
                }
                
                variables[varName] = trimmedValue;
            }
        }
    }
    
    if (line.find("rename variable") != std::string::npos) {
        std::regex renameDeclaration("rename variable\\s+(\\w+)\\s+(string|numeric|floating|boolean)\\s*=\\s*([^;]+);");
        std::smatch match;
        
        if (std::regex_search(line, match, renameDeclaration)) {
            std::string varName = match[1].str();
            std::string varType = match[2].str();
            std::string varValue = match[3].str();
            
            if (variables.find(varName) == variables.end()) {
                setRedColor();
                std::cerr << "Variable '" << varName << "' does not exist for rename on line " << lineNum << "." << std::endl;
                resetColor();
                return;
            }
            
            if (varValue.find("input(") != std::string::npos) {
                std::regex inputCall("input\\((\"[^\"]*\"|[^)]*)\\)");
                std::smatch inputMatch;
                if (std::regex_search(varValue, inputMatch, inputCall)) {
                    std::string prompt = inputMatch[1].str();
                    if (!prompt.empty() && prompt.front() == '"' && prompt.back() == '"') {
                        prompt = prompt.substr(1, prompt.length() - 2);
                    }
                    std::string inputValue = executeInput(prompt, varType);
                    variables[varName] = inputValue;
                }
            } else {
                std::string trimmedValue = varValue;
                trimmedValue.erase(0, trimmedValue.find_first_not_of(" \t"));
                trimmedValue.erase(trimmedValue.find_last_not_of(" \t") + 1);
                
                if (varType == "string" && trimmedValue.front() == '"' && trimmedValue.back() == '"') {
                    trimmedValue = trimmedValue.substr(1, trimmedValue.length() - 2);
                }
                
                variables[varName] = trimmedValue;
            }
        }
    }
}

void IornInterpreter::processIfStatement(const std::string& cleanCode) {
    size_t ifPos = cleanCode.find("if (");
    size_t thenPos = cleanCode.find("then:");
    size_t elsePerformPos = cleanCode.find("else perform:");
    size_t endifPos = cleanCode.find("endif;");
    
    if (ifPos == std::string::npos || thenPos == std::string::npos || endifPos == std::string::npos) {
        setRedColor();
        std::cerr << "Error: Invalid if statement structure." << std::endl;
        resetColor();
        return;
    }
    
    size_t closeParenPos = cleanCode.find(")", ifPos);
    std::string condition1 = cleanCode.substr(ifPos + 4, closeParenPos - ifPos - 4);
    
    size_t thenStart = thenPos + 5;
    size_t firstElsePos = cleanCode.find("else to if (", thenStart);
    size_t thenEnd = (firstElsePos != std::string::npos) ? firstElsePos : 
                    (elsePerformPos != std::string::npos) ? elsePerformPos : endifPos;
    std::string thenBlock = cleanCode.substr(thenStart, thenEnd - thenStart);
    
    std::vector<std::pair<std::string, std::string>> elseIfBlocks;
    size_t searchPos = thenEnd;
    
    while (searchPos < endifPos) {
        size_t elseToIfPos = cleanCode.find("else to if (", searchPos);
        if (elseToIfPos == std::string::npos || elseToIfPos >= endifPos) break;
        
        size_t resumePos = cleanCode.find(") resume:", elseToIfPos);
        if (resumePos == std::string::npos) break;
        
        std::string condition = cleanCode.substr(elseToIfPos + 12, resumePos - elseToIfPos - 12);
        
        size_t blockStart = resumePos + 9;
        size_t nextElsePos = cleanCode.find("else to if (", blockStart);
        size_t blockEnd = (nextElsePos != std::string::npos && nextElsePos < endifPos) ? nextElsePos :
                         (elsePerformPos != std::string::npos && elsePerformPos > blockStart) ? elsePerformPos : endifPos;
        
        std::string block = cleanCode.substr(blockStart, blockEnd - blockStart);
        elseIfBlocks.push_back({condition, block});
        
        searchPos = blockEnd;
    }
    
    std::string elseBlock = "";
    if (elsePerformPos != std::string::npos) {
        size_t elseStart = elsePerformPos + 13;
        elseBlock = cleanCode.substr(elseStart, endifPos - elseStart);
    }
    
    try {
        if (evaluateCondition(condition1)) {
            executeBlock(thenBlock);
            return;
        }
        
        for (const auto& elseIf : elseIfBlocks) {
            if (evaluateCondition(elseIf.first)) {
                executeBlock(elseIf.second);
                return;
            }
        }
        
        if (!elseBlock.empty()) {
            executeBlock(elseBlock);
        }
    } catch (...) {
        setRedColor();
        std::cerr << "Error: Invalid condition or expression in if statement." << std::endl;
        resetColor();
    }
}

void createPackagedApp(const std::string& sourceFile, const std::string& extension, 
                      const std::string& outName, const std::string& icon, bool loopMain) {
    std::string templateCode = R"(
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

int main() {
    std::string iornCode = R"IORN_CODE(
)IORN_CODE_PLACEHOLDER(
)IORN_CODE";
    
    std::ofstream tempFile("temp_iorn_code.iorn");
    tempFile << iornCode;
    tempFile.close();
    
    std::string command = "iorn.exe temp_iorn_code.iorn";
    int result = system(command.c_str());
    
    remove("temp_iorn_code.iorn");
    
    LOOP_PLACEHOLDER
    
    return result;
}
)";
    
    std::ifstream sourceFileStream(sourceFile);
    if (!sourceFileStream.is_open()) {
        std::cerr << "Error: Cannot open source file " << sourceFile << std::endl;
        return;
    }
    
    std::string iornCode((std::istreambuf_iterator<char>(sourceFileStream)), 
                        std::istreambuf_iterator<char>());
    sourceFileStream.close();
    
    size_t pos = templateCode.find(")IORN_CODE_PLACEHOLDER(");
    if (pos != std::string::npos) {
        templateCode.replace(pos, 23, iornCode);
    }
    
    std::string loopCode = loopMain ? 
        "std::cout << \"Press Enter to exit...\"; std::cin.get();" : "";
    pos = templateCode.find("LOOP_PLACEHOLDER");
    if (pos != std::string::npos) {
        templateCode.replace(pos, 16, loopCode);
    }
    
    std::string cppFile = outName + "_generated.cpp";
    std::ofstream cppFileStream(cppFile);
    cppFileStream << templateCode;
    cppFileStream.close();
    
    // Create resource file with icon
    std::string rcFile = outName + ".rc";
    std::ofstream rcStream(rcFile);
    rcStream << "IDI_ICON1 ICON \"..\\assets\\iorn_ico_exeFile_ordinary.ico\"" << std::endl;
    rcStream.close();
    
    // Compile resource and then executable
    std::string rcCompileCmd = "windres " + rcFile + " -o " + outName + ".o";
    int rcResult = system(rcCompileCmd.c_str());
    
    std::string compileCmd = "g++ -std=c++11 -O2 -o " + outName + extension + " " + cppFile;
    if (rcResult == 0) {
        compileCmd += " " + outName + ".o";
    }
    
    if (system(compileCmd.c_str()) == 0) {
        std::cout << "Successfully packaged to: " << outName << extension << std::endl;
        remove(cppFile.c_str());
        remove((outName + ".rc").c_str());
        remove((outName + ".o").c_str());
        remove((outName + ".res").c_str());

    } else {
        std::cerr << "Error: Failed to compile packaged application" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    IornInterpreter interpreter;
    
    if (argc > 1) {
        std::string firstArg = argv[1];
        
        bool isPackaging = false;
        std::string sourceFile, extension = ".exe", outName = "app", icon = "False";
        bool loopMain = false;
        
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg.find("--package=") == 0) {
                isPackaging = true;
                extension = arg.substr(10);
            } else if (arg.find("--out_name=") == 0) {
                outName = arg.substr(11);
            } else if (arg.find("--icon=") == 0) {
                icon = arg.substr(7);
            } else if (arg.find("--loop_main=") == 0) {
                std::string loopStr = arg.substr(12);
                loopMain = (loopStr == "True" || loopStr == "true");
            } else if (arg.find(".iorn") != std::string::npos) {
                sourceFile = arg;
            }
        }
        
        if (isPackaging) {
            if (sourceFile.empty()) {
                std::cerr << "Error: No .iorn source file specified" << std::endl;
                return 1;
            }
            createPackagedApp(sourceFile, extension, outName, icon, loopMain);
        } else {
            interpreter.interpretFile(firstArg);
        }
    } else {
        std::cout << "Usage: iorn <filename.iorn>" << std::endl;
    }
    
    return 0;
}