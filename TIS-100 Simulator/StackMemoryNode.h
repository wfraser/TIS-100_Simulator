#pragma once

class StackMemoryNode : public INode
{
private:
    bool m_writeReady;
    std::shared_ptr<IOChannel> m_neighbors[static_cast<size_t>(Neighbor::COUNT)];
    std::vector<int> m_data;

public:
    StackMemoryNode();
    virtual void SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO);
    virtual void Initialize();

    virtual void Read();
    virtual void Compute();
    virtual void Write();
    virtual void WriteComplete();
    virtual void Step();
};