
#pragma once


#include <memory>
#include <functional>
#include <thread>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/bind.hpp>


namespace Nitro
{
namespace Community
{

class TaskCounter
{
public:
    TaskCounter()
    {
        m_countLocker.lock();
        ++m_counter;
        m_countLocker.unlock();
    }

    ~TaskCounter()
    {
        m_countLocker.lock();
        --m_counter;
        m_countLocker.unlock();
    }


    static int GetTaskCount()
    {
        return m_counter;
    }

private:
    static std::mutex  m_countLocker;
    static int         m_counter;
};

#define INIT_TASK_COUNTER() std::mutex  Nitro::Community::TaskCounter::m_countLocker; \
        int  Nitro::Community::TaskCounter::m_counter = 0;

#define MESSAGE_SIZE    2048

template<class TChar, class TAsioTransport>
class ServerTask
    : public std::enable_shared_from_this<ServerTask<TChar, TAsioTransport>>
{
public:

    ServerTask(boost::asio::io_service& ioService)
        : m_currentState(State::None)
        , m_bufferSend(MESSAGE_SIZE, 0)
        , m_bufferRecv(MESSAGE_SIZE, 0)
        , m_transport(ioService)
        , m_id(std::rand() % 1024)
    {
        SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Start new task." << m_id;
    }


    ~ServerTask()
    {
        SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Finish Task.";
        //helper::close(m_connection);
    }

    void RunIdle()
    {
        SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Idle at process.";
    }

    TAsioTransport& GetTransport()
    {
        return m_transport;
    }

    void StartListen() {
        SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Async read start. State = " << m_currentState;

        m_transport.async_read_some(boost::asio::buffer(&m_bufferRecv[0], MESSAGE_SIZE),
                                    boost::bind(&ServerTask<TChar, TAsioTransport>::HandleRead,
                                    this->shared_from_this(),
                                    boost::asio::placeholders::error));

        m_taskCounter = std::make_shared<TaskCounter>();
    }


private:
    void DoResponse(std::vector<std::uint8_t>& responseBuffer)
    {
        ++m_currentState;

        std::string     response;

        switch (m_currentState)
        {
            case State::Hello:
            {
                response = "Send hello from Server.";
            }
            break;
            case State::HowArYou:
            {
                response = "Send \"how are you\" from Server.";
            }
            break;
            case State::BeforeRandomSleep:
            {
                response = "Send \"before random sleep\" from Server.";
                std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 2000));
            }
            break;
            case State::AfterRandomSleep:
            {
                response = "Send \"after random sleep\" from Server.";
            }
            break;
            case State::Buy:
            {
                response = "Send buy from Server.";
            }
            break;
        }

        int copyIndex = 0;
        for (auto& symbol : response)
        {
            responseBuffer[copyIndex++] = symbol;
        }
    }



    void HandleRead(const boost::system::error_code& error) {
        if (error) {
            SYNC_OUTPUT() << "[Task:" << m_id << "]" << "ERROR!" << error;
            return;
        }

        SYNC_OUTPUT() << "[Task:" << m_id << "]" <<
            "Async read end. Read buffer was get:" << (char *)&m_bufferRecv[0];


        if (m_currentState != State::Buy) {
            StartListen();
        }

        int currentState = m_currentState;

        DoResponse(m_bufferSend);

        SYNC_OUTPUT() << "[Task:" << m_id << "]" <<
            "Async write start. Write buffer for sending:" <<
            (char *)&m_bufferSend[0];

        m_transport.async_write_some(
            boost::asio::buffer(&m_bufferSend[0], MESSAGE_SIZE),
            boost::bind(&ServerTask<TChar, TAsioTransport>::HandleWrite,
            this->shared_from_this(),
            boost::asio::placeholders::error));
    }

    void HandleWrite(const boost::system::error_code &error) {
        if (error) {
            SYNC_OUTPUT() << "[Task:" << m_id << "]" << "ERROR!" <<
                error;
            return;
        }

        SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Async write end.";

        if (m_currentState == State::Buy) {
            SYNC_OUTPUT() << "[Task:" << m_id << "]" <<
                "current state is Buy. So we finish task.";
        }
    }


    enum State
    {
        None,
        Hello,
        HowArYou,
        BeforeRandomSleep,
        AfterRandomSleep,
        Buy,
    };

    int                             m_id;

    int                             m_currentState;
    std::vector<std::uint8_t>       m_bufferSend;
    std::vector<std::uint8_t>       m_bufferRecv;

    TAsioTransport                  m_transport;

    std::shared_ptr<TaskCounter>    m_taskCounter;
};


}
}