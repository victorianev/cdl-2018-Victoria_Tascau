#include <iostream>
#include <string>
#include <queue>
#include <stack>
#include <vector>
#include <array>

#include <cstdio>
#include <cstdlib>

struct process
{
    size_t ip;
    int registers[32];
    std::queue<int> recv_queue;
    bool waiting, finished;

    process()
    {
        ip = 0;
        for(auto& r: registers)
        {
            r = 0;
        }
        waiting = finished = false;
    }
};

struct instruction
{
    enum type
    {
        set,
        add,
        mul,
        mod,
        jgz,
        snd,
        rcv
    } it;

    enum target
    {
        none,
        reg,
        number
    }t1, t2;

    int v1, v2;

    instruction()
    {
        it = type::set;
        t1 = target::none;
        t2 = target::none;
        v1 = v2 = 0;
    }
};

std::vector<instruction> instructions;
std::vector<process> processes;

int main()
{
    freopen("code.in", "r", stdin);
    freopen("code.out", "w", stdout);

    int N = 0;
    std::cin >> N;

    processes.insert(processes.begin(), N, process());
    for(size_t i = 0 ; i < processes.size() ; ++i)
    {
        processes[i].registers[0] = i;
    }

    std::string instr, t1, t2;

    while(true)
    {
        std::cin >> instr;
        if(std::cin.eof())
        {
            break;
        }

        instruction inst;

        if(instr == "set")
        {
            inst.it = instruction::set;
        }
        else if(instr == "add")
        {
            inst.it = instruction::add;
        }
        else if(instr == "mul")
        {
            inst.it = instruction::mul;
        }
        else if(instr == "mod")
        {
            inst.it = instruction::mod;
        }
        else if(instr == "jgz")
        {
            inst.it = instruction::jgz;
        }
        else if(instr == "snd")
        {
            inst.it = instruction::snd;
        }
        else if(instr == "rcv")
        {
            inst.it = instruction::rcv;
        }
        else
        {
            break;
        }

        std::string target;
        std::cin >> target;

        if(target[0] == 'R')
        {
            target.erase(0, 1);

            inst.t1 = instruction::reg;
        } else
        {
            inst.t1 = instruction::number;
        }

        inst.v1 = std::atoi(target.c_str());

        if(instr != "snd" && instr != "rcv")
        {
            std::cin >> target;
            if(target[0] == 'R')
            {
                target.erase(0, 1);

                inst.t2 = instruction::reg;
            } else
            {
                inst.t2 = instruction::number;
            }

            inst.v2 = std::atoi(target.c_str());
        }

        instructions.push_back(inst);
    }

    std::stack<int> pstack;
    pstack.push(0);

    while(!pstack.empty())
    {
        auto  current_process_i = pstack.top();
        auto& current_process = processes[current_process_i];
        pstack.pop();

        if(current_process.finished)
        {
            continue;
        }

        int ip_increment = 1;

        auto& current_instruction = instructions[current_process.ip];

        switch(current_instruction.it)
        {
        case instruction::set:
            {
                auto& target_register = current_process.registers[current_instruction.v1];

                if(current_instruction.t2 == instruction::reg)
                {
                    target_register = current_process.registers[current_instruction.v2];
                } else
                {
                    target_register = current_instruction.v2;
                }

                break;
            }
        case instruction::add:
            {
                auto& target_register = current_process.registers[current_instruction.v1];

                if(current_instruction.t2 == instruction::reg)
                {
                    target_register += current_process.registers[current_instruction.v2];
                } else
                {
                    target_register += current_instruction.v2;
                }

                break;
            }
        case instruction::mul:
            {
                auto& target_register = current_process.registers[current_instruction.v1];

                if(current_instruction.t2 == instruction::reg)
                {
                    target_register *= current_process.registers[current_instruction.v2];
                } else
                {
                    target_register *= current_instruction.v2;
                }

                break;
            }
        case instruction::mod:
            {
                auto& target_register = current_process.registers[current_instruction.v1];

                if(current_instruction.t2 == instruction::reg)
                {
                    target_register %= current_process.registers[current_instruction.v2];
                } else
                {
                    target_register %= current_instruction.v2;
                }

                break;
            }
        case instruction::jgz:
            {
                auto& target_register = current_process.registers[current_instruction.v1];

                if(target_register > 0)
                {
                    if(current_instruction.t2 == instruction::reg)
                    {
                        ip_increment = current_process.registers[current_instruction.v2];
                    } else
                    {
                        ip_increment = current_instruction.v2;
                    }
                }

                break;
            }
        case instruction::snd:
            {
                auto target_process_i = (current_process_i + 1) % N;
                auto& target_process = processes[target_process_i];

                if(current_instruction.t1 == instruction::reg)
                {
                    target_process.recv_queue.push(current_process.registers[current_instruction.v1]);
                } else
                {
                    target_process.recv_queue.push(current_instruction.v1);
                }

                break;
            }
        case instruction::rcv:
            {
                auto target_process_i =  current_process_i == 0 ? (N - 1) : (current_process_i - 1);
                auto& target_register = current_process.registers[current_instruction.v1];

                if(current_process.recv_queue.empty())
                {
                    if((processes[target_process_i].waiting && processes[target_process_i].recv_queue.empty()) || processes[target_process_i].finished)
                    {
                        current_process.finished = true;

                        break;
                    }

                    pstack.push(current_process_i);
                    pstack.push(target_process_i);

                    current_process.waiting = true;

                    break;
                } else
                {
                    target_register = current_process.recv_queue.front();
                    current_process.recv_queue.pop();

                    current_process.waiting = false;
                }

                break;
            }
        }

        if(current_process.waiting || current_process.finished)
        {
            continue;
        }

        current_process.ip += ip_increment;

        if(current_process.ip < instructions.size())
        {
            pstack.push(current_process_i);
        }
        else
        {
            current_process.finished = true;

            if(pstack.empty())
            {
                for(size_t i = 0 ; i != processes.size() ; ++i)
                {
                    if(processes[i].finished == false)
                    {
                        pstack.push(i);
                        break;
                    }
                }
            }
        }
    }

    for(auto& process: processes)
    {
        for(auto& reg: process.registers)
        {
            if(reg == 0)
                continue;

            std::cout << reg << " ";
        }

        std::cout << "\n";
    }

    return 0;
}
