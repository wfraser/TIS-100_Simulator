#include "pch.h"
#include "Node.h"
#include "StackMemoryNode.h"
#include "IOChannel.h"
#include <assert.h>

StackMemoryNode::StackMemoryNode()
    : m_readReady(true)
    , m_writeReady(true)
    , m_neighbors()
{}

void StackMemoryNode::SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO)
{
    m_neighbors[static_cast<size_t>(direction)] = spIO;
}

void StackMemoryNode::Initialize()
{
    m_data.clear();
}

void StackMemoryNode::Read()
{
    if (!m_readReady)
        return;
    m_readReady = false;

    for (size_t i = 0; i < static_cast<size_t>(Neighbor::COUNT); ++i)
    {
        std::shared_ptr<IOChannel>& spIO = m_neighbors[i];
        if (spIO != nullptr)
        {
            spIO->Read(this);

            if (m_readReady)
            {
                // A read succeeded; don't bother attempting any others.
                break;
            }
        }
    }
}

void StackMemoryNode::ReadComplete(int value)
{
    assert(!m_readReady);

    m_data.push_back(value);
    for (size_t i = 0; i < static_cast<size_t>(Neighbor::COUNT); ++i)
    {
        std::shared_ptr<IOChannel>& spIO = m_neighbors[i];
        if (spIO != nullptr)
        {
            spIO->CancelRead(this);
            spIO->CancelWrite(this);
        }
    }

    m_readReady = true;
    m_writeReady = true;
}

void StackMemoryNode::Compute()
{
    // Nothing.
}

void StackMemoryNode::Write()
{
    if (!m_writeReady || m_data.empty())
        return;
    m_writeReady = false;

    int value = m_data.back();
    for (size_t i = 0; i < static_cast<size_t>(Neighbor::COUNT); ++i)
    {
        std::shared_ptr<IOChannel>& spIO = m_neighbors[i];
        if (spIO != nullptr)
        {
            spIO->Write(this, value);

            if (m_writeReady)
            {
                // A write succeeded. Don't bother attempting any others.
            }
        }
    }
}

void StackMemoryNode::WriteComplete()
{
    assert(!m_writeReady);

    m_data.pop_back();
    for (size_t i = 0; i < static_cast<size_t>(Neighbor::COUNT); ++i)
    {
        std::shared_ptr<IOChannel>& spIO = m_neighbors[i];
        if (spIO != nullptr)
        {
            spIO->CancelWrite(this);
        }
    }

    m_writeReady = true;
}

void StackMemoryNode::Step()
{
    // Nothing.
}