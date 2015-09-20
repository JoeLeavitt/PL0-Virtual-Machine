#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Struct representing the Execute cycle */
typedef struct instructions
{
    int OP; // Opcode
    int L;  // Lexicographical Level
    int M;  // Parameter
    int R;  // Register
} instructions;

/* Function Definitions */
void printCodes(FILE *ofp, int code[][MAX_CODE_LENGTH], int codeLen);
instructions fetch(int PC, int code[][MAX_CODE_LENGTH]);
int base(int l, int base, int *stack);
char *getOpcode(int OP);

