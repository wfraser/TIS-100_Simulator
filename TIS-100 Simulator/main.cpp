#include "pch.h"
#include "Node.h"
#include "InputNode.h"
#include "OutputBase.h"
#include "OutputNode.h"
#include "ComputeNode.h"
#include "StackMemoryNode.h"
#include "Grid.h"
#include "VisualizationNode.h"
#include "Puzzle.h"
#include "ComputeGrid.h"

#include "Constants.h"

std::default_random_engine g_RandomEngine;

// Read a save file.
//
// Formal Parameters:
//  path: path to the save file.
//  programs: array of strings set to the corresponding assembly text for each node.
//  badNodes: the indices of non-functional nodes in the puzzle.
//  stackNodes: the indices of stack memory nodes in the puzzle.
//
// Indices in the programs array that are in badNodes and stackNodes will be skipped over and left
// empty. (The indices are needed because the save file skips over them entirely and just writes
// the working ComputeNode programs.)
void ReadSaveFile(
    const wchar_t* path,
    std::string programs[NodeGridCount],
    const std::set<int>& badNodes,
    const std::set<int>& stackNodes
    )
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

            while (badNodes.find(nodeNumber) != badNodes.end()
                || stackNodes.find(nodeNumber) != stackNodes.end())
            {
                nodeNumber++;
            }
        }
        else
        {
            program += line + "\n";
        }
    }
}

// Run the program in the given puzzle against the puzzle described in the same.
// Returns true if the output matches the expected data. Returns false if the data does not match.
// Stores program statistics in the remaining formal parameters:
//  pNumCycles: number of cycles to program halt, either due to matching output data, or the first
//              mismatch.
//  pNodeCount: the number of ComputeNodes that were programmed with at least one instruction.
//  pInstructionCount: the total number of instructions programmed into all ComputeNodes.
bool TestPuzzle(
    const Puzzle& puzzle,
    ComputeGrid<NodeGridHeight, NodeGridWidth>& grid,
    int* pCycleCount
    )
{
    grid.Initialize();

    bool isFailure = false;
    while (!grid.IsFinished(puzzle, &isFailure))
    {
        ++(*pCycleCount);

#ifdef DEBUG_OUTPUT
        printf("\t\t\t\t\t\t\tcycle %d\n", cycleCount);
#endif

        grid.Step();
    }

    return !isFailure;
}

int DoTest(int puzzleNumber, const wchar_t* saveFilePath)
{
    std::string puzzleName;
    Puzzle puzzle = GetPuzzle(puzzleNumber, puzzleName);

    if (puzzleNumber > 0)
        ReadSaveFile(saveFilePath, puzzle.programs, puzzle.badNodes, puzzle.stackNodes);

    ComputeGrid<NodeGridHeight, NodeGridWidth> grid(puzzle);

    int instructionCount = 0;
    int nodeCount = 0;
    grid.GetStats(&nodeCount, &instructionCount);
    std::cout << puzzleNumber << ": " << puzzleName
        << " - " << nodeCount << " nodes, "
        << instructionCount << " instructions.\n";

    // Use the default seed to produce the same sequence every time (for debugability).
    g_RandomEngine.seed();

    for (int testRun = 0; testRun < 3; ++testRun)
    {
        int cycleCount = 0;
        bool success = false;

        try
        {
            success = TestPuzzle(puzzle, grid, &cycleCount);
        }
        catch (std::exception ex)
        {
            const char* message = ex.what();
            std::cout << message << std::endl;
            return 1;
        }

        std::cout << "\t" << (success ? "success" : "failure") << " in "
            << cycleCount << " cycles.\n";
    }

    return 0;
}

int wmain(int argc, wchar_t** argv)
{
    if ((argc == 3) && (std::wstring(argv[1]) == L"all"))
    {
        using namespace std::tr2::sys;

        for (const path& entry : directory_iterator(argv[2]))
        {
            std::wstring saveFilename(entry.filename().generic_wstring());

            auto pos = saveFilename.find_first_of(L'.');
            if (pos == saveFilename.npos)
                continue;

            std::wstring numberPart(saveFilename.substr(0, pos));
            wchar_t* end;
            int puzzleNumber = wcstol(numberPart.c_str(), &end, 10);

            if (*end == L'\0')
            {
                std::wcout << L"Save file: " << saveFilename << std::endl;
                DoTest(puzzleNumber, entry.c_str());
            }
        }

        return 0;
    }
    else if (argc == 3)
    {
        int puzzleNumber;
        const wchar_t* saveFilePath;

        if (0 == swscanf_s(argv[1], L"%u", &puzzleNumber))
        {
            std::cout << "invalid puzzle number\n";
        }
        saveFilePath = argv[2];

        return DoTest(puzzleNumber, saveFilePath);
    }
    else
    {
        std::cout << "usage: " << argv[0] << " <puzzle number> <save file>\n"
            "\n"
            "look for saves in "
            R"(%USERPROFILE%\Documents\my games\TIS-100\<random number>\save)"
            "\n";
        return -1;
    }
}