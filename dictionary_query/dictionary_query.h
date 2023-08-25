#ifndef DICTIONARY_QUERY_H
#define DICTIONARY_QUERY_H

#include "../utils/common_structs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*这个函数的目的是从一个指定的文件（例如 dict.txt）中加载词典。词典的每一行包含一个单词和对应的释义，中间由空格分隔。这个函数会将文件中的每一个词条（单词和对应的释义）读取到一个DictionaryEntry类型的数组中。

参数：
filename: 要加载的词典文件的名称（包含路径）
返回值：
如果加载成功返回 true，否则返回 false*/ 
bool load_dictionary(const char *filename);
/*这个函数的目的是在已加载的词典中查找指定的单词，并返回这个单词的释义。

参数：

word: 要查找的单词。
返回值：

如果找到了单词，返回该单词的释义。
如果没有找到单词，返回 NULL*/ 
const char* search_word(const char *word);

#endif
