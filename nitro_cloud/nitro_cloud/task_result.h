
#pragma once

namespace nitro
{
namespace task
{

enum TaskCallbackCommnad
{
    TaskNotImplemented,
    TaskContinue,
    TaskCancel,
};

enum TaskResult
{
    Canceled,
    Succeeded,
    Failed,
};

}
}