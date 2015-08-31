#include "pch.h"
#include "Node.h"
#include "IOChannel.h"
#include <assert.h>

IOChannel::IOChannel(INode * a, INode * b)
    : m_a(Endpoint{ a, false, 0 })
    , m_b(Endpoint{ b, false, 0 })
{
}

void IOChannel::GetEndpoints(INode* node, Endpoint** ppThatEndpoint, Endpoint** ppOtherEndpoint)
{
    *ppThatEndpoint = &m_a;
    *ppOtherEndpoint = &m_b;
    if (node == m_b.node)
        std::swap(*ppThatEndpoint, *ppOtherEndpoint);
    else
        assert(node == m_a.node);
}

void IOChannel::Write(INode * senderNode, int value)
{
    Endpoint* receiver;
    Endpoint* sender;
    GetEndpoints(senderNode, &sender, &receiver);

    sender->writePending = true;
    sender->sentValue = value;
}

bool IOChannel::Read(INode * receiverNode, int* pValue)
{
    Endpoint* receiver;
    Endpoint* sender;
    GetEndpoints(receiverNode, &receiver, &sender);

    if (sender->writePending)
    {
        *pValue = sender->sentValue;
        sender->writePending = false;
        sender->node->WriteComplete();
        return true;
    }
    else
    {
        return false;
    }
}

void IOChannel::CancelWrite(INode* senderNode)
{
    Endpoint* receiver;
    Endpoint* sender;
    GetEndpoints(senderNode, &sender, &receiver);
    sender->writePending = false;
}