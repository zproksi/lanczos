#include "stdafx.h"
#include "CommandQueue.h"

namespace DialogsSync
{
Command commands[Command::cmdTotal] =
{
    {Command::cmdNothing},
    {Command::cmdClose},
    {Command::cmdScrollTo}
};

Command* CommandQueue::next() noexcept
{
    std::lock_guard<std::mutex> guard(_guard);
    if (_commands.empty())
        return nullptr;
    return _commands.front();
}

void CommandQueue::release() noexcept
{
    std::lock_guard<std::mutex> guard(_guard);
    if (!_commands.empty())
       _commands.pop();
}
void CommandQueue::add(Command* acmd)
{
    std::lock_guard<std::mutex> guard(_guard);
        _commands.push(acmd);
}

}