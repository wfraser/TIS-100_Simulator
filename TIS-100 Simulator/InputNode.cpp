#include "pch.h"
#include "Node.h"
#include "InputNode.h"
#include "IOChannel.h"

InputNode::InputNode(const std::vector<int>& data)
    : m_data(data)
    , m_position(0)
    , m_state(State::Ready)
{}

void InputNode::SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO)
{
    if ((m_spIO != nullptr) && (m_neighborDirection != direction))
    {
        throw std::exception("InputNode can only have one neighbor.");
    }

    m_spIO = spIO;
    m_neighborDirection = direction;
}

void InputNode::Initialize()
{
    m_position = 0;
}

void InputNode::Read()
{
    // Nothing
}

void InputNode::ReadComplete(int /*value*/)
{
    throw std::exception("Unexpected ReadComplete on InputNode");
}

void InputNode::Compute()
{
    // Nothing
}

void InputNode::Write()
{
    switch (m_state)
    {
    case State::Ready:
        if (m_position < m_data.size())
        {
            m_state = State::Write;
            m_spIO->Write(this, m_data[m_position]);
        }
        break;
    case State::Write:
        break;
    }
}

void InputNode::WriteComplete()
{
    switch (m_state)
    {
    case State::Write:
        m_state = State::WriteComplete;
        break;
    case State::Ready:
    case State::WriteComplete:
        throw std::exception("unexpected WriteComplete");
    }
}

void InputNode::Step()
{
    switch (m_state)
    {
    case State::Ready:
    case State::Write:
        break;
    case State::WriteComplete:
        m_state = State::Ready;
        ++m_position;
        break;
    }
}