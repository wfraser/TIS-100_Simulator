#include "pch.h"
#include "Node.h"
#include "OutputBase.h"
#include "Grid.h"
#include "VisualizationNode.h"
#include "IOChannel.h"

VisualizationNode::VisualizationNode(size_t width, size_t height)
    : m_state(State::ReadX)
    , Grid(width, height)
    , m_xPosition(std::numeric_limits<size_t>::max())
    , m_yPosition(std::numeric_limits<size_t>::max())
{
}

static size_t Clamp(int value, size_t max)
{
    if (value < 0)
        return 0U;

    if (static_cast<size_t>(value) > max)
        return max;

    return static_cast<size_t>(value);
}

void VisualizationNode::Initialize()
{
    OutputBase::Initialize();
    Grid.Clear();
    m_state = State::ReadX;
}

void VisualizationNode::ReadData(int value)
{
    switch (m_state)
    {
    case State::ReadX:
        m_xPosition = Clamp(value, Grid.Width());
        m_state = State::ReadY;
        break;
    case State::ReadY:
        m_yPosition = Clamp(value, Grid.Height());
        m_state = State::ReadValues;
        break;
    case State::ReadValues:
        if (value < 0)
        {
            m_state = State::ReadX;
            m_xPosition = std::numeric_limits<size_t>::max();
            m_yPosition = std::numeric_limits<size_t>::max();
        }
        else if (m_xPosition < Grid.Width() && m_yPosition < Grid.Height())
        {
            int color = Clamp(value, 5);
            if (color == 5)
            {
                // Out-of-range values are treated as "black" or 0.
                color = 0;
            }

            Grid[{m_xPosition, m_yPosition}] = value;
            ++m_xPosition;
        }
        break;
    }
}
