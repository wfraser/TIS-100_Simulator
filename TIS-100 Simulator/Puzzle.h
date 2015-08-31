#pragma once

template <int NodeCount>
class PuzzleBase
{
public:
    // Assembly code text for the ComputeNodes.
    // There needs to be a corresponding ComputeNode at the same index, otherwise the program will
    // not be applied.
    std::string programs[NodeCount];

    struct IO
    {
        // Node to which the input/output is connected.
        int toNode;

        // The direction of the input/output relative to the node specified by toNode.
        Neighbor direction;

        // For inputs, the data to feed as input.
        // For outputs, the expected data used for verifying the program.
        std::vector<int> data;
    };

    // Inputs and outputs.
    std::vector<IO> inputs, outputs, visualization;

    int visualizationWidth, visualizationHeight;

    // Indices of bad (non-working) nodes.
    std::set<int> badNodes;

    // Indices of stack memory nodes.
    std::set<int> stackNodes;
};