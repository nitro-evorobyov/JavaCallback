
#pragma once

#include <memory>
#include <functional>
#include <thread>
#include <mutex>

#include "helper.h"


namespace Nitro
{
namespace Community
{

template<class TChar> class ServerTask;


template<class TChar>
class ServerTaskCollector
    : public boost::noncopyable
{
public:
    typedef std::shared_ptr<ServerTask<TChar>>  ServerTaskPtr;

    void Add(ServerTaskPtr& serverTaskPtr)
    {
        m_collectionSynchronizer.lock();

        m_serverTaskCollection.push_back(serverTaskPtr);

        m_collectionSynchronizer.unlock();
    }

    int AllTaskIdleRun()
    {
        m_collectionSynchronizer.lock();

        size_t collectionLength = m_serverTaskCollection.size();

        std::for_each(m_serverTaskCollection.begin(),
                      m_serverTaskCollection.end(),
                      [](const ServerTaskPtr& p)
        {
            p->RunIdle();
        });

        m_collectionSynchronizer.unlock();

        return static_cast<int>(collectionLength);
    }

    void ClearAll()
    {
        m_collectionSynchronizer.lock();
        
        m_serverTaskCollection.clear(); 

        m_collectionSynchronizer.unlock();
    }

    void Remove(ServerTaskPtr& serverTaskPtr)
    {
        m_collectionSynchronizer.lock();

        auto iteratorToRemove = std::find(m_serverTaskCollection.begin(), 
                                          m_serverTaskCollection.end(), 
                                          serverTaskPtr);

        if (iteratorToRemove != m_serverTaskCollection.end())
        {
            m_serverTaskCollection.erase(iteratorToRemove);
        }

        m_collectionSynchronizer.unlock();
    }


private:
    std::vector<ServerTaskPtr>  m_serverTaskCollection;
    std::mutex                  m_collectionSynchronizer;
};



#define MESSAGE_SIZE    2048

template<class TChar>
class ServerTask 
    : public std::enable_shared_from_this<ServerTask<TChar>>
{
public:

    static  std::shared_ptr<ServerTask<TChar>> 
        CreateInstance(helper::native_io_t connection,
        boost::asio::strand& strand,
        ServerTaskCollector<TChar>& taskCollector)
    {
        std::shared_ptr<ServerTask<TChar>> serverTask(new ServerTask<TChar>(connection, strand, taskCollector));
        taskCollector.Add(serverTask);
        return serverTask;
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

    void Finish()
    {
        m_serverTaskCollector.Remove(shared_from_this());
    }

private:

    ServerTask(helper::native_io_t connection,
               boost::asio::strand& strand,
               ServerTaskCollector<TChar>& taskCollector)
        : m_currentState(State::None)
        , m_bufferSend(MESSAGE_SIZE, 0)
        , m_bufferRecv(MESSAGE_SIZE, 0)
        , m_strand(strand)
        , m_connectionAsyncStream(strand.get_io_service(), connection)
        , m_connection(connection)
        , m_serverTaskCollector(taskCollector)
        , m_id(std::rand() % 1024)
    {
        SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Start new task." << m_id;
        Read();
    }


    void Read()
    {
        SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Async read start. State = " << m_currentState;

        m_connectionAsyncStream.async_read_some(boost::asio::buffer(&m_bufferRecv[0], MESSAGE_SIZE),
                                                [this](const boost::system::error_code& error, std::size_t bytes_transferred)
        {
            if (error)
            {
                SYNC_OUTPUT() << "[Task:" << m_id << "]" << "ERROR!" << error;
                return;
            }

            SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Async read end. Read buffer was get:" << (char*)&m_bufferRecv[0];


            if (m_currentState != State::Buy)
            {
                Read();
            }

            int currentState = m_currentState;

            DoResponse(m_bufferSend);

            SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Async write start. Write buffer for sending:" << (char*)&m_bufferSend[0];

            m_connectionAsyncStream.async_write_some(boost::asio::buffer(&m_bufferSend[0], MESSAGE_SIZE),
                                                        [this, currentState](const boost::system::error_code& error, std::size_t bytes_transferred)
            {
                if (error)
                {
                    SYNC_OUTPUT() << "[Task:" << m_id << "]" << "ERROR!" << error;
                    return;
                }

                SYNC_OUTPUT() << "[Task:" << m_id << "]" << "Async write end.";

                if (currentState == State::Buy)
                {
                    SYNC_OUTPUT() << "[Task:" << m_id << "]" << "current state is Buy. So we finish task.";
                    Finish();
                }
            });
        });
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
    
    int                         m_id;

    int                         m_currentState;
    std::vector<std::uint8_t>   m_bufferSend;
    std::vector<std::uint8_t>   m_bufferRecv;

    boost::asio::strand&        m_strand;
    ServerTaskCollector<TChar>& m_serverTaskCollector;

    helper::async_stream_t      m_connectionAsyncStream;
    helper::native_io_t         m_connection;
};




template<class TChar>
class Server
    : public boost::noncopyable
{
public:
    Server(const std::basic_string<TChar>& pipePostfix, boost::asio::io_service& service, boost::asio::strand& strand)
      : m_service(service)
      , m_strand(strand)
      , m_serviceHolder(new boost::asio::io_service::work(service))
      , m_stopLoop(false)
    {
        SYNC_OUTPUT() << "[Server:]" << "Start server.";
        helper::generate_pipe_name(pipePostfix, m_pipeName);
        helper::create_server(m_pipeServer, m_pipeName);
    }

    ~Server()
    {
        helper::close(m_pipeServer);
        SYNC_OUTPUT() << "[Server:]" << "Finish server.";
    }

    void Run()
    {
        SYNC_OUTPUT() << "[Server:]" "Start awaiting loop.";

        while (true)
        {
            if (m_stopLoop)
            {
                break;
            }

            helper::native_io_t connection = helper::accept_connection(m_pipeServer, m_pipeName);
            if (helper::is_valid(connection))
            {
                ServerTask<TChar>::CreateInstance(connection, m_strand, m_taskHolder);
                continue;
            }

            break;
        }
    }

    int OnIdle()
    {
        return m_taskHolder.AllTaskIdleRun();
    }

    void Stop()
    {
        m_stopLoop = true;
        m_serviceHolder.reset();
        m_taskHolder.ClearAll();

        helper::native_io_t unblockConnection = helper::connect_to_server(m_pipeName);
        helper::close (unblockConnection);
    }

    const std::basic_string<TChar>&     GetPipeName()
    { 
        return m_pipeName; 
    }

private:


    boost::asio::io_service&        m_service;
    boost::asio::strand&            m_strand;
    
    helper::native_io_t             m_pipeServer;
    std::basic_string<TChar>        m_pipeName;

    ServerTaskCollector<TChar>      m_taskHolder;

    bool                            m_stopLoop;

    std::shared_ptr<boost::asio::io_service::work>   m_serviceHolder;
};

}
}