
#pragma once 

#include "command.h"
#include "serializer.h"




namespace nitro
{
namespace command
{


#include "../../../nitro_cloud/include/sync_out_puter.h"


const char fileName[] = "../test.txt";

class CountImpl
    : public Count
{
public:
    CountImpl(int rowIndex = -1)
        : Count(rowIndex)
    {
    }

    std::string  Run() override
    {
        nitro::serializer::Reader reader(fileName);
        auto row = reader.GetRowById(m_rowIndex);

        std::stringstream   outputBuffer;

        if (row != nullptr)
        {
            auto charsCount = row->GetCharsCount();
            outputBuffer << "Row[" << m_rowIndex << "] = " << charsCount;
#ifdef ENABLE_LOG
            SYNC_OUTPUT("=CountImpl=") << "Result - the chars count of Row[" << m_rowIndex << "] = " << charsCount;
#endif

        }
        else
        {
            outputBuffer << "Row[" << m_rowIndex << "] = <error>";
        }

        return outputBuffer.str();
    }
};


class CommandGenerator
    : public ICommandGenerator
{
public:
    CommandGenerator()
        : m_iteration(4000)
    {
    }

    virtual std::shared_ptr<nitro::command::Base> NextCommand()
    {
        if (m_iteration == 0)
        {
            return nullptr;
        }

        --m_iteration;


        auto rowIndex = rand() % 2999;

#ifdef ENABLE_LOG
        SYNC_OUTPUT("=CommandGenerator=") << "NextCommand - CountImpl. rowIndex = " << rowIndex;
#endif

        return  std::make_shared<CountImpl>(rowIndex);
    }

    virtual std::shared_ptr<Base> FromStream(std::istream& readStream)
    {
#ifdef ENABLE_LOG
        SYNC_OUTPUT("=CommandGenerator=") << "parse commad stream";
#endif

        switch (Base::ReadNextCommandType(readStream))
        {
            case CountCommand:
            {
#ifdef ENABLE_LOG
                SYNC_OUTPUT("=CommandGenerator=") << "Read CommandType from stream. The type is CountCommand";
#endif
                return Base::Deserialize<CountImpl>(readStream);
            }
            break;
        }

        return nullptr;
    }

private:
    int  m_iteration;
};

}
}

