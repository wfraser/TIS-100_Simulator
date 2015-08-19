#include "pch.h"
#include "Node.h"
#include "IOChannel.h"
#include <assert.h>

#define BREAK_ON_CONFLICT

#ifdef BREAK_ON_CONFLICT
#include "ComputeNode.h"
#include "StackMemoryNode.h"
#endif

IOChannel::IOChannel(INode * a, INode * b)
    : m_a(Endpoint{ a, false, false, 0 })
    , m_b(Endpoint{ b, false, false, 0 })
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

#ifdef BREAK_ON_CONFLICT
    assert(!sender->writePending);
    if ((dynamic_cast<StackMemoryNode*>(senderNode) == nullptr)
        && sender->readPending)
    {
        // Can't have both a read and a write pending, unless it's a StackMemoryNode.
        __debugbreak();
    }
#endif

    if (receiver->readPending)
    {
        receiver->node->ReadComplete(value);
        receiver->readPending = false;
        senderNode->WriteComplete();
    }
    else
    {
        sender->writePending = true;
        sender->sentValue = value;
    }
}

void IOChannel::Read(INode * receiverNode)
{
    Endpoint* receiver;
    Endpoint* sender;
    GetEndpoints(receiverNode, &receiver, &sender);

#ifdef BREAK_ON_CONFLICT
    assert(!receiver->readPending);
    if ((dynamic_cast<StackMemoryNode*>(receiverNode) == nullptr)
        && receiver->writePending)
    {
        // Can't have both a read and a write pending, unless it's a StackMemoryNode.
        __debugbreak();
    }
#endif

    if (sender->writePending)
    {
        receiver->node->ReadComplete(sender->sentValue);
        sender->node->WriteComplete();
        sender->writePending = false;
    }
    else
    {
        receiver->readPending = true;
    }
}

void IOChannel::CancelRead(INode* receiverNode)
{
    Endpoint* receiver;
    Endpoint* sender;
    GetEndpoints(receiverNode, &receiver, &sender);
    receiver->readPending = false;
}

void IOChannel::CancelWrite(INode* senderNode)
{
    Endpoint* receiver;
    Endpoint* sender;
    GetEndpoints(senderNode, &sender, &receiver);
    sender->writePending = false;
}