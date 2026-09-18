# Scrapes "void test_xxx(void)" out of a Unity test file and writes a main()
# that runs each one. Keeps CI free of any Ruby dependency.

file(READ ${SRC} content)
string(REGEX MATCHALL "void[ \t]+test_[A-Za-z0-9_]+[ \t]*\\([ \t]*void[ \t]*\\)" matches "${content}")

set(decls "")
set(calls "")
foreach(m IN LISTS matches)
    string(REGEX REPLACE "void[ \t]+(test_[A-Za-z0-9_]+).*" "\\1" fn "${m}")
    set(decls "${decls}void ${fn}(void);\n")
    set(calls "${calls}    RUN_TEST(${fn});\n")
endforeach()

file(WRITE ${OUT}
"#include \"unity.h\"

void setUp(void);
void tearDown(void);
${decls}
int main(void)
{
    UNITY_BEGIN();
${calls}    return UNITY_END();
}
")
