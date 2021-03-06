
#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "../mem/serializer.h"

namespace nitro
{
namespace command
{

enum Type
{
    CountCommand,
    ExitCommand,
    //Open,
};


namespace statistic
{

struct Result
{
    std::string     request;
    std::string     response;
};


class ResultCollection :
    public serializer::FileHolder
{
public:
    ResultCollection(const std::string& file)
        : FileHolder(file, "w")
    {
    }

    void AddResult(std::shared_ptr<Result>& result)
    {
        m_collecitonLocker.lock();
        m_collection.push_back(result);
        m_collecitonLocker.unlock();
    }

    void Dump()
    {
        std::stringstream   resultBuffer;

        int rowCount = 0;
        
        m_collecitonLocker.lock();

        for (auto& result : m_collection)
        {
            resultBuffer << rowCount << ". Command Send Buffer = " << ", Command Receive Buffer = " << result->response << "\n";
            rowCount++;
        }
        
        m_collecitonLocker.unlock();

        resultBuffer.flush();

        auto result = resultBuffer.str();
        fwrite(result.c_str(), sizeof(char), result.length(), GetFile());
    }

private:
    using ResultVector = std::vector<std::shared_ptr<Result>>;

    ResultVector    m_collection;
    std::mutex      m_collecitonLocker;
};

}


class Base
{
public:

    static Type ReadNextCommandType(std::istream& readStream)
    {
        auto saveSeek = readStream.tellg();

        int type = 0;
        readStream >> type;
        readStream.seekg(saveSeek, std::ios_base::beg);

        return (Type) type;
    }


    template<class TBase>
    static std::shared_ptr<TBase>  Deserialize(std::istream& readStream)
    {
        int type = 0;

        auto saveSeek = readStream.tellg();

        readStream >> type;

        std::shared_ptr<Base> readValue(new TBase());

        if (readValue->m_type != type)
        {
            readStream.seekg(saveSeek, std::ios_base::beg);
            return nullptr;
        }

        readValue->Read(readStream);

        return std::dynamic_pointer_cast<TBase>(readValue);
    }

    static bool Serialize(std::ostream& writeStream, const Base& base)
    {
        writeStream << base.m_type << '\r';

        base.Write(writeStream);

        writeStream << '\r';

        return true;
    }

    virtual std::string  Run()
    {
        assert(0 && "Implement in derived class");
        return "";
    }

protected:
    Base(Type type)
        : m_type(type)
    {
    }

    virtual void Read(std::istream& readStream) = 0;
    virtual void Write(std::ostream& writeStream) const = 0;

private:
    int  m_type;
};


class Count
    : public Base
{
public:
    Count(const int& rowIndex)
        : Base(CountCommand)
        , m_rowIndex(rowIndex)
    {
    }


protected:
    void Read(std::istream& readStream) override
    {
        readStream >> m_rowIndex;
    }

    void Write(std::ostream& writeStream)  const override
    {
        writeStream << m_rowIndex << '\r';
    }

protected:
    int   m_rowIndex;
};

class ICommandGenerator
{
public:
    virtual bool HasCommands() = 0;

    virtual std::shared_ptr<nitro::command::Base> NextCommand() = 0;
    virtual std::shared_ptr<nitro::command::Base> FromStream(std::istream& readStream) = 0;
};

}
}