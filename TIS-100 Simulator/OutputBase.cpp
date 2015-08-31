#include "pch.h"
#include "Node.h"
#include "OutputBase.h"
#include "IOChannel.h"

OutputBase::OutputBase()
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
{}

void OutputBase::Read()
{
    if (m_spIO != nullptr)
    {
        int value;
        if (m_spIO->Read(this, &value))
        {
            ReadData(value);
        }
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