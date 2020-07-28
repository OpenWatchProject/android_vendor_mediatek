#include <string>
#include <map>
#include <vector>

// For c++98
typedef int32_t MEMBER_T;
//using MEMBER_T = int32_t;
// FIXME std::stoul should be updated if change MEMBER_T type

namespace Parser {

struct VariableMapping {
    size_t itemCount;
    size_t variableOffset;
};

struct VariableRecord {
    std::string variableName;
    std::vector<MEMBER_T> variableList;
};

struct ObjectRecord {
    std::vector<VariableRecord> data;
};

bool readFile(std::string fileName,
              const std::map<std::string, VariableMapping> &structMapping,
              std::vector<ObjectRecord> &parseResult);

// deprecated
bool applyParseRecord(void *object,
                      const std::map<std::string, VariableMapping> &structMapping,
                      const std::vector<VariableRecord> &parseResult);

bool applyParseRecord(void *object,
                      const void *objectDefaultValue, const size_t objectSize,
                      const std::map<std::string, VariableMapping> &structMapping,
                      const std::vector<ObjectRecord> &parseResult,
                      size_t idx);

} // end of namespace Parser
