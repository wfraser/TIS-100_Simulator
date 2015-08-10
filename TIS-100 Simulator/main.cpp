#include "pch.h"
#include "Node.h"
#include "InputNode.h"
#include "OutputNode.h"
#include "ComputeNode.h"

#ifdef PARALLEL
#include <ppl.h>
#endif

static const size_t PuzzleInputSize = 39;

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

bool TestPuzzle(const Puzzle& puzzle, int* pNumCycles, int* pNodeCount, int* pInstructionCount)
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

    for (size_t i = 0; i < _countof(computeNodes); ++i)
    {
        int instructions = computeNodes[i].InstructionCount();
        if (instructions > 0)
        {
            (*pInstructionCount) += instructions;
            ++(*pNodeCount);
        }
    }

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

static std::vector<int> FunctionGenerator(std::function<bool(size_t, int*)> fn)
{
    std::vector<int> values;
    for (size_t i = 0; true; ++i)
    {
        int value;
        if (fn(i, &value))
            values.push_back(value);
        else
            break;
    }
    return values;
}

static std::vector<int> RandomGenerator(size_t count, int min, int max)
{
    std::default_random_engine engine;
    std::uniform_int_distribution<int> distribution(min, max);
    return FunctionGenerator([count, &distribution, &engine](size_t i, int* value) -> bool
    {
        if (i == count)
            return false;
        else
        {
            *value = distribution(engine);
            return true;
        }
    });
}

static std::vector<int> PuzzleInputSimpleGenerator(const Puzzle::IO& input, std::function<int(int)> fn)
{
    std::vector<int> values;
    for (int value : input.data)
    {
        values.push_back(fn(value));
    }
    return values;
}

int wmain(int argc, wchar_t** argv)
{
    int puzzleNumber;
    const wchar_t* saveFilePath;
    if (argc != 3)
    {
        std::cout << "usage: " << argv[0] << " <puzzle number> <save file>\n"
            "\n"
            "look for saves in "
            R"(%USERPROFILE%\Documents\my games\TIS-100\<random number>\save)"
            "\n";
        return -1;
    }
    else
    {
        if (0 == swscanf_s(argv[1], L"%u", &puzzleNumber))
        {
            std::cout << "invalid puzzle number\n";
        }
        saveFilePath = argv[2];
    }

    try
    {
        Puzzle puzzle;

        switch (puzzleNumber)
        {
        case 150:
            // Node arrangement:
            //
            //  I        I
            //  0  x  2  3
            //  4  x  6  x
            //  8  x 10 11
            //  O        O
            puzzle.badNodes = { 1,5,7,9 };
            puzzle.inputs.emplace_back(Puzzle::IO{ 0, Neighbor::UP, RandomGenerator(PuzzleInputSize, 10, 100) });
            puzzle.inputs.emplace_back(Puzzle::IO{ 3, Neighbor::UP, RandomGenerator(PuzzleInputSize, 10, 100) });
            puzzle.outputs.emplace_back(Puzzle::IO{ 8, Neighbor::DOWN, PuzzleInputSimpleGenerator(puzzle.inputs[0], [](int value) { return value; }) });
            puzzle.outputs.emplace_back(Puzzle::IO{ 11, Neighbor::DOWN, PuzzleInputSimpleGenerator(puzzle.inputs[1], [](int value) { return value; }) });
            break;

        case 10981:
            // Node arrangement:
            //
            //     I
            //  0  1  2  x
            //  4  5  6  7
            //  x  9 10 11
            //        O
            puzzle.badNodes = { 3, 8 };
            puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, RandomGenerator(PuzzleInputSize, 10, 100) });
            puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, PuzzleInputSimpleGenerator(puzzle.inputs[0], [](int value) { return value * 2; }) });
            break;

        default:
            std::cout << "Unknown puzzle " << puzzleNumber << ".\n";
            return -1;
        }


        ReadSaveFile(saveFilePath,
            puzzle.programs,
            puzzle.badNodes);

        int cycles = 0;
        int nodeCount = 0;
        int instructionCount = 0;

        bool success = TestPuzzle(puzzle, &cycles, &nodeCount, &instructionCount);

        std::cout << (success ? "success" : "failure") << " in "
            << cycles << " cycles, "
            << nodeCount << " nodes, "
            << instructionCount << " instructions.\n";
    }
    catch (std::exception ex)
    {
        const char* message = ex.what();
        std::cout << message << std::endl;
    }

    return 0;
}