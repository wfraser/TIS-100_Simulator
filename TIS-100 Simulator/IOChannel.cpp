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
    // Unless it's a StackMemoryNode:
    if (dynamic_cast<StackMemoryNode*>(senderNode) == nullptr)
    {
        // Can't have both a read and a write pending.
        assert(!sender->readPending);

        // Shouldn't already have a write pending.
        assert(!sender->writePending);
    }
#endif

    if (receiver->readPending)
    {
        receiver->readPending = false;
        receiver->node->ReadComplete(value);
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
    
    if (dynamic_cast<StackMemoryNode*>(receiverNode) == nullptr)
    {
        // Can't have both a read and a write pending.
        assert(!receiver->writePending);

        // Shouldn't already have a read pending.
        assert(!receiver->readPending);
    }
#endif

    if (sender->writePending)
    {
        receiver->node->ReadComplete(sender->sentValue);
        sender->writePending = false;
        sender->node->WriteComplete();
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