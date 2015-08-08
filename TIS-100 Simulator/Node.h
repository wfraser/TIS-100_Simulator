#pragma once

enum class Neighbor
{
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    COUNT
};

inline Neighbor OppositeNeighbor(Neighbor n)
{
    switch (n)
    {
    case Neighbor::UP: return Neighbor::DOWN;
    case Neighbor::DOWN: return Neighbor::UP;
    case Neighbor::LEFT: return Neighbor::RIGHT;
    case Neighbor::RIGHT: return Neighbor::LEFT;
    default:
        throw std::exception("invalid neighbor");
    }
}

class IOChannel;

class INode
{
public:
    virtual void SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO) = 0;
    virtual void Initialize() = 0;
    virtual void Read() = 0;
    virtual void Compute() = 0;
    virtual void Write() = 0;
    virtual void WriteComplete() = 0;
    virtual void Step() = 0;

    static void Join(INode* nodeA, Neighbor directionOfBRelativeToA, INode* nodeB);
};