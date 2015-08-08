#include "pch.h"
#include "Node.h"
#include "IOChannel.h"

IOChannel::IOChannel(INode * a, INode * b)
    : m_nodeA(a)
    , m_nodeB(b)
    , m_sender(nullptr)
{
}

void IOChannel::Write(INode * sender, int value)
{
    m_sender = sender;
    m_value = value;
}

bool IOChannel::Read(INode * receiver, int * pValue)
{
    if ((m_sender != receiver) && (m_sender != nullptr))
    {
        *pValue = m_value;
        m_sender->WriteComplete();
        m_sender = nullptr;
        return true;
    }
    return false;
}