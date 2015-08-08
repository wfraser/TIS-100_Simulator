#pragma once
#include "pch.h"
#include "Node.h"
#include "OutputNode.h"
#include "IOChannel.h"

OutputNode::OutputNode()
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
    int value;
    if ((m_spIO != nullptr) && m_spIO->Read(this, &value))
    {
        Data.push_back(value);
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