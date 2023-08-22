#include "../dictionary_query/word_search.c"
#include "../dictionary_query/dictionary_query.h"
#include <assert.h>
#include "../logs/logs.h"

int main(int argc, char const *argv[])
{
    bool load_result = load_dictionary("../dictionary_query/dict.txt");
    assert(load_result == true);
    LogInfo("测试1: 加载词典文件成功");

    const char* definition = search_word("abandon");
    assert(definition != NULL);
    LogInfo("测试2: 查找单词成功");

    const char *expected = "v.  go away from (a person or thing or place) not intending to return; forsake; desert";
    assert(strcmp(definition, "v.  go away from (a person or thing or place) not intending to return; forsake; desert") == 0);
    LogInfo("测试3: 查找单词意思成功");

    definition = search_word("nonexistentword");
    assert(definition == NULL);
    LogInfo("测试3: 非法单词测试成功");

    LogInfo("所有函数的测试均已通过!");

    return 0;
}
