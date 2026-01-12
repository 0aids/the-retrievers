# Code style
```c
typedef struct {
    int valuesDontNeedNamespacing;
    int iHaveAnAcroynymFSM;
} namespace_structNameGoesHere_t;

// _g prefix for global variables
extern namespace_nameOfGlobalVariable_g;

typedef enum {
    namespace_enumName_enumValueName
} namespace_enumName_e;

#define namespace_valueNameIsCamelCase_d whatever
const type namespace_valueName_c;
// static vars, if local to a file, don't matter too much just
// need camel casing.
static type valueName;

void namespace_functionName(int inputsNameShouldBeCamel)
{
    size_t scopedVarsShouldBeCamel;
    return scopedVarsShouldBeCamel;
}
```
