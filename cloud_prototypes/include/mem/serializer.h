
#pragma once
#include <memory>
#include <sstream>
#include <vector>

#include "row.h"

#pragma warning(push)
#pragma warning( disable: 4996 )

namespace nitro
{
namespace serializer
{

#define XREF_END     "\n"

class FileHolder
{
protected:
    FileHolder(const std::string& file, char* mode)
    {
        m_file = fopen(file.c_str(), mode);
    }

    virtual ~FileHolder()
    {
        fflush(m_file);
        fclose(m_file);

        m_file = nullptr;
    }

    FILE* GetFile()
    {
        return  m_file;
    }

private:
    FILE*       m_file;
};


class Reader
    : public FileHolder
{
public:
    Reader(const std::string& file)
        : FileHolder(file, "rb+")
    {
    }

    std::shared_ptr<element::Row>  GetRowById(uint16_t rowId)
    {
        std::shared_ptr<element::Row> result;

        FILE* fileForReading = GetFile();

        if (fileForReading != nullptr)
        {
            size_t      offsetRefTable = 0;
            fread(&offsetRefTable, 1, sizeof(offsetRefTable), fileForReading);

            fseek(fileForReading, offsetRefTable, SEEK_SET);

            size_t sizeRowsCollection = 0;
            offsetRefTable += fread(&sizeRowsCollection, 1, sizeof(sizeRowsCollection), fileForReading);

            for (size_t rowIndex = 0; rowIndex < sizeRowsCollection; ++rowIndex)
            {
                size_t  offsetRef = 0;
                fread(&offsetRef, 1, sizeof(offsetRef), fileForReading);
                offsetRefTable += sizeof(offsetRef);

                uint16_t    rowNumber = 0;

                fread(&rowNumber, 1, sizeof(rowNumber), fileForReading);
                offsetRefTable += sizeof(rowNumber);

                if (rowId == rowNumber)
                {
                    fseek(fileForReading, offsetRef, SEEK_SET);

                    result.reset(new element::Row(""));
                    result->Read(fileForReading);

                    break;
                }
            }
        }

        return result;
    }

    void Read(std::vector<std::shared_ptr<element::Row>>& collection)
    {
        FILE* fileForReading = GetFile();

        if (fileForReading != nullptr)
        {
            size_t      offsetRefTable = 0;
            fread(&offsetRefTable, 1, sizeof(offsetRefTable), fileForReading);

            fseek(fileForReading, offsetRefTable, SEEK_SET);

            size_t sizeRowsCollection = 0;
            offsetRefTable += fread(&sizeRowsCollection, 1, sizeof(sizeRowsCollection), fileForReading);

            for (size_t rowIndex = 0; rowIndex < sizeRowsCollection; ++rowIndex)
            {
                size_t  offsetRef = 0;
                fread(&offsetRef, 1, sizeof(offsetRef), fileForReading);
                offsetRefTable += sizeof(offsetRef);
                uint16_t    rowNumber = 0;
                fread(&rowNumber, 1, sizeof(rowNumber), fileForReading);
                offsetRefTable += sizeof(rowNumber);

                fseek(fileForReading, offsetRef, SEEK_SET);

                std::shared_ptr<element::Row>   newRow(new element::Row(""));
                newRow->Read(fileForReading);

                fseek(fileForReading, offsetRefTable, SEEK_SET);

                collection.push_back(newRow);
            }
        }
    }
};

class Writer
    : public FileHolder
{
public:
    Writer(const std::string& file)
        : FileHolder(file, "wb")
    {
    }

    void Write(std::vector<std::shared_ptr<element::Row>>& collection)
    {
        FILE* fileForWriting = GetFile();

        if (fileForWriting != nullptr)
        {
            std::vector<std::pair<size_t, uint16_t>> offsetCollection;

            uint16_t    number = 0;
            size_t      offsetSeek = 0;

            size_t      offsetRefTable = 0;

            //reserve start place for xref offset:
            offsetSeek += fwrite(&offsetRefTable, 1, sizeof(offsetRefTable), fileForWriting);

            for (const auto& itemtoWrite : collection)
            {
                size_t writeSize = itemtoWrite->Write(fileForWriting);
                offsetCollection.push_back({ offsetSeek, ++number });
                offsetSeek += writeSize;
            }

            offsetRefTable = offsetSeek;

            size_t sizeRowsCollection = offsetCollection.size();
            fwrite(&sizeRowsCollection, 1, sizeof(sizeRowsCollection), fileForWriting);


            for (const auto& offset : offsetCollection)
            {
                fwrite(&offset.first, 1, sizeof(offset.first), fileForWriting);
                fwrite(&offset.second, 1, sizeof(offset.second), fileForWriting);
            }

            fseek(fileForWriting, 0, SEEK_SET);
            fwrite(&offsetRefTable, 1, sizeof(offsetRefTable), fileForWriting);
        }
    }
};

}
}

#pragma warning(pop)