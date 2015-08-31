#include "pch.h"
#include "Node.h"
#include "StackMemoryNode.h"
#include "IOChannel.h"
#include <assert.h>

StackMemoryNode::StackMemoryNode()
    : m_writeReady(true)
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
    for (size_t i = 0; i < static_cast<size_t>(Neighbor::COUNT); ++i)
    {
        std::shared_ptr<IOChannel>& spIO = m_neighbors[i];
        if (spIO != nullptr)
        {
            int value;
            if (spIO->Read(this, &value))
            {
                m_data.push_back(value);
                for (const std::shared_ptr<IOChannel>& spIO : m_neighbors)
                {
                    if (spIO != nullptr)
                        spIO->CancelWrite(this);
                }

                m_writeReady = true;

                // Don't bother attempting any other reads.
                break;
            }
        }
    }
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