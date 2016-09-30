
#pragma once 

#include <mutex>

#include "mem/serializer.h"

namespace nitro
{
namespace command
{
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
}
}   