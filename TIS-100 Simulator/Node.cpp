#include "pch.h"
#include "Node.h"
#include "IOChannel.h"

void INode::Join(INode* nodeA, Neighbor directionOfBRelativeToA, INode* nodeB)
{
    auto channel = std::make_shared<IOChannel>(nodeA, nodeB);
    nodeA->SetNeighbor(directionOfBRelativeToA, channel);
    nodeB->SetNeighbor(OppositeNeighbor(directionOfBRelativeToA), channel);
}
