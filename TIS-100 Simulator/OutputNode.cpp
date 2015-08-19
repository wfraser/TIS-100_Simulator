#pragma once
#include "pch.h"
#include "Node.h"
#include "OutputNode.h"
#include "IOChannel.h"

OutputNode::OutputNode()
    : m_state(State::Run)
{}

void OutputNode::SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO)
{
    if ((m_spIO != nullptr) && (m_neighborDirection != direction))
    {
        throw std::exception("OutputNode can only have one neighbor");
    }

    m_spIO = spIO;
    m_neighborDirection = direction;
}

void OutputNode::Initialize()
{
    Data.clear();
}

void OutputNode::Read()
{
    if (m_state == State::Run)
    {
        m_state = State::Read;
        if (m_spIO != nullptr)
        {
            m_spIO->Read(this);
        }
    }
}

void OutputNode::ReadComplete(int value)
{
    if (m_state == State::Read)
    {
        Data.push_back(value);
        m_state = State::Run;
    }
    else
    {
        throw std::exception("unexpected ReadComplete");
    }
}

void OutputNode::Compute()
{
    // Nothing
}

void OutputNode::Write()
{
    // Nothing
}

void OutputNode::WriteComplete()
{
    // Nothing
}

void OutputNode::Step()
{
    // Nothing
}