#pragma once

class OutputBase : public INode
{
private:
    std::shared_ptr<IOChannel> m_spIO;
    Neighbor m_neighborDirection;

public:
    OutputBase();

    virtual void SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO);
    virtual void Read();
    virtual void Compute();
    virtual void Write();
    virtual void WriteComplete();
    virtual void Step();

    // Subclasses should override these.
    virtual void Initialize();
    virtual void ReadData(int value) = 0;
};