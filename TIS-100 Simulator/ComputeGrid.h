#pragma once

template <int GridHeight, int GridWidth>
class ComputeGrid
{
private:
    typedef PuzzleBase<GridHeight * GridWidth> PuzzleType;

    std::vector<ComputeNode*> m_computeNodes;
    std::vector<StackMemoryNode*> m_stackNodes;
    std::vector<InputNode> m_inputNodes;
    std::vector<OutputNode> m_outputNodes;
    std::vector<VisualizationNode> m_vizNodes;

    std::unique_ptr<INode> m_grid[GridHeight * GridWidth];

    std::vector<INode*> m_allNodes;

public:
    ComputeGrid(const PuzzleType& puzzle)
    {
        for (int row = 0; row < GridHeight; ++row)
        {
            for (int col = 0; col < GridWidth; ++col)
            {
                int index = row * GridWidth + col;

                std::unique_ptr<INode>& spCurrentNode = m_grid[index];

                if (puzzle.stackNodes.find(index) != puzzle.stackNodes.end())
                {
                    auto pStackNode = new StackMemoryNode();
                    spCurrentNode.reset(pStackNode);
                    m_stackNodes.push_back(pStackNode);
                }
                else
                {
                    auto pComputeNode = new ComputeNode();
                    pComputeNode->Assemble(puzzle.programs[index]);
                    spCurrentNode.reset(pComputeNode);
                    m_computeNodes.push_back(pComputeNode);
                }

                spCurrentNode->NodeId = index;

                if (col > 0)
                    INode::Join(m_grid[index - 1].get(), Neighbor::RIGHT, spCurrentNode.get());
                if (row > 0)
                    INode::Join(m_grid[index - GridWidth].get(), Neighbor::DOWN, spCurrentNode.get());
            }
        }

        m_inputNodes.reserve(puzzle.inputs.size());
        for (const PuzzleType::IO& io : puzzle.inputs)
        {
            m_inputNodes.emplace_back(io.data);
            INode* node = &m_inputNodes.back();
            INode::Join(m_grid[io.toNode].get(), io.direction, node);
        }

        m_outputNodes.reserve(puzzle.outputs.size());
        for (const PuzzleType::IO& io : puzzle.outputs)
        {
            m_outputNodes.emplace_back();
            INode* node = &m_outputNodes.back();
            INode::Join(m_grid[io.toNode].get(), io.direction, node);
        }

        m_vizNodes.reserve(puzzle.visualization.size());
        for (const PuzzleType::IO& io : puzzle.visualization)
        {
            m_vizNodes.emplace_back(puzzle.visualizationWidth, puzzle.visualizationHeight);
            INode* node = &m_vizNodes.back();
            INode::Join(m_grid[io.toNode].get(), io.direction, node);
        }
    }

    void GetStats(int* pComputeNodeCount, int* pInstructionCount)
    {
        for (const ComputeNode* node : m_computeNodes)
        {
            int count = node->InstructionCount();
            if (count > 0)
            {
                ++(*pComputeNodeCount);
                *pInstructionCount += count;
            }
        }
    }

    void Step()
    {
        for (auto& node : m_allNodes)
            node->Read();

        for (auto& node : m_allNodes)
            node->Compute();

        for (auto& node : m_allNodes)
            node->Write();

        for (auto& node : m_allNodes)
            node->Step();
    }

    bool IsFinished(const PuzzleType& puzzle, bool* pIsFailure)
    {
        bool outputFinished = true;

        for (size_t i = 0, n = puzzle.outputs.size(); i < n; ++i)
        {
            const std::vector<int>& actual = m_outputNodes[i].Data;
            const std::vector<int>& expected = puzzle.outputs[i].data;

            if (!actual.empty() && (actual.back() != expected[actual.size() - 1]))
            {
                *pIsFailure = true;
                return true;
            }

            if (actual.size() != expected.size())
                outputFinished = false;
        }

        bool vizMatch = true;
        for (size_t i = 0, n = puzzle.visualization.size(); i < n; ++i)
        {
            VisualizationNode& vizNode = m_vizNodes[i];

            for (size_t j = 0, n = vizNode.Grid.Width() * vizNode.Grid.Height(); j < n; ++j)
            {
                int expected = 0;
                // allow under-sizing the expected vector
                if (puzzle.visualization[i].data.size() > j)
                    expected = puzzle.visualization[i].data[j];

                if (vizNode.Grid[j] != expected)
                {
                    vizMatch = false;
                    break;
                }
            }
            if (!vizMatch)
                break;
        }

        if (outputFinished && vizMatch)
        {
            *pIsFailure = false;
            return true;
        }
        else
        {
            return false;
        }
    }

    void Initialize()
    {
        m_allNodes.clear();

        for (INode& node : m_inputNodes)
        {
            node.Initialize();
            m_allNodes.push_back(&node);
        }

        for (INode& node : m_outputNodes)
        {
            node.Initialize();
            m_allNodes.push_back(&node);
        }

        for (INode& node : m_vizNodes)
        {
            node.Initialize();
            m_allNodes.push_back(&node);
        }

        for (ComputeNode* node : m_computeNodes)
        {
            if (node->InstructionCount() > 0)
            {
                node->Initialize();
                m_allNodes.push_back(node);
            }
        }

        for (StackMemoryNode* node : m_stackNodes)
        {
            node->Initialize();
            m_allNodes.push_back(node);
        }
    }
};