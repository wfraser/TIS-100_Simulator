#include "pch.h"
#include "Node.h"
#include "OutputBase.h"
#include "OutputNode.h"
#include "IOChannel.h"

OutputNode::OutputNode()
{}

void OutputNode::Initialize()
{
    OutputBase::Initialize();
    Data.clear();
}

void OutputNode::ReadData(int value)
{
    Data.push_back(value);
}
