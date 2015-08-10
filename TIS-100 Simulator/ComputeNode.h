#pragma once

enum class Opcode
{
    Indeterminate,
    NOP,
    MOV,
    ADD,
    SUB,
    SAV,
    SWP,
    JMP,
    JEZ,
    JNZ,
    JGZ,
    JLZ,
    JRO,
    HCF // lol
};

enum class Target
{
    None,
    NIL,
    ACC,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    ANY,
    LAST
};

enum class JumpTargetType
{
    Indeterminate,
    Target,
    Offset,
    Label
};

class JumpTarget
{
public:
    JumpTargetType type;
    union {
        Target target;
        int offset;
        std::string* label;
    } value;

    JumpTarget();
    JumpTarget(Target target);
    JumpTarget(int offset);
    JumpTarget(std::string&& label);
    JumpTarget(const JumpTarget& other);
    JumpTarget(JumpTarget&& other);
    ~JumpTarget();
};

enum class InstructionArgsType
{
    Target,
    Immediate,
    JumpTarget
};

class Instruction
{
public:
    Instruction();
    Instruction(const Instruction& other);
    Instruction(Instruction&& other);
    ~Instruction();
    void Clear();

    Opcode op;
    InstructionArgsType argsType;
    union InstructionArgs {
        struct {
            union {
                Target target;
                int immediate;
            } arg1;
            Target arg2;
        };
        JumpTarget* jumpTarget;
    } args;
};

class ComputeNode : public INode
{
private:
    enum class State
    {
        Unprogrammed,
        Run,
        Read,
        Write,
        WriteComplete,
    };

    static int s_nextNodeId;

private:
    int m_nodeId;
    State m_state;
    size_t m_pc;
    int m_acc;
    int m_bak;
    int m_temp;
    Target m_last;
    std::vector<Instruction> m_instructions;
    std::unordered_map<std::string, size_t> m_labels;
    std::shared_ptr<IOChannel> m_neighbors[static_cast<size_t>(Neighbor::COUNT)];

public:
    ComputeNode();

    void Assemble(const std::string& assembly);
    int InstructionCount();

    virtual void SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO);
    virtual void Initialize();

    virtual void Read();
    virtual void Compute();
    virtual void Write();
    virtual void WriteComplete();
    virtual void Step();

private:
    std::shared_ptr<IOChannel>& IO(Target target);
};