#pragma once

static constexpr size_t PuzzleInputSize = 39;
static constexpr size_t NodeGridWidth = 4;
static constexpr size_t NodeGridHeight = 3;
static constexpr size_t NodeGridCount = NodeGridWidth * NodeGridHeight;
static constexpr size_t VisualizationWidth = 30;
static constexpr size_t VisualizationHeight = 18;

typedef PuzzleBase<NodeGridCount> Puzzle;