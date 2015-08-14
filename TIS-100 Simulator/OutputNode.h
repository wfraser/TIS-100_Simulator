#pragma once

class OutputNode : public INode
{
private:
    enum class State
    {
        Run,
        Read
    };

    State m_state;
    std::shared_ptr<IOChannel> m_spIO;
    Neighbor m_neighborDirection;

public:
    std::vector<int> Data;

    OutputNode();

    virtual void SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO);
    virtual void Initialize();
    virtual void Read();
    virtual void ReadComplete(int value);
    virtual void Compute();
    virtual void Write();
    virtual void WriteComplete();
    virtual void Step();
};