#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_HEIGHT 200
#define MAX_CODE_LENGTH 500
#define MAX_LINE_LENGTH 10
#define MAX_LEXI_LEVEL 3

typedef struct Execute_Cycle_Instructions
{
    int OP; // Opcode
    int L;  // Lexicographical Level
    int M;  // Parameter
} instruction;

void read(FILE *ifp, instruction *instructions);
void printInstructions(FILE *ofp, instruction* instructions);
void printStacktrace(int prevPC, instruction *IR, int PC, int BP, int SP, int *stack, FILE *ofp)
int base(int l, int base, int *stack);
char *getOpcode(int OP);
