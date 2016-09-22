
#pragma once

#include <string>
#include <vector>

namespace nitro
{
namespace element
{
#define ROW_END             '\n'

class Row
{
public:
    Row(const std::string& value)
        : m_value(value)
    {
    }
    
    size_t Write(FILE* fileToWrite)
    {
        static const char rowEnd = ROW_END;
        size_t offsetSeek = fwrite(m_value.c_str(), sizeof(char), m_value.length(), fileToWrite);
        offsetSeek += fwrite(&rowEnd, sizeof(char), 1, fileToWrite);
        return offsetSeek;
    }

    bool  Read(FILE* fileToRead)
    {
        m_value.clear();

        char c = EOF;
        do
        {
            c = fgetc(fileToRead);
            
            if (c != EOF)
            {
                m_value.push_back(c); 
            }

        } while (c != EOF && c != ROW_END);

        m_value.push_back(0); 

        return !m_value.empty(); 
    }

    size_t GetCharsCount() 
    { 
        return m_value.length(); 
    }

private:
    std::string m_value;
};

}
}