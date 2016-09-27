
#pragma once 

#include "command.h"
#include "../mem/serializer.h"
#include "../sync_out_puter.h"


namespace nitro
{
namespace command
{

const char fileName[] = "../test.txt";

class CountImpl
    : public Count
{
public:
    CountImpl(int rowIndex = -1, 
              std::shared_ptr<nitro::serializer::Reader> reader = nullptr)
        : Count(rowIndex)
        , m_reader (reader)
    {
    }

    void SetReader(std::shared_ptr<nitro::serializer::Reader> reader)
    {
        m_reader = reader;
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

private:

    std::shared_ptr<nitro::serializer::Reader>  GetReader()
    {
        if (m_reader == nullptr)
        {
            m_reader.reset(new nitro::serializer::Reader(fileName));
        }

        return m_reader;
    }

private:
    std::shared_ptr<nitro::serializer::Reader>  m_reader;
};


class CommandGenerator
    : public ICommandGenerator
{
public:
    CommandGenerator(std::shared_ptr<nitro::serializer::Reader> reader = nullptr)
        : m_iteration(4000)
        , m_commonReader(reader)
    {
    }

    bool HasCommands() override
    {
        return m_iteration > 0;
    }

    std::shared_ptr<nitro::command::Base> NextCommand() override
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

    std::shared_ptr<Base> FromStream(std::istream& readStream) override
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
                auto command = Base::Deserialize<CountImpl>(readStream);
                
                if (m_commonReader != nullptr)
                {
                    command->SetReader(m_commonReader);
                }

                return command;
            }
            break;
        }

        return nullptr;
    }

private:
    int     m_iteration;

    std::shared_ptr<nitro::serializer::Reader>  m_commonReader;
};

}
}

