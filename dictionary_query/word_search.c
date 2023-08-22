#include "../dictionary_query/dictionary_query.h"
#include "../utils/common_structs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../logs/logs.h"

#define MAX_ENTRIES 10000 // 假设词典最多有10000个词条

Response dictionary[MAX_ENTRIES];
int total_entries = 0;

bool load_dictionary(const char *filename)
{
    FILE *file = fopen(filename ,"r");
    if (!file)
    {
        LogError("无法打开词典 \n");
        return false;
    }

    char line[600];
    while(fgets(line, sizeof(line), file) && total_entries < MAX_ENTRIES)
    {
        char *token = strtok(line, " "); // 使用空格或制表符作为分隔符
        if(!token)
            continue;

        strncpy(dictionary[total_entries].word, token, sizeof(dictionary[total_entries].word));

        token = strtok(NULL, "\r\n"); // 获取该行的剩余部分，即释义
        if(token)
        {
            while (*token == ' ' || *token == '\t')
            {
                token++;
            }
            strncpy(dictionary[total_entries].definition, token, sizeof(dictionary[total_entries].definition));
        }

        total_entries++;
    }

    fclose(file);
    return true; 
}

const char* search_word(const char *word)
{
    for (int i = 0; i < total_entries; i++)
    {
        if(strcmp(dictionary[i].word, word) == 0)
        {
            return dictionary[i].definition;
        }
    }
    return NULL;
}
