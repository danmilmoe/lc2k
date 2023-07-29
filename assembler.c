/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000
#define MAXLABELLENGTH 7

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static inline int isNumber(char *);

typedef struct Label {
    int PC;
    char name[MAXLINELENGTH];
    struct Label *next;
} Label;

Label *labelHead = NULL;

#define ERROR(PC, ...) {\
    fprintf(stderr, "error: PC = %d: ", PC);\
    fprintf(stderr, __VA_ARGS__);\
    fprintf(stderr, "\n");\
    exit(1);\
}

void errorExit(char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

Label * findLabel(char *label) {
    Label *i;

    i = labelHead;
    while (i) {
        if (!strcmp(label, i->name))  {
            return i;
        }
        i = i->next;
    }
    return NULL;
}

void insertLabel(int pc, char *name) {
    Label *label = (Label *) malloc(sizeof(Label));

    strncpy(label->name, name, MAXLINELENGTH);
    label->PC = pc;
    label->next = NULL;

    // Insert into linked list
    label->next = labelHead;
    labelHead = label;
}

void readAllLabels(FILE *file) {
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], 
         arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int PC = 0;

    while (readAndParse(file, label, opcode, arg0, arg1, arg2) ) {
        int size = strlen(label);
        if (size) {
            if (size > MAXLABELLENGTH) {
                ERROR(PC, "label: %s length > %d", label, MAXLABELLENGTH);
            }
            // check if label already exists
            if (findLabel(label)) {
                ERROR(PC, "label: %s already exists", label);
            }
            
            insertLabel(PC, label);
        }
        PC++;
    }
}

int registerStringToNumber(int pc, char *str) {
    if (isNumber(str)) {
        int reg = atoi(str);
        if (reg < 0 || reg > 7) {
            ERROR(pc, "Register %s not in range [0 - 7]", str);
        }
        return reg;
    }
    ERROR(pc, "Cannot convert register %s to integer", str);
}

int opcodeNameToOpcode(int pc, char *opcodeName) {
    char *names[] = { "add", "nor", "lw", "sw", "beq", "jalr", "halt", "noop"};
    int opcodes[] = { 0b000, 0b001, 0b010, 0b011, 0b100, 0b101, 0b110, 0b111};
    int i;

    for (i = 0; i < 8; ++i) {
        if (!strcmp(names[i], opcodeName)) {
            return opcodes[i];
        }
    }
    ERROR(pc, "Cannot convert %s to opcode", opcodeName);
}

int assembleRTypeInstruction(int pc, char *opcodeName, char *field0, char *field1, char *field2) {
    /*
        add, nor
        opcode, field0, field1, and field2 are required fields:
        field0, 1, 2 are registers A, B, dest respectively
    */

    int opcode = opcodeNameToOpcode(pc, opcodeName);
    int regA = registerStringToNumber(pc, field0);
    int regB = registerStringToNumber(pc, field1);
    int regD = registerStringToNumber(pc, field2);

    // bits 24-22: opcode, bits 21-19: reg A, bits 18-16: reg B, bits 15-3: unused (should all be 0), bits 2-0: destReg
    int instruction = 0;
    instruction |= (opcode << 22);
    instruction |= (regA << 19);
    instruction |= (regB << 16);
    instruction |= (regD << 0);
    
    return instruction;
}

int assembleITypeInstruction(int pc, char *opcodeName, char *field0, char *field1, char *field2) {
    /*
        lw, sw, beq
        opcode , field0 , field1 and field2 are required fields:
        field0, 1 are registers A and B; 2 is a number or symbol address is a register (regA)
    */

    int opcode = opcodeNameToOpcode(pc, opcodeName);
    int regA = registerStringToNumber(pc, field0);
    int regB = registerStringToNumber(pc, field1);
    int offset = 0;

    if (isNumber(field2)) {
        offset = atoi(field2);
    } else {
        Label *label = findLabel(field2);
        if (!label) {
            ERROR(pc, "label %s not found", field2);
        }
        offset = label->PC;

        // beq, convert offset such that when added to PC + 1 gives old offset
        if (opcode == 0b100) {
            offset = offset - (pc + 1);
        }
    }

    if (offset < -32768 || offset > 32767) {
        ERROR(pc, "offset wont fit in 16 bits");
    }

    // upper 16 to 0s
    offset = offset & 0x0000ffff;

    // bits 24-22: opcode, bits 21-19: reg A, bits 18-16: reg B
    // bits 15-0: offsetField (a 16-bit, 2’s complement number with a range of -32768 to 32767)
    int instruction = 0;
    instruction |= (opcode << 22);
    instruction |= (regA << 19);
    instruction |= (regB << 16);
    instruction |= (offset << 0);
    
    return instruction;
}


int assembleJTypeInstruction(int pc, char *opcodeName, char *field0, char *field1, char *field2) {
    /*
        jalr
        opcode, field0, and field1 are required fields:
        field0, 1 are registers A and B
    */

    int opcode = opcodeNameToOpcode(pc, opcodeName);
    int regA = registerStringToNumber(pc, field0);
    int regB = registerStringToNumber(pc, field1);

    // bits 24-22: opcode, bits 21-19: reg A, bits 18-16: reg B
    // bits 15-0: unused (should all be 0)
    int instruction = 0;
    instruction |= (opcode << 22);
    instruction |= (regA << 19);
    instruction |= (regB << 16);
    
    return instruction;
}

int assembleOTypeInstruction(int pc, char *opcodeName, char *field0, char *field1, char *field2) {
    /*
        noop, halt
        Only the opcode field is required
    */
    int opcode = opcodeNameToOpcode(pc, opcodeName);

    // bits 24-22: opcode
    // bits 21-0: unused (should all be 0)
    int instruction = 0;
    instruction |= (opcode << 22);

    return instruction;
}

int assembleFill(int pc, char *opcodeName, char *field0, char *field1, char *field2) {
    int value = 0;

    if (isNumber(field0)) {
        value = atoi(field0);    
    } else {
        Label *label = findLabel(field0);
        if (!label) {
            ERROR(pc, "label %s not found", field0);
        }
        value = label->PC;
    }
    return value;
}

void assembler(FILE *inFile, FILE *outFile) {
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], field0[MAXLINELENGTH], 
         field1[MAXLINELENGTH], field2[MAXLINELENGTH];
    int PC = 0;

    while (readAndParse(inFile, label, opcode, field0, field1, field2) ) {
        int instruction = 0;

        if (!strcmp(opcode, "add") || !strcmp(opcode, "nor")) {
            instruction = assembleRTypeInstruction(PC, opcode, field0, field1, field2);
        } else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq")) {
            instruction = assembleITypeInstruction(PC, opcode, field0, field1, field2);
        } else if (!strcmp(opcode, "jalr")) {
            instruction = assembleJTypeInstruction(PC, opcode, field0, field1, field2);
        } else if (!strcmp(opcode, "halt") || !strcmp(opcode, "noop")) {
            instruction = assembleOTypeInstruction(PC, opcode, field0, field1, field2);
        } else if (!strcmp(opcode, ".fill")) {
            instruction = assembleFill(PC, opcode, field0, field1, field2);
        } else {
            ERROR(PC, "Invalid opcode: %s", opcode);
        }
        fprintf(outFile, "%d\n", instruction);
        PC++;
    }
}

int main(int argc, char **argv) {
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    //files to args
    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    //Read all labels
    readAllLabels(inFilePtr);    

    //this is how to rewind the file ptr so that you start reading from thebeginning of the file 
    rewind(inFilePtr);
    assembler(inFilePtr, outFilePtr);
    fclose(inFilePtr);
    fclose(outFilePtr);
    exit(0);
}

/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2) {
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Treat a blank line as end of file.
    // Arguably, we could just ignore and continue, but that could
    // get messy in terms of label line numbers etc.
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(size_t line_idx = 0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                ++line_char_is_whitespace;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            ++nonempty_line;
            break;
        }
    }
    if(nonempty_line == 0) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int isNumber(char *string) {
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}