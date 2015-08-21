#include "pch.h"
#include "Node.h"
#include "OutputBase.h"
#include "IOChannel.h"

OutputBase::OutputBase()
    : m_state(State::Run)
{}

void OutputBase::SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO)
{
    if ((m_spIO != nullptr) && (m_neighborDirection != direction))
    {
        throw std::exception("OutputNode can only have one neighbor");
    }

    m_spIO = spIO;
    m_neighborDirection = direction;
}

void OutputBase::Initialize()
{
    m_state = State::Run;
}

void OutputBase::Read()
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

void OutputBase::ReadComplete(int value)
{
    if (m_state == State::Read)
    {
        m_state = State::Run;
        ReadData(value);
    }
    else
    {
        throw std::exception("unexpected ReadComplete");
    }
}

void OutputBase::Compute()
{
    // Nothing
}

void OutputBase::Write()
{
    // Nothing
}

void OutputBase::WriteComplete()
{
    // Nothing
}

void OutputBase::Step()
{
    // Nothing
}