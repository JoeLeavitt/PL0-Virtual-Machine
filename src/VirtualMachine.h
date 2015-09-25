/* Joseph Leavitt
 * Header File for the PM/0 VM
 * 9/25/2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
void printStacktrace(FILE *ofp, int prev, instruction *IR, int PC, int BP, int SP, int *stack);
int base(int L, int BP, int stack[]);
char *getOpcode(int OP);
