#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstddef>
#include "inc/parser.h"
#include <cstring>
#include <cstdio>

namespace Parser {

namespace {

bool simpleCheckIsEndSymbol(const std::string &line) {
    const std::string endSymbol = "[END]";

    return line.compare(0, endSymbol.size(), endSymbol) == 0;
}

inline bool parseObjectRecord(std::ifstream &ifs,
                              const std::map<std::string, VariableMapping> &structMapping,
                              ObjectRecord &objectResult) {
    bool isEndSymbolExtracted = false;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.size() == 0) {
            continue;
        }

        bool isLineEndSymbol = simpleCheckIsEndSymbol(line);
        if (isLineEndSymbol) {
            isEndSymbolExtracted = true;
            break;
        }

        bool isDetectedLabelSymbol = 
            (line.find('[') != std::string::npos || line.find(']') != std::string::npos);
        if (isDetectedLabelSymbol) {
            break;
        }

        std::istringstream iss(line);

        std::string variableName;
        std::getline(iss, variableName, ',');
        if (structMapping.find(variableName) == structMapping.end()) {
            continue;
        }

        std::vector<MEMBER_T> numberList;
        std::string parseNumber;
        size_t numberCount = 0;
        while (std::getline(iss, parseNumber, ',')) {
            if (parseNumber.size() == 0) {
                continue;
            }
            MEMBER_T number;
            const char* cstr_parseNumber = parseNumber.c_str();

            int matched = sscanf(cstr_parseNumber, "%d", &number);
            if (matched < 1) {
                continue;
            }
            numberList.push_back(number);
            numberCount += 1;
        }

        const VariableMapping &varMapping = structMapping.find(variableName)->second;
        if (numberList.size() != varMapping.itemCount) {
            continue;
        }

        VariableRecord record;
        record.variableName = variableName;
        record.variableList = numberList;
        objectResult.data.push_back(record);

        std::clog << "Add valid record: " << variableName;
        std::clog << "\titemCount: " << numberCount << std::endl;
    }

    return isEndSymbolExtracted;
}

inline bool isObjectStartSymbol(const std::string &line, const size_t startSymbolCount) {
    size_t lpos = line.find('[');
    size_t rpos = line.rfind(']');
    if (lpos == std::string::npos || rpos == std::string::npos || lpos >= rpos) {
        return false;
    }

    const std::string matchNumberString = line.substr(lpos + 1, (rpos - 1) - (lpos + 1) + 1);

    size_t matchNumber = strtol(matchNumberString.c_str(), NULL, 10);

    if (matchNumber != startSymbolCount) {
        return false;
    }

    return true;
}

} // end of anonymous namespace

// FIXME use std::optional as return value (C++17)
bool readFile(std::string fileName,
              const std::map<std::string, VariableMapping> &structMapping,
              std::vector<ObjectRecord> &parseResult)
{
    std::ifstream ifs;

    ifs.open(fileName.c_str(), std::ifstream::in);

    if (!ifs) {
        return false;
    }

    size_t startSymbolCount = 0;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.size() == 0) {
            continue;
        }

        // Match Start Symbol [0], [1], ...
        bool isMatchedStartSymbol = isObjectStartSymbol(line, startSymbolCount);

        if (!isMatchedStartSymbol) {
            // start symbol error
            return false;
        }

        startSymbolCount++;

        // Match End Symbol [END]
        ObjectRecord objectRecord;
        bool isParseSucceeded = parseObjectRecord(ifs, structMapping, objectRecord);

        if (!isParseSucceeded) {
            // end symbol error
            return false;
        }

        parseResult.push_back(objectRecord);
    }
    return true;
}

bool applyParseRecord(void *struct_object,
                      const std::map<std::string, VariableMapping> &structMapping,
                      const std::vector<VariableRecord> &parseResult)
{
    uint8_t *object = (uint8_t *)struct_object;
    for (std::vector<VariableRecord>::size_type i = 0; i < parseResult.size(); i++) {
        const std::string &varName = parseResult[i].variableName;
        const std::vector<MEMBER_T> &varList = parseResult[i].variableList;

        const std::map<std::string, VariableMapping>::const_iterator &iter
            = structMapping.find(varName);
        if (iter == structMapping.end()) {
            continue;
        }

        const VariableMapping &varMapping = iter->second;
        if (varMapping.itemCount != varList.size()) {
            continue;
        }

        memcpy(object + varMapping.variableOffset, varList.data(),
               sizeof(MEMBER_T) * varList.size());
    }

    return true;
}

bool applyParseRecord(void *object,
                      const void *objectDefaultValue, const size_t objectSize,
                      const std::map<std::string, VariableMapping> &structMapping,
                      const std::vector<ObjectRecord> &parseResult,
                      size_t idx)
{
    if (idx >= parseResult.size()) {
        return false;
    }

    if (objectDefaultValue) {
        memcpy(object, objectDefaultValue, objectSize);
    }

    return applyParseRecord(object, structMapping, parseResult[idx].data);
}

}
