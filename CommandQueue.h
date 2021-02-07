#pragma once

namespace DialogsSync
{
class Command
{
public:

    enum cmdtype : uint32_t
    {
        cmdNothing = 0,
        cmdClose = 1,
        cmdScrollTo = 2,

        cmdTotal = 3
    };

    Command(const cmdtype acmdtype) : _type(acmdtype) {}

    cmdtype type()const noexcept { return _type; }

public:
    Gdiplus::PointF _imageOrigin = { 0., 0. };// we are drawing the image from this point. So for image 10 on 10  we can draw 1 pixel from 9:9

private:
    cmdtype _type = cmdNothing;
};

extern Command commands[Command::cmdTotal];

class CommandQueue
{
public:

    Command* next() noexcept; ///@brief get next command to execute
    void release() noexcept;  ///@brief release command from queue
    void add(Command* acmd);  ///@brief add comand to execute
protected:
    std::mutex _guard;
    std::queue<Command*> _commands;
};

template <typename T>
class CommandSequence
{
    CommandSequence& operator=(const CommandSequence&) = delete;
    CommandSequence&& operator=(CommandSequence&&) = delete;
    CommandSequence(const CommandSequence&) = delete;
    CommandSequence(CommandSequence&&) = delete;

public:
    typedef std::shared_ptr<T> TCommand;
    CommandSequence() {}

    bool next(TCommand& ret) noexcept ///@brief get next command to execute
    {
        std::lock_guard<std::mutex> guard(_guard);
        if (!_commands.empty())
        {
            std::swap(ret, _commands.front());
            _commands.pop();
            return true;
        }
        ret.reset();
        return false;
    }
    void add(TCommand&& acmd)  ///@brief add comand to execute
    {
        std::lock_guard<std::mutex> guard(_guard);
        _commands.emplace(acmd);
    }

protected:
    std::mutex _guard;
    std::queue<TCommand> _commands;
};

} // namespace DialogsSync



