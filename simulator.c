/*
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 *
 * Make sure to NOT modify printState or any of the associated functions
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//DO NOT CHANGE THE FOLLOWING DEFINITIONS 

// Machine Definitions
#define NUMMEMORY 65536 /* maximum number of words in memory (maximum number of lines in a given file)*/
#define NUMREGS 8 /*total number of machine registers [0,7]*/

// File Definitions
#define MAXLINELENGTH 1000 /* MAXLINELENGTH is the max number of characters we read */

typedef struct 
stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

static inline int convertNum(int32_t);

int 
main(int argc, char **argv)
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s , please ensure you are providing the correct path", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state.numMemory) {
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address  %d\n. Please ensure you are providing a machine code file.", state.numMemory);
            perror("sscanf");
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }


    /*
        The simulator should begin by initializing all registers and the program counter to 0. 
        The simulator will then simulate the program until the program executes a halt.
        The simulator should call the printState function before executing each instruction 
        and once just before exiting the program. 
    */
    for (int i = 0; i < NUMREGS; ++i) {
        state.reg[i] = 0;
    }
    state.pc = 0;

    int numInstructions = 0;
    while (true) {
        ++numInstructions;
        printState(&state);

        int instruction = state.mem[state.pc];

        // bits 24-22: opcode
        int opcode = (instruction & 0b00000001110000000000000000000000) >> 22;

        // add, nor
        if (opcode == 0b000 || opcode == 0b001) {
            // bits 21-19: reg A
            // bits 18-16: reg B
            // bits 2-0: destReg
            int regA = (instruction & 0b00000000001110000000000000000000) >> 19;
            int regB = (instruction & 0b00000000000001110000000000000000) >> 16;
            int destReg = (instruction & 0b00000000000000000000000000000111) >> 0;
            // Add contents of regA with contents of regB, store results in destReg.
            if (opcode == 0b000) {
                state.reg[destReg] = state.reg[regA] + state.reg[regB];
                ++state.pc;
            }
            // Nor contents of regA with contents of regB, store results in destReg. 
            else {
                state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
                ++state.pc;
            }
        }
        // lw, sw, beq
        else if (opcode == 0b010 || opcode == 0b011 || opcode == 0b100) {
            // bits 21-19: reg A
            // bits 18-16: reg B
            // bits 15-0: offsetField (a 16-bit, 2’s complement number with a range of -32768 to 32
            int regA = (instruction & 0b00000000001110000000000000000000) >> 19;
            int regB = (instruction & 0b00000000000001110000000000000000) >> 16;
            int offsetField = (instruction & 0b00000000000000001111111111111111) >> 0;
            offsetField = convertNum(offsetField);

            // lw - “Load Word”; Load regB from memory. 
            // Memory address is formed by adding offsetField with the contents of regA
            if (opcode == 0b010) {
                int addr = offsetField + state.reg[regA];
                state.reg[regB] = state.mem[addr];
                ++state.pc;
            }
            // “Store Word”; Store regB into memory. 
            // Memory address is formed by adding offsetField with the contents of regA
            else if (opcode == 0b011) {
                int addr = offsetField + state.reg[regA];
                state.mem[addr] = state.reg[regB];
                ++state.pc;
            }
            // “Branch if equal” If the contents of regA and regB are the same, 
            // then branch to the address PC+1+offsetField, where PC is the address of this beq instruction.
            else {
                if (state.reg[regA] == state.reg[regB]) {
                    state.pc = state.pc + 1 + offsetField;
                }
                else {
                    ++state.pc;
                }
            }
        }
        else if (opcode == 0b101) {
            // bits 21-19: reg A
            // bits 18-16: reg B
            int regA = (instruction & 0b00000000001110000000000000000000) >> 19;
            int regB = (instruction & 0b00000000000001110000000000000000) >> 16;
            // Jump and Link Register; First store the value PC+1 into regB, 
            // where PC is the address where this jalr instruction is defined. 
            // Then branch (set PC) to the address contained in regA
            state.reg[regB] = state.pc + 1;
            state.pc = state.reg[regA];
        }
        else if (opcode == 0b110 || opcode == 0b111) {
            // halt
            if (opcode == 0b110) {
                // Increment the PC (as with all instructions), 
                // then halt the machine (let the simulator notice that the machine halted).
                state.pc++;
                break;
            }
            // noop
            else {
                // Do nothing
                state.pc++;
            }
        }
    }

    printf("machine halted\n");
    printf("total of %d instructions executed\n", numInstructions);
    printf("final state of machine:\n");
    printState(&state);

    return(0);
}

/*
* DO NOT MODIFY ANY OF THE CODE BELOW. 
*/

void 
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) 
              printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) 
              printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    printf("end state\n");
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) 
{
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

/*
* Write any helper functions that you wish down here. 
*/