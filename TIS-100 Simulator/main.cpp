#include "pch.h"
#include "Node.h"
#include "InputNode.h"
#include "OutputNode.h"
#include "ComputeNode.h"

#ifdef PARALLEL
#include <ppl.h>
#endif

void ReadSaveFile(const wchar_t* path, std::string programs[12], const std::set<int>& badNodes)
{
    std::ifstream file(path);
    std::string line;
    std::string program;
    int nodeNumber = -1;
    for (;;)
    {
        std::getline(file, line);
        if (file.eof() || (line.size() > 0 && line[0] == '@')) 
        {
            if (!program.empty())
            {
                programs[nodeNumber].swap(program);
                program.clear();
            }

            if (file.eof())
                break;

            // this assumes that the nodes are always written out in order.
            ++nodeNumber;

            if (badNodes.find(nodeNumber) != badNodes.end())
                nodeNumber++;
        }
        else
        {
            program += line + "\n";
        }
    }
}

class Puzzle
{
public:
    std::string programs[12];

    struct IO
    {
        int toNode;
        Neighbor direction;
        std::vector<int> data;
    };
    std::vector<IO> inputs, outputs;

    std::set<int> badNodes;
};

bool TestPuzzle(const Puzzle& puzzle, int* pNumCycles)
{
    ComputeNode computeNodes[12];

    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            INode* cur = &computeNodes[row * 4 + col];
            if (col < 3)
                INode::Join(cur, Neighbor::RIGHT, &computeNodes[row * 4 + col + 1]);
            if (row < 2)
                INode::Join(cur, Neighbor::DOWN, &computeNodes[(row + 1) * 4 + col]);
        }
    }

    std::vector<InputNode> inputNodes;
    std::vector<OutputNode> outputNodes;

    for (const Puzzle::IO& io : puzzle.inputs)
    {
        inputNodes.emplace_back(io.data);
        INode::Join(&computeNodes[io.toNode], io.direction, &inputNodes.back());
    }

    for (const Puzzle::IO& io : puzzle.outputs)
    {
        outputNodes.emplace_back();
        INode::Join(&computeNodes[io.toNode], io.direction, &outputNodes.back());
    }

    std::vector<INode*> nodes;

    for (size_t i = 0, n = _countof(computeNodes); i < n; i++)
    {
        ComputeNode& node = computeNodes[i];
        const std::string& program = puzzle.programs[i];
        if (!program.empty())
            node.Assemble(program);
        nodes.push_back(&node);
    }

    for (InputNode& node : inputNodes)
        nodes.push_back(&node);

    for (OutputNode& node : outputNodes)
        nodes.push_back(&node);

    for (INode* node : nodes)
        node->Initialize();

    *pNumCycles = 0;
    for (;;)
    {
        bool outputFinished = true;
        for (size_t i = 0, n = puzzle.outputs.size(); i < n; ++i)
        {
            const std::vector<int>& actual = outputNodes[i].Data;
            const std::vector<int>& expected = puzzle.outputs[i].data;

            if (!actual.empty() && (actual.back() != expected[actual.size() - 1]))
                return false;

            if (actual.size() != expected.size())
                outputFinished = false;
        }

        if (outputFinished)
        {
            return true;
        }

        ++(*pNumCycles);

#ifndef PARALLEL

        for (auto& node : nodes)
            node->Read();

        for (auto& node : nodes)
            node->Compute();

        for (auto& node : nodes)
            node->Write();

        for (auto& node : nodes)
            node->Step();

#else

        concurrency::parallel_for_each(nodes.begin(), nodes.end(), [](INode* node) {
            node->Read();
        });

        concurrency::parallel_for_each(nodes.begin(), nodes.end(), [](INode* node) {
            node->Compute();
        });

        concurrency::parallel_for_each(nodes.begin(), nodes.end(), [](INode* node) {
            node->Write();
        });

        concurrency::parallel_for_each(nodes.begin(), nodes.end(), [](INode* node) {
            node->Step();
        });
#endif
    }
}

int wmain(int argc, wchar_t** argv)
{
    const wchar_t* saveFilePath;
    if (argc != 2)
    {
        std::cout << "usage: " << argv[0] << " <save file>\n"
            "\n"
            "look for saves in "
            R"(%USERPROFILE%\Documents\my games\TIS-100\<random number>\save)"
            "\n";
        return -1;
    }
    else
    {
        saveFilePath = argv[1];
    }

    try
    {
        // hard-coded 00150 puzzle.
        // TODO: encode the rest
        // also, wrong number of inputs, so the cycle count doesn't match the game

        Puzzle puzzle00150;
        puzzle00150.badNodes = { 1,5,7,9 };
        puzzle00150.inputs.emplace_back(Puzzle::IO{ 0, Neighbor::UP, {1,2,3,4} });
        puzzle00150.inputs.emplace_back(Puzzle::IO{ 3, Neighbor::UP, {10,11,12,13} });
        puzzle00150.outputs.emplace_back(Puzzle::IO{ 8, Neighbor::DOWN, {1,2,3,4} });
        puzzle00150.outputs.emplace_back(Puzzle::IO{ 11, Neighbor::DOWN, {10,11,12,13} });

        // Node arrangement:
        //
        //  I        I
        //  0  x  2  3
        //  4  x  6  x
        //  8  x 10 11
        //  O        O

        ReadSaveFile(saveFilePath,
            puzzle00150.programs,
            puzzle00150.badNodes);

        int cycles = 0;

        bool success = TestPuzzle(puzzle00150, &cycles);

        std::cout << (success ? "success" : "failure") << " in " << cycles << " cycles.\n";
    }
    catch (std::exception ex)
    {
        const char* message = ex.what();
        std::cout << message << std::endl;
    }

    return 0;
}