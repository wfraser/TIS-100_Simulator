#include "pch.h"
#include "Node.h"
#include "InputNode.h"
#include "OutputNode.h"
#include "ComputeNode.h"
#include "StackMemoryNode.h"

static const size_t PuzzleInputSize = 39;
static std::default_random_engine g_RandomEngine;

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

            while (badNodes.find(nodeNumber) != badNodes.end())
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
    std::set<int> stackNodes;
};

bool TestPuzzle(const Puzzle& puzzle, int* pNumCycles, int* pNodeCount, int* pInstructionCount)
{
    std::vector<ComputeNode> computeNodes;
    std::vector<StackMemoryNode> stackNodes;
    std::vector<INode*> nodeGrid(12);

    // Reserve enough space so that the elements won't ever get relocated.
    computeNodes.reserve(12);
    stackNodes.reserve(12);

    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            int index = row * 4 + col;

            INode** cur = &nodeGrid[index];

            if (puzzle.stackNodes.find(index) != puzzle.stackNodes.end())
            {
                stackNodes.emplace_back();
                *cur = &stackNodes.back();
            }
            else
            {
                computeNodes.emplace_back();
                *cur = &computeNodes.back();
            }

            if (col > 0)
                INode::Join(nodeGrid[index - 1], Neighbor::RIGHT, *cur);
            if (row > 0)
                INode::Join(nodeGrid[index - 4], Neighbor::DOWN, *cur);
        }
    }

    std::vector<InputNode> inputNodes;
    std::vector<OutputNode> outputNodes;

    inputNodes.reserve(puzzle.inputs.size());
    for (const Puzzle::IO& io : puzzle.inputs)
    {
        inputNodes.emplace_back(io.data);
        INode::Join(nodeGrid[io.toNode], io.direction, &inputNodes.back());
    }

    outputNodes.reserve(puzzle.outputs.size());
    for (const Puzzle::IO& io : puzzle.outputs)
    {
        outputNodes.emplace_back();
        INode::Join(nodeGrid[io.toNode], io.direction, &outputNodes.back());
    }

    std::vector<INode*> nodes;

    for (size_t i = 0, n = computeNodes.size(); i < n; i++)
    {
        ComputeNode& node = computeNodes[i];

        node.Assemble(puzzle.programs[i]);

        int instructions = node.InstructionCount();
        if (instructions > 0)
        {
            // Only start and add the compute node if it has a program to execute.
            node.Initialize();
            nodes.push_back(&node);

            (*pInstructionCount) += instructions;
            ++(*pNodeCount);
        }
    }

    for (InputNode& node : inputNodes)
    {
        node.Initialize();
        nodes.push_back(&node);
    }

    for (OutputNode& node : outputNodes)
    {
        node.Initialize();
        nodes.push_back(&node);
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

        for (auto& node : nodes)
            node->Read();

        for (auto& node : nodes)
            node->Compute();

        for (auto& node : nodes)
            node->Write();

        for (auto& node : nodes)
            node->Step();
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
    std::uniform_int_distribution<int> distribution(min, max);
    return FunctionGenerator([count, &distribution](size_t i, int* value) -> bool
    {
        if (i == count)
            return false;
        else
        {
            *value = distribution(g_RandomEngine);
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

bool Test(int puzzleNumber, const wchar_t* saveFilePath, int* pCycleCount, int *pNodeCount, int* pInstructionCount)
{
    Puzzle puzzle;

    switch (puzzleNumber)
    {
    case -1: // Connectivity check. Hardcoded program; ignores the save file path.
        puzzle.badNodes = {};

        puzzle.programs[0] = "MOV RIGHT,DOWN";
        puzzle.programs[1] = "MOV UP,ACC\nMOV ACC,LEFT\nMOV ACC,RIGHT\nMOV ACC,DOWN";
        puzzle.programs[2] = "MOV LEFT,ACC\nMOV ACC,RIGHT\nMOV ACC,DOWN";
        puzzle.programs[3] = "MOV LEFT,DOWN";

        puzzle.programs[4] = "MOV UP,ACC\nMOV ACC,RIGHT\nMOV ACC,DOWN";
        puzzle.programs[5] = "MOV UP,ACC\nADD LEFT\nMOV ACC,RIGHT\nMOV ACC,DOWN";
        puzzle.programs[6] = "MOV UP,ACC\nADD LEFT\nMOV ACC,RIGHT\nMOV ACC,DOWN";
        puzzle.programs[7] = "MOV UP,ACC\nADD LEFT\nMOV ACC,DOWN";

        puzzle.programs[8] = "MOV UP,RIGHT";
        puzzle.programs[9] = "MOV UP,ACC\nADD LEFT\nMOV ACC,RIGHT\n";
        puzzle.programs[10] = "MOV UP,ACC\nADD RIGHT\nADD LEFT\nMOV ACC,DOWN";
        puzzle.programs[11] = "MOV UP,LEFT";

        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, {1, 2, 3, 4} });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, { 10, 20, 30, 40 } });

        return TestPuzzle(puzzle, pCycleCount, pNodeCount, pInstructionCount);

    case 150:   // Self-Test Diagnostic
        // Node arrangement:
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

    case 10981: // Signal Amplifier
        // Node arrangement:
        //     I
        //  0  1  2  x
        //  4  5  6  7
        //  x  9 10 11
        //        O
        puzzle.badNodes = { 3, 8 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, RandomGenerator(PuzzleInputSize, 10, 100) });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, PuzzleInputSimpleGenerator(puzzle.inputs[0], [](int value) { return value * 2; }) });
        break;

    case 20176: // Differential Converter
        // Node arrangement:
        //     I  I
        //  0  1  2  3
        //  4  5  6  x
        //  8  9 10 11
        //     O  O
        puzzle.badNodes = { 7 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, RandomGenerator(PuzzleInputSize, 10, 100) });
        puzzle.inputs.push_back(Puzzle::IO{ 2, Neighbor::UP, RandomGenerator(PuzzleInputSize, 10, 100) });
        puzzle.outputs.push_back(Puzzle::IO{ 9, Neighbor::DOWN, FunctionGenerator([&puzzle](size_t i, int* value)->bool
        {
            if (i < puzzle.inputs[0].data.size())
            {
                *value = puzzle.inputs[0].data[i] - puzzle.inputs[1].data[i];
                return true;
            }
            else
                return false;
        }) });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, FunctionGenerator([&puzzle](size_t i, int* value)->bool
        {
            if (i < puzzle.inputs[0].data.size())
            {
                *value = puzzle.inputs[1].data[i] - puzzle.inputs[0].data[i];
                return true;
            }
            else
                return false;
        }) });
        break;

    case 21340: // Signal Comparator
        // Node arrangement:
        //  I
        //  0  1  2  3
        //  4  x  x  x
        //  8  9 10 11
        //     O  O  O
        puzzle.badNodes = { 5, 6, 7 };
        puzzle.inputs.push_back(Puzzle::IO{ 0, Neighbor::UP, RandomGenerator(PuzzleInputSize, -2, 2) });
        puzzle.outputs.push_back(Puzzle::IO{ 9, Neighbor::DOWN, PuzzleInputSimpleGenerator(puzzle.inputs[0], [](int input)->int {
            return (input > 0) ? 1 : 0;
        }) });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, PuzzleInputSimpleGenerator(puzzle.inputs[0], [](int input)->int {
            return (input == 0) ? 1 : 0;
        }) });
        puzzle.outputs.push_back(Puzzle::IO{ 11, Neighbor::DOWN, PuzzleInputSimpleGenerator(puzzle.inputs[0], [](int input)->int {
            return (input < 0) ? 1 : 0;
        }) });
        break;

    case 22280: // Signal Multiplexer
        // Node arrangement:
        //     I  I  I
        //  0  1  2  3
        //  4  5  6  7
        //  x  9 10 11
        //        O
        puzzle.badNodes = { 8 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, RandomGenerator(PuzzleInputSize, -30, 0) });
        puzzle.inputs.push_back(Puzzle::IO{ 2, Neighbor::UP, RandomGenerator(PuzzleInputSize, -1, 1) });
        puzzle.inputs.push_back(Puzzle::IO{ 3, Neighbor::UP, RandomGenerator(PuzzleInputSize, 0, 30) });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, FunctionGenerator([&puzzle](size_t i, int* value)->bool {
            if (i < PuzzleInputSize)
            {
                switch (puzzle.inputs[1].data[i])
                {
                case -1:
                    *value = puzzle.inputs[0].data[i];
                    break;
                case 0:
                    *value = puzzle.inputs[0].data[i] + puzzle.inputs[2].data[i];
                    break;
                case 1:
                    *value = puzzle.inputs[2].data[i];
                    break;
                default:
                    throw std::exception("invalid input");
                }
                return true;
            }
            else
                return false;
        }) });
        break;

    case 30647: // Sequence Generator
        // Node arrangement:
        //     I  I
        //  0  1  2  3
        //  4  5  6  7
        //  8  x 10 11
        //        O
        puzzle.badNodes = { 9 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, RandomGenerator(PuzzleInputSize / 3, 10, 100) });
        puzzle.inputs.push_back(Puzzle::IO{ 2, Neighbor::UP, RandomGenerator(PuzzleInputSize / 3, 10, 100) });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, FunctionGenerator([&puzzle](size_t i, int* value)->bool {
            if (i / 3 >= puzzle.inputs[0].data.size())
                return false;

            switch (i % 3)
            {
            case 0:
                *value = std::min(puzzle.inputs[0].data[i / 3], puzzle.inputs[1].data[i / 3]);
                break;
            case 1:
                *value = std::max(puzzle.inputs[0].data[i / 3], puzzle.inputs[1].data[i / 3]);
                break;
            case 2:
                *value = 0;
            }
            return true;
        }) });
        break;

    case 31904: // Sequence Counter
        // Node arrangement:
        //     I
        //  0  1  2  x
        //  4  5  6  7
        //  8  9 10 11
        //     O  O
        puzzle.badNodes = { 3 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, {} });
        puzzle.outputs.push_back(Puzzle::IO{ 9, Neighbor::DOWN, {0} });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, {0} });
        for (size_t i = 0; i < PuzzleInputSize; ++i)
        {
            if (0 == std::uniform_int_distribution<int>(0, 5)(g_RandomEngine))
            {
                puzzle.inputs[0].data.push_back(0);
                puzzle.outputs[0].data.push_back(0);
                puzzle.outputs[1].data.push_back(0);
            }
            else
            {
                int value = std::uniform_int_distribution<int>(10, 100)(g_RandomEngine);
                puzzle.inputs[0].data.push_back(value);
                puzzle.outputs[0].data.back() += value;
                puzzle.outputs[1].data.back() += 1;
            }
        }
        puzzle.outputs[0].data.pop_back();
        puzzle.outputs[1].data.pop_back();
        break;

    case 32050: // Signal Edge Detector
        // Node arrangement:
        //     I
        //  0  1  2  3
        //  4  5  6  7
        //  x  9 10 11
        //        O
        puzzle.badNodes = { 8 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, RandomGenerator(PuzzleInputSize, -20, 40) });
        puzzle.inputs[0].data[0] = 0;   // alter the first to be zero
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, FunctionGenerator([&puzzle](size_t i, int* value)->bool {
            if (i == 0)
            {
                *value = 0;
                return true;
            }
            else if (i < puzzle.inputs[0].data.size())
            {
                int a = puzzle.inputs[0].data[i - 1];
                int b = puzzle.inputs[0].data[i];
                *value = (std::abs(a - b) >= 10) ? 1 : 0;
                return true;
            }
            else
            {
                return false;
            }

        }) });
        break;

    case 33762: // Interrupt Handler
        // Node arrangement:
        //  I  I  I  I
        //  0  1  2  3
        //  4  5  6  7
        //  x  9 10 11
        //        O
        puzzle.badNodes = { 8 };
        puzzle.inputs.push_back(Puzzle::IO{ 0, Neighbor::UP, {} });
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, {} });
        puzzle.inputs.push_back(Puzzle::IO{ 2, Neighbor::UP, {} });
        puzzle.inputs.push_back(Puzzle::IO{ 3, Neighbor::UP, {} });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, {} });
        for (size_t i = 0, which = 0; i < PuzzleInputSize; ++i)
        {
            for (size_t j = 0; j < 4; ++j)
            {
                int value = 0;
                if (i > 0)
                {
                    if (j + 1 == which)
                    {
                        // This input is selected to change.
                        if (puzzle.inputs[j].data.back() == 1)
                        {
                            // This input has a high value; set it low.
                            value = 0;

                            // Output should be 0 because no positive edge happened.
                            which = 0;
                        }
                        else
                        {
                            // Positive edge.
                            value = 1;
                        }
                    }
                    else
                    {
                        // Channel keeps its last value.
                        value = puzzle.inputs[j].data.back();
                    }
                }

                puzzle.inputs[j].data.push_back(value);
            }

            puzzle.outputs[0].data.push_back(which);

            which = std::uniform_int_distribution<int>(0, 4)(g_RandomEngine);
        }

        // Verify that the above generator adheres to the invariant:
        // no two signals can change on the same cycle.
        for (size_t i = 1; i < PuzzleInputSize; ++i)
        {
            bool posEdge = false;
            int change = 0;

            for (size_t j = 0; j < 4; ++j)
            {
                int prev = puzzle.inputs[j].data[i - 1];
                int curr = puzzle.inputs[j].data[i];

                if (prev != curr)
                {
                    if (change != 0)
                    {
                        __debugbreak();
                    }
                    change = j + 1;

                    if (curr == 1)
                        posEdge = true;
                }
            }

            if (puzzle.outputs[0].data[i] != (posEdge ? change : 0))
                __debugbreak();
        }
        break;

    case 40196: // Signal Pattern Detector
        // Node arrangement:
        //     I
        //  0  1  2  x
        //  4  5  6  7
        //  8  9 10 11
        //        O
        puzzle.badNodes = { 3 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, {1} });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, {0} });
        for (size_t i = 1, zeroes = 0; i < PuzzleInputSize; ++i)
        {
            if (0 == std::uniform_int_distribution<int>(0, 3)(g_RandomEngine))
            {
                puzzle.inputs[0].data.push_back(std::uniform_int_distribution<int>(1, 30)(g_RandomEngine));
                puzzle.outputs[0].data.push_back(0);
                zeroes = 0;
            }
            else
            {
                puzzle.inputs[0].data.push_back(0);
                puzzle.outputs[0].data.push_back((++zeroes == 3) ? (--zeroes, 1) : 0);
            }
        }
        break;

    case 41427: // Sequence Peak Detector
        // Node arrangement:
        //     I
        //  0  1  2  3
        //  4  5  6  x
        //  8  9 10 11
        //     O  O
        puzzle.badNodes = { 7 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, {} });
        puzzle.outputs.push_back(Puzzle::IO{ 9, Neighbor::DOWN, {999} });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, {0} });
        for (size_t i = 0; i < PuzzleInputSize; ++i)
        {
            if ((i > 0)
                && (puzzle.inputs[0].data.back() != 0)
                && ((i == PuzzleInputSize - 1)
                    || (0 == std::uniform_int_distribution<int>(0, 5)(g_RandomEngine))))
            {
                puzzle.inputs[0].data.push_back(0);

                if (i != PuzzleInputSize - 1)
                {
                    puzzle.outputs[0].data.push_back(999);
                    puzzle.outputs[1].data.push_back(0);
                }
            }
            else
            {
                int value = std::uniform_int_distribution<int>(10, 100)(g_RandomEngine);
                puzzle.inputs[0].data.push_back(value);
                if (value < puzzle.outputs[0].data.back())
                    puzzle.outputs[0].data.back() = value;
                if (value > puzzle.outputs[1].data.back())
                    puzzle.outputs[1].data.back() = value;
            }
        }
        break;

    case 42656: // Sequence Reverser
                // Node arrangement:
                //     I
                //  0  1  S  3
                //  4  5  6  x
                //  X  S 10 11
                //        O
        puzzle.badNodes = { 8 };
        puzzle.stackNodes = { 2, 9 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, {} });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, {} });
        for (size_t i = 0, sequenceStart = 0; i < PuzzleInputSize; ++i)
        {
            // Zero
            if ((i == PuzzleInputSize - 1)
                || ((i > 0)
                    && (0 == std::uniform_int_distribution<int>(0, 5)(g_RandomEngine))))
            {
                //for (size_t j = i - 1; j >= sequenceStart; --j)
                for (size_t j = 1, n = puzzle.inputs.back().data.size(); j <= n - sequenceStart; ++j)
                {
                    puzzle.outputs.back().data.push_back(puzzle.inputs.back().data[n - j]);
                }
                puzzle.inputs.back().data.push_back(0);
                puzzle.outputs.back().data.push_back(0);
                sequenceStart = i + 1;
            }
            else
            {
                puzzle.inputs.back().data.push_back(std::uniform_int_distribution<int>(10, 100)(g_RandomEngine));
            }
        }
        break;

    case 43786: // Signal Multiplier
                // this is as far as I've gotten in the game :)
        throw std::exception("That puzzle hasn't been implemented yet.");

    default:
        throw std::exception("Unknown puzzle number.");
    }

    ReadSaveFile(saveFilePath,
        puzzle.programs,
        puzzle.badNodes);

    return TestPuzzle(puzzle, pCycleCount, pNodeCount, pInstructionCount);
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

    // Use the default seed to produce the same sequence every time (for debugability).
    g_RandomEngine.seed();

    for (int testRun = 0; testRun < 3; ++testRun)
    {
        int cycleCount = 0;
        int nodeCount = 0; 
        int instructionCount = 0;
        bool success;

        try
        {
            success = Test(puzzleNumber, saveFilePath, &cycleCount, &nodeCount, &instructionCount);
        }
        catch (std::exception ex)
        {
            const char* message = ex.what();
            std::cout << message << std::endl;
            return 1;
        }

        std::cout << (success ? "success" : "failure") << " in "
            << cycleCount << " cycles, "
            << nodeCount << " nodes, "
            << instructionCount << " instructions.\n";
    }
}