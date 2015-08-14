#include "pch.h"
#include "Node.h"
#include "StackMemoryNode.h"
#include "IOChannel.h"

StackMemoryNode::StackMemoryNode()
{}

void StackMemoryNode::SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO)
{
    //TODO
}

void StackMemoryNode::Initialize()
{
    m_data.clear();
}

void StackMemoryNode::Read()
{
    //TODO
}

void StackMemoryNode::Compute()
{
    //TODO
}

void StackMemoryNode::Write()
{
    //TODO
}

void StackMemoryNode::WriteComplete()
{
    //TODO
}

void StackMemoryNode::Step()
{
    //TODO
}