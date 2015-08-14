#include "pch.h"
#include "Node.h"
#include "IOChannel.h"

#define BREAK_ON_CONFLICT

IOChannel::IOChannel(INode * a, INode * b)
    : m_nodeA(a)
    , m_nodeB(b)
{
}

void IOChannel::Write(INode * sender, int value)
{
#ifdef BREAK_ON_CONFLICT
    if (m_sender != nullptr && m_sender != sender)
    {
        // Write conflict.
        __debugbreak();
    }
#endif

    if (m_receiver != nullptr)
    {
        // Receiver is waiting. Send immediately.
        m_receiver->ReadComplete(value);
        m_receiver = nullptr;
        sender->WriteComplete();
    }
    else
    {
        // No receiver waiting. Sender will have to block.
        // Save the sender so we can notify on read.
        m_sender = sender;
        m_value = value;
    }
}

void IOChannel::Read(INode * receiver)
{
    if ((m_sender != receiver) && (m_sender != nullptr))
    {
        // Value is already available. Send the completions right away.
        m_sender->WriteComplete();
        m_sender = nullptr;
        receiver->ReadComplete(m_value);
    }
    else
    {
        // Value is not available; reader will have to block.
        if (m_receiver == nullptr)
        {
            // Save the receiver so we can notify on write.
            m_receiver = receiver;
        }
#ifdef BREAK_ON_CONFLICT
        else
        {
            // Read conflict.
            __debugbreak();
        }
#endif
    }
}

void IOChannel::CancelRead(INode* receiver)
{
    if (m_receiver == receiver)
    {
        m_receiver = nullptr;
    }
}

void IOChannel::CancelWrite(INode* sender)
{
    if (m_sender == sender)
    {
        m_sender = nullptr;
    }
}