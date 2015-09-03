#pragma once

class InputNode : public INode
{
private:
    enum class State
    {
        Ready,
        Write,
        WriteComplete,
    };

private:
    std::vector<int> m_data;
    size_t m_position;
    State m_state;
    std::shared_ptr<IOChannel> m_spIO;
    Neighbor m_neighborDirection;

public:
    InputNode(const std::vector<int>& data);

    void SetData(std::vector<int>&& data);

    virtual void SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO);
    virtual void Initialize();
    virtual void Read();
    virtual void ReadComplete(int value);
    virtual void Compute();
    virtual void Write();
    virtual void WriteComplete();
    virtual void Step();
};