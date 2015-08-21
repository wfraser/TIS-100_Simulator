#pragma once

class VisualizationNode : public OutputBase
{
private:
    enum class State
    {
        ReadX,
        ReadY,
        ReadValues,
    };

    State m_state;
    Grid<int> m_grid;
    size_t m_xPosition;
    size_t m_yPosition;

public:
    VisualizationNode(size_t width, size_t height);
    virtual void Initialize() override;
    virtual void ReadData(int value) override;
};