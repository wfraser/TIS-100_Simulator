#include "pch.h"
#include "Node.h"
#include "Puzzle.h"
#include "Constants.h"

extern std::default_random_engine g_RandomEngine;

// Generate data according to a given lambda.
// The lambda is given an index, and stores the value in the given int pointer.
// The lambda returns true if it generates a value; false if no more data should be generated.
// The lambda is called with the indices in order, starting at zero.
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

// Generate a given number of random integers, in the given range.
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

// Transforms the data in one IO into another data set, according to a lambda.
// The lambda is given an integer, and returns the desired transformation of that integer.
static std::vector<int> PuzzleInputSimpleGenerator(const Puzzle::IO& input, std::function<int(int)> fn)
{
    std::vector<int> values;
    for (int value : input.data)
    {
        values.push_back(fn(value));
    }
    return values;
}

// Generate a TIS-100 puzzle.
//
// Formal Parameters:
//  puzzleNumber: the puzzle number to get.
//  puzzleName: is set to the name of the puzzle specified by puzzleNumber.
//
// Returns the specified puzzle.
Puzzle GetPuzzle(
    int puzzleNumber,
    std::string& puzzleName
    )
{
    Puzzle puzzle;

    puzzle.visualizationHeight = VisualizationHeight;
    puzzle.visualizationWidth = VisualizationWidth;

    switch (puzzleNumber)
    {
    case -3:
        puzzleName = "[simulator debug] Visualization Node Test";
        puzzle.badNodes = {};
        puzzle.visualization.push_back(Puzzle::IO{ 0, Neighbor::UP,{ 3,3,3,3,3 } });
        puzzle.programs[0] = "MOV 0,UP\nMOV 0,UP\nMOV 3,UP\nJRO -1";
        puzzle.programs[1] = "ADD 1";
        break;

    case -2:
        puzzleName = "[simulator debug] Stack Memory Test";
        puzzle.badNodes = {};
        puzzle.stackNodes = { 1 };
        puzzle.inputs.push_back(Puzzle::IO{ 0, Neighbor::UP,{ 1,2,3,4 } });
        puzzle.outputs.push_back(Puzzle::IO{ 2, Neighbor::UP,{ 1,2,3,4 } });

        puzzle.programs[0] = "MOV UP,RIGHT";
        puzzle.programs[2] = "NOP\nMOV LEFT,UP";
        break;

    case -1:
        puzzleName = "[simulator debug] Connectivity Check";
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

        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP,{ 1, 2, 3, 4 } });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN,{ 10, 20, 30, 40 } });
        break;

    case 150:
        puzzleName = "Self-Test Diagnostic";
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

    case 10981:
        puzzleName = "Signal Amplifier";
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

    case 20176:
        puzzleName = "Differential Converter";
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

    case 21340:
        puzzleName = "Signal Comparator";
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

    case 22280:
        puzzleName = "Signal Multiplexer";
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

    case 30647:
        puzzleName = "Sequence Generator";
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

    case 31904:
        puzzleName = "Sequence Counter";
        // Node arrangement:
        //     I
        //  0  1  2  x
        //  4  5  6  7
        //  8  9 10 11
        //     O  O
        puzzle.badNodes = { 3 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP,{} });
        puzzle.outputs.push_back(Puzzle::IO{ 9, Neighbor::DOWN,{ 0 } });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN,{ 0 } });
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

    case 32050:
        puzzleName = "Signal Edge Detector";
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

    case 33762:
        puzzleName = "Interrupt Handler";
        // Node arrangement:
        //  I  I  I  I
        //  0  1  2  3
        //  4  5  6  7
        //  x  9 10 11
        //        O
        puzzle.badNodes = { 8 };
        puzzle.inputs.push_back(Puzzle::IO{ 0, Neighbor::UP,{} });
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP,{} });
        puzzle.inputs.push_back(Puzzle::IO{ 2, Neighbor::UP,{} });
        puzzle.inputs.push_back(Puzzle::IO{ 3, Neighbor::UP,{} });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN,{} });
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

    case 40196:
        puzzleName = "Signal Pattern Detector";
        // Node arrangement:
        //     I
        //  0  1  2  x
        //  4  5  6  7
        //  8  9 10 11
        //        O
        puzzle.badNodes = { 3 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP,{ 1 } });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN,{ 0 } });
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

    case 41427:
        puzzleName = "Sequence Peak Detector";
        // Node arrangement:
        //     I
        //  0  1  2  3
        //  4  5  6  x
        //  8  9 10 11
        //     O  O
        puzzle.badNodes = { 7 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP,{} });
        puzzle.outputs.push_back(Puzzle::IO{ 9, Neighbor::DOWN,{ 999 } });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN,{ 0 } });
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

    case 42656:
        puzzleName = "Sequence Reverser";
        // Node arrangement:
        //     I
        //  0  1  S  3
        //  4  5  6  7
        //  x  S 10 11
        //        O
        puzzle.badNodes = { 8 };
        puzzle.stackNodes = { 2, 9 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP,{} });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN,{} });
        for (size_t i = 0, sequenceStart = 0; i < PuzzleInputSize; ++i)
        {
            // Zero
            if ((i == PuzzleInputSize - 1)
                || ((i > 0)
                    && (0 == std::uniform_int_distribution<int>(0, 5)(g_RandomEngine))))
            {
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

    case 43786:
        puzzleName = "Signal Multiplier";
        // Node arrangement:
        //     I  I
        //  0  1  2  3
        //  S  5  6  S
        //  x  S 10 11
        //        O
        puzzle.badNodes = { 8 };
        puzzle.stackNodes = { 4, 7 };
        puzzle.inputs.push_back(Puzzle::IO{ 1, Neighbor::UP, RandomGenerator(PuzzleInputSize, 0, 9) });
        puzzle.inputs.push_back(Puzzle::IO{ 2, Neighbor::UP, RandomGenerator(PuzzleInputSize, 0, 9) });
        puzzle.outputs.push_back(Puzzle::IO{ 10, Neighbor::DOWN, FunctionGenerator([&puzzle](size_t i, int* value)->bool
        {
            if (i >= PuzzleInputSize)
                return false;

            *value = puzzle.inputs[0].data[i] * puzzle.inputs[1].data[i];
            return true;
        }) });
        break;

    case 50370:
        puzzleName = "Image Test Pattern 1";
        // Node arrangement:
        //  0  1  2  3
        //  x  5  6  7
        //  8  9 10 11
        //        V
        puzzle.badNodes = { 4 };
        puzzle.visualization.push_back(Puzzle::IO{ 10, Neighbor::DOWN, FunctionGenerator([](size_t i, int* value)->bool
        {
            if (i >= VisualizationWidth * VisualizationHeight)
                return false;

            *value = 3;
            return true;
        }) });
        break;

    case 51781:
        puzzleName = "Image Test Pattern 2";
        // Node arrangement:
        //  x  1  2  3
        //  4  5  6  7
        //  8  9 10 11
        //        V
        puzzle.badNodes = { 0 };
        puzzle.visualization.push_back(Puzzle::IO{ 10, Neighbor::DOWN, FunctionGenerator([](size_t i, int* value)->bool {
            if (i >= VisualizationWidth * VisualizationHeight)
                return false;

            if ((i / VisualizationWidth) % 2 == 0)
            {
                *value = (((i % VisualizationWidth) % 2) == 0) ? 3 : 0;
            }
            else
            {
                *value = (((i % VisualizationWidth) % 2) == 1) ? 3 : 0;
            }
            return true;
        }) });
        break;

    case 52544: // Exposure Mask Viewer
    case 53897: // Histogram Viewer
    case 60099: // Signal Window Filter
    case 61212: // Signal Divider
    case 62711: // Sequence Indexer
    case 63534: // Sequence Sorter
                // this is as far as I've gotten in the game :)
        throw std::exception("That puzzle hasn't been implemented yet.");

    default:
        throw std::exception("Unknown puzzle number.");
    }

    return puzzle;
}