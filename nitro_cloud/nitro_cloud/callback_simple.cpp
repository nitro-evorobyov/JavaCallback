
#include <string.h>
#include <thread>
#include <chrono>

#include <sstream>

#include "callback_simple.h"



__declspec(dllexport) void __stdcall RunSimple(int stepsCount, callback javaCallback) {


    std::thread([stepsCount, javaCallback](){

        std::thread::id     threadId = std::this_thread::get_id();
        std::stringstream   callbackMessage;

        callbackMessage << "Thread id = " << threadId << ". Start.";

        (*javaCallback) ("Hello from C++.", callbackMessage.str().c_str());


        for (int step = 0; step < stepsCount; ++step)
        {
            callbackMessage.str("");
            callbackMessage << "Thread id = " << threadId << ". In progress.";

            std::stringstream progressMessage;
            progressMessage << " Step: " << step << "/" << stepsCount << ".";

            (*javaCallback) (progressMessage.str().c_str(), callbackMessage.str().c_str());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        (*javaCallback)("See Later.", "Finished.");
    }).detach();
}

