#pragma once

class OutputNode : public OutputBase
{
public:
    std::vector<int> Data;

    OutputNode();

    virtual void Initialize() override;
    virtual void ReadData(int value) override;
};