#include "pch.h"
#include "Node.h"
#include "ComputeNode.h"
#include "IOChannel.h"

JumpTarget::JumpTarget()
    : type(JumpTargetType::Indeterminate)
{}

JumpTarget::JumpTarget(Target target)
    : type(JumpTargetType::Target)
{
    value.target = target;
}

JumpTarget::JumpTarget(int offset)
    : type(JumpTargetType::Offset)
{
    value.offset = offset;
}

JumpTarget::JumpTarget(std::string&& label)
    : type(JumpTargetType::Label)
{
    value.label = new std::string(label);
}

JumpTarget::JumpTarget(const JumpTarget& other)
    : type(other.type)
    , value(other.value)
{
    if (type == JumpTargetType::Label)
        value.label = new std::string(*other.value.label);
}

JumpTarget::JumpTarget(JumpTarget&& other)
    : type(other.type)
    , value(other.value)
{
    if (type == JumpTargetType::Label)
    {
        other.value.label = nullptr;
    }
}

JumpTarget::~JumpTarget()
{
    if (type == JumpTargetType::Label)
        delete value.label;
}

Instruction::Instruction()
    : op(Opcode::Indeterminate)
    , argsType(InstructionArgsType::Target)
{
    args = {}; // targets are set to None
}

Instruction::Instruction(const Instruction& other)
    : op(other.op)
    , argsType(other.argsType)
    , args(other.args)
{
    if (argsType == InstructionArgsType::JumpTarget)
        args.jumpTarget = new JumpTarget(*other.args.jumpTarget);
}

Instruction::Instruction(Instruction&& other)
    : op(other.op)
    , argsType(other.argsType)
    , args(other.args)
{
    if (argsType == InstructionArgsType::JumpTarget)
    {
        other.args.jumpTarget = nullptr;
    }
}

Instruction::~Instruction()
{
    switch (op)
    {
    case Opcode::JMP:
    case Opcode::JEZ:
    case Opcode::JNZ:
    case Opcode::JGZ:
    case Opcode::JLZ:
    case Opcode::JRO:
        delete args.jumpTarget;
    }
}

void Instruction::Clear()
{
    op = Opcode::Indeterminate;
    argsType = InstructionArgsType::Target;
    args = {};
}

static std::unordered_map<std::string, Opcode> s_opcodes = {
#define OP(_) { #_, Opcode::_ }
    OP(NOP),
    OP(MOV),
    OP(ADD),
    OP(SUB),
    OP(SAV),
    OP(SWP),
    OP(JMP),
    OP(JEZ),
    OP(JNZ),
    OP(JGZ),
    OP(JLZ),
    OP(JRO),
    OP(HCF)
#undef OP
};

static std::unordered_map<std::string, Target> s_targets = {
#define TGT(_) { #_, Target::##_ }
    TGT(NIL),
    TGT(ACC),
    TGT(UP),
    TGT(DOWN),
    TGT(LEFT),
    TGT(RIGHT),
    TGT(ANY)
#undef TGT
};

static Neighbor TargetToNeighbor(Target target)
{
    switch (target)
    {
#define MAP(_) case Target::##_: return Neighbor::##_
        MAP(UP);
        MAP(DOWN);
        MAP(LEFT);
        MAP(RIGHT);
#undef MAP
    default:
        throw std::exception("Target is not a neighbor direction");
    }
}

static void ParseOpcode(const std::string& str, Opcode* pOpcode)
{
    auto pair = s_opcodes.find(str);
    if (pair == s_opcodes.end())
        throw std::exception("unrecognized instruction opcode");

    *pOpcode = pair->second;
}

static bool TryParseTarget(const std::string& str, Target* pTarget)
{
    auto pair = s_targets.find(str);
    if (pair == s_targets.end())
        return false;

    *pTarget = pair->second;
    return true;
}

static void ParseTarget(const std::string& str, Target* pTarget)
{
    if (!TryParseTarget(str, pTarget))
        throw std::exception("unrecognized target");
}

static void ParseTargetOrLiteral(const std::string& str, Target* pTarget, int* pImmediate, InstructionArgsType* pType)
{
    if (TryParseTarget(str, pTarget))
    {
        *pType = InstructionArgsType::Target;
    }
    else
    {
        if (0 == sscanf_s(str.c_str(), "%d", pImmediate))
        {
            throw std::exception("expected a port, register or integer literal");
        }
        *pType = InstructionArgsType::Immediate;
    }
}

static void ParseJumpTarget(std::string&& str, Opcode op, JumpTarget** ppTarget)
{
    if (op == Opcode::JRO)
    {
        Target target;
        if (TryParseTarget(str, &target))
        {
            *ppTarget = new JumpTarget(target);
        }
        else
        {
            int value = 0;
            bool isNumeric = (1 == sscanf_s(str.c_str(), "%d", &value));

            if (isNumeric)
            {
                *ppTarget = new JumpTarget(value);
            }
            else
            {
                throw std::exception("JRO needs either a port, a register, or a number");
            }
        }
    }
    else
    {
        *ppTarget = new JumpTarget(std::move(str));
    }
}

static bool IsJumpOpcode(Opcode op)
{
    switch (op)
    {
    case Opcode::JMP:
    case Opcode::JEZ:
    case Opcode::JNZ:
    case Opcode::JGZ:
    case Opcode::JLZ:
    case Opcode::JRO:
        return true;
    default:
        return false;
    }
}

static bool IsOneArgOpcode(Opcode op)
{
    switch (op)
    {
    case Opcode::ADD:
    case Opcode::SUB:
        return true;
    default:
        return false;
    }
}

static bool IsTwoArgOpcode(Opcode op)
{
    switch (op)
    {
    case Opcode::MOV:
        return true;
    default:
        return false;
    }
}

int ComputeNode::s_nextNodeId = 0;

ComputeNode::ComputeNode()
    : m_nodeId(s_nextNodeId++)
    , m_state(State::Unprogrammed)
    , m_pc(0)
    , m_acc(0)
    , m_bak(0)
{
}

void ComputeNode::Assemble(const std::string& assembly)
{
    m_instructions.clear();

    Instruction instr;
    int line = 1;
    int column = 0;

    std::string word;
    bool wordComplete = false;
    bool inComment = false;
    bool instrComplete = false;
    for (size_t i = 0; i <= assembly.size(); i++)
    {
        auto parse_error = [&line, &column, &word](const char* message)
        {
            std::stringstream out;
            out << "line " << line << ", column " << (column - word.length());
            if (word.length() > 0)
            {
                out << "-" << column << " \"" << word << "\"";
            }
            out << ": " << message;
            throw std::exception(out.str().c_str());
        };

        try
        {
            char c;

            if (i < assembly.size())
            {
                c = assembly[i];
            }
            else
            {
                c = '\0';
                wordComplete = true;
            }

            if ((i > 0) && (assembly[i - 1] == '\n'))
            {
                column = 0;
                line++;
            }

            column++;

            if (inComment)
            {
                if (c == '\n')
                    inComment = false;
                continue;
            }
            if (c == ' ' || c == '\n')
            {
                if (word.empty() || wordComplete)
                {
                    continue;
                }
                else
                {
                    wordComplete = true;
                    if (c == '\n')
                    {
                        instrComplete = true;
                    }
                    continue;
                }
            }
            else if (c == '#')
            {
                inComment = true;
                wordComplete = true;
                instrComplete = true;
            }
            else if ((c == ':') && (instr.op == Opcode::Indeterminate))
            {
                // label was defined
                m_labels.emplace(word, m_instructions.size());
                word.clear();
                continue;
            }
            else if ((c == ',')
                && IsTwoArgOpcode(instr.op)
                && (instr.argsType == InstructionArgsType::Target)
                && (instr.args.arg1.target == Target::None))
            {
                wordComplete = true;
                continue;
            }

            if (wordComplete)
            {
                if (instr.op == Opcode::Indeterminate)
                {
                    if (!word.empty())
                        ParseOpcode(word, &instr.op);
                }
                else if (IsOneArgOpcode(instr.op))
                {
                    if ((instr.argsType == InstructionArgsType::Target)
                        && (instr.args.arg1.target == Target::None))
                    {
                        ParseTargetOrLiteral(word, &instr.args.arg1.target, &instr.args.arg1.immediate, &instr.argsType);
                    }
                    else
                    {
                        parse_error("instruction already has an arg1");
                    }
                }
                else if (IsTwoArgOpcode(instr.op))
                {
                    if ((instr.argsType == InstructionArgsType::Target)
                        && (instr.args.arg1.target == Target::None))
                    {
                        // The source can be a target or a literal.
                        ParseTargetOrLiteral(word, &instr.args.arg1.target, &instr.args.arg1.immediate, &instr.argsType);
                    }
                    else if (instr.args.arg2 == Target::None)
                    {
                        // The destination can only be a target.
                        ParseTarget(word, &instr.args.arg2);
                    }
                    else
                    {
                        parse_error("instruction already has an arg2");
                    }
                }
                else if (IsJumpOpcode(instr.op))
                {
                    instr.argsType = InstructionArgsType::JumpTarget;
                    ParseJumpTarget(std::move(word), instr.op, &instr.args.jumpTarget);
                }
                else
                {
                    parse_error("instruction does not take arguments");
                }

                word.clear();
                wordComplete = false;
            }

            if (!inComment)
            {
                if ((c >= 'a' && c <= 'z')
                    || ((c >= 'A' && c <= 'Z'))
                    || ((c >= '0' && c <= '9'))
                    || (c == '-'))
                {
                    word.push_back(c);
                }
                else if (c != '\0')
                {
                    parse_error("invalid character");
                }
            }

            if (instrComplete)
            {
                if (instr.op != Opcode::Indeterminate)
                    m_instructions.push_back(std::move(instr));
                instr.Clear();
                instrComplete = false;
            }
        }
        catch (std::exception ex)
        {
            parse_error(ex.what());
        }
    }

    if (instr.op != Opcode::Indeterminate)
    {
        m_instructions.push_back(instr);
    }
}

int ComputeNode::InstructionCount()
{
    return m_instructions.size();
}

void ComputeNode::SetNeighbor(Neighbor direction, std::shared_ptr<IOChannel>& spIO)
{
    m_neighbors[static_cast<size_t>(direction)] = spIO;
}

void ComputeNode::Initialize()
{
    if (m_instructions.size() > 0)
    {
        m_state = State::Run;
    }
    else
    {
        m_state = State::Unprogrammed;
    }
    m_pc = 0;
}

std::shared_ptr<IOChannel>& ComputeNode::IO(Target target)
{
    return m_neighbors[static_cast<size_t>(TargetToNeighbor(target))];
}

void ComputeNode::Read()
{
    if (m_state != State::Run && m_state != State::Read)
        return;

    Instruction& instr = m_instructions[m_pc];

    Target readTarget = Target::None;

    switch (instr.op)
    {
    case Opcode::MOV:
    case Opcode::ADD:
    case Opcode::SUB:
        if (instr.argsType == InstructionArgsType::Immediate)
            m_temp = instr.args.arg1.immediate;
        else
            readTarget = instr.args.arg1.target;
        break;

    case Opcode::JRO:
        if (instr.args.jumpTarget->type == JumpTargetType::Target)
        {
            readTarget = instr.args.jumpTarget->value.target;
        }
        break;

    case Opcode::JMP:
    case Opcode::JEZ:
    case Opcode::JNZ:
    case Opcode::JGZ:
    case Opcode::JLZ:
        if (instr.args.jumpTarget->type == JumpTargetType::Target)
        {
            throw std::exception("target jumps are only supported for JRO");
        }
        break;
    }

    switch (readTarget)
    {
    case Target::None:
        break;

    case Target::NIL:
        m_temp = 0;
        break;

    case Target::ACC:
        m_temp = m_acc;
        break;

    case Target::UP:
    case Target::DOWN:
    case Target::LEFT:
    case Target::RIGHT:
    {
        m_state = State::Read;
        std::shared_ptr<IOChannel>& spIO = IO(readTarget);
        if ((spIO != nullptr) && spIO->Read(this, &m_temp))
        {
            m_state = State::Run;
        }
    }
    break;

    case Target::ANY:
        throw std::exception("not implemented");
    }
}

void ComputeNode::Compute()
{
    if (m_state != State::Run)
        return;

    Instruction& instr = m_instructions[m_pc];

    switch (instr.op)
    {
    case Opcode::ADD:
        m_acc += m_temp;
        break;

    case Opcode::SUB:
        m_acc -= m_temp;
        break;

    case Opcode::SAV:
        m_bak = m_acc;
        break;

    case Opcode::SWP:
        std::swap(m_acc, m_bak);
        break;

    case Opcode::HCF:
        throw std::exception("halt and catch fire"); // lol
    }
}

void ComputeNode::Write()
{
    if (m_state != State::Run)
        return;

    Instruction& instr = m_instructions[m_pc];
    Target writeTarget = Target::None;

    switch (instr.op)
    {
    case Opcode::MOV:
        writeTarget = instr.args.arg2;
        break;
    }

    switch (writeTarget)
    {
    case Target::None:
    case Target::NIL:
        break;

    case Target::ACC:
        m_acc = m_temp;
        break;

    case Target::UP:
    case Target::DOWN:
    case Target::LEFT:
    case Target::RIGHT:
    {
        m_state = State::Write;
        std::shared_ptr<IOChannel>& spIO = IO(writeTarget);
        if (spIO != nullptr)
        {
            spIO->Write(this, m_temp);
        }
    }
    break;

    case Target::ANY:
        throw std::exception("not implemented");
    }
}

void ComputeNode::WriteComplete()
{
    switch (m_state)
    {
    case State::Write:
        m_state = State::WriteComplete;
        break;
    default:
        throw std::exception("unexpected WriteComplete");
    }
}

void ComputeNode::Step()
{
    switch (m_state)
    {
    case State::Run:
        break;

    case State::Unprogrammed:
    case State::Read:
    case State::Write:
        return;

    case State::WriteComplete:
        m_state = State::Run;
        break;
    }

    Instruction& instr = m_instructions[m_pc];

    bool jumpPredicate = false;

    switch (instr.op)
    {
    case Opcode::JMP:
        jumpPredicate = true;
        break;

    case Opcode::JEZ:
        jumpPredicate = (m_acc == 0);
        break;

    case Opcode::JNZ:
        jumpPredicate = (m_acc != 0);
        break;

    case Opcode::JGZ:
        jumpPredicate = (m_acc > 0);
        break;

    case Opcode::JLZ:
        jumpPredicate = (m_acc < 0);
        break;

    case Opcode::JRO:
        jumpPredicate = true;
        break;
    }

    if (jumpPredicate)
    {
        switch (instr.args.jumpTarget->type)
        {
        case JumpTargetType::Indeterminate:
            throw std::exception("indeterminate jump target");
        case JumpTargetType::Label:
            m_pc = m_labels.find(*instr.args.jumpTarget->value.label)->second;
            break;
        case JumpTargetType::Offset:
            m_pc += instr.args.jumpTarget->value.offset;
            break;
        case JumpTargetType::Target:
            // offset was loaded by Read()
            m_pc += m_temp;
        }
    }
    else
    {
        ++m_pc;
    }

    if (m_pc >= m_instructions.size())
    {
        if ((instr.op == Opcode::JRO) && jumpPredicate)
        {
            // if you JRO to an out-of-range instruction, the pc goes to the last instruction.
            m_pc = m_instructions.size() - 1;
        }
        else
        {
            m_pc = 0;
        }
    }
    else if (m_pc < 0)
    {
        m_pc = 0;
    }
}