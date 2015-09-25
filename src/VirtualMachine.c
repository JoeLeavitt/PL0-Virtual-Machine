/* Joseph Leavitt
 * Main program for the PM/0 VM
 * 09/25/2015
 */

#include "VirtualMachine.h"

int main(int argc, char *argv[])
{
    // Initialize read and write files
    FILE *ifp = fopen(argv[1], "r");
    FILE *ofp = fopen(argv[2], "w+");

    // Exit program if a file was not opened
    if(ifp == NULL) { printf("Input File Not Found\n");  return 0; }
    if(ofp == NULL) { printf("Output File Not Found\n"); return 0; }

    // Register architecture
    int BP = 1; // Base Pointer
    int SP = 0; // Stack Pointer
    int PC = 0; // Program Counter
    instruction *IR; // Instructions (Opcode, Lexi. Level, Parameter)
    int prev;

    // Initialize stack
    int stack[MAX_STACK_HEIGHT];
    memset(stack, 0, MAX_STACK_HEIGHT * sizeof(int));

    // Create and intialize an array of structs
    instruction instructions[MAX_CODE_LENGTH];
    read(ifp, instructions);

    // Close input file
    fclose(ifp);

    // Print the instructions to be executed
    printInstructions(ofp, instructions);

    // Print header for the stacktrace
    fprintf(ofp, "\n\n\t\t\t\t\tPC\tBP\tSP\tStack \nInitial Values \t\t\t\t\t%d\t%d\t%d\n", 0, 1, stack[0]);

    printf("Fetch -> Execute Cycle\n");

    // Fetch -> Execute Cycle
    bool halt = false;
    while(halt == false)
    {
        // Get instruction
        IR = &instructions[PC];

        // Save last program counter
        prev = PC;

        // Set instructions for cleaner usage
        int OP = IR->OP;
        int L = IR->L;
        int M = IR->M;

        switch(OP)
        {
            // LIT - Push value M onto the stack
            case 1:
                SP++;
                stack[SP] = M;
                PC++;
                break;
            /* OPR - Return from a procedure call
             *       or do an ALU op, specified by M */
            case 2:
                switch(M)
                {
                    // Return from a procedure call
                    case 0:
                       SP = BP - 1;
                       PC = stack[SP + 4];
                       BP = stack[SP + 3];
                        break;
                    // NEG - Pop once -> push the negation of the result
                    case 1:
                        stack[SP] = -stack[SP];
                        break;
                    // ADD - Pop twice -> add -> push
                    case 2:
                        SP--;
                        stack[SP] = stack[SP] + stack[SP + 1];
                        break;
                    // SUB - Pop twice -> subtract top value from bottom value -> push
                    case 3:
                        SP--;
                        stack[SP] = stack[SP] - stack[SP + 1];
                        break;
                    // MUL - Pop twice -> multiply -> push
                    case 4:
                        SP--;
                        stack[SP] = stack[SP] * stack[SP + 1];
                        break;
                    // DIV - Pop twice -> divide top value from bottom value -> push
                    case 5:
                        SP--;
                        stack[SP] = stack[SP] / stack[SP + 1];
                        break;
                    // ODD - Pop once -> push 1 if odd and 0 if even
                    case 6:
                        stack[SP] = stack[SP] % 2;
                        break;
                    // MOD - Pop twice -> mod top value from bottom value -> push
                    case 7:
                        SP--;
                        stack[SP] = stack[SP] % stack[SP + 1];
                        break;
                    // EQL - Pop twice -> push 1 if first = second, 0 otherwise
                    case 8:
                        SP--;
                        stack[SP] = stack[SP] == stack[SP + 1];
                    // NEQ - Pop twice -> push 1 if first does not equal second, 0 otherwise
                    case 9:
                        SP--;
                        stack[SP] = stack[SP] != stack[SP + 1];
                        break;
                    // LSS - Pop twice -> push 1 if second < first, 0 otherwise
                    case 10:
                        SP--;
                        stack[SP] = stack[SP] < stack[SP + 1];
                        break;
                    // LEQ - Pop twice -> push 1 if second <= first, 0 otherwise
                    case 11:
                        SP--;
                        stack[SP] = stack[SP] <= stack[SP + 1];
                        break;
                    // GTR - Pop twice -> push 1 if second > first, 0 otherwise
                    case 12:
                        SP--;
                        stack[SP] = stack[SP] > stack[SP + 1];
                    // GEQ - Pop twice -> push 1 if second >= first, 0 otherwise
                    case 13:
                        SP--;
                        stack[SP] = stack[SP] >= stack[SP + 1];
                }
                break;
            // LOD - Read the value at offset M, L levels down, and push onto stack
            case 3:
                SP++;
                stack[SP] = stack[base(L, BP, stack) + M];
                PC++;
                break;
            // STO - Pop the stack and write value into offset M, L levels down
            case 4:
                stack[base(L, BP, stack) + M] = stack[SP];
                stack[SP] = 0;
                SP--;
                PC++;
                break;
            // CAL - Call the procedure at M
            case 5:
                stack[SP + 1] = 0;
                stack[SP + 2] = base(L, BP, stack);
                stack[SP + 3] = BP;
                stack[SP + 4] = PC + 1;
                BP = SP + 1;
                PC = M;
                break;
            // INC - Allocate space for M local variables, will always allocate atleast 4
            case 6:
                SP = SP + M;
                PC++;
                break;
            // JMP - Branch to M
            case 7:
                PC = M;
                break;
            // JPC - Pop the stack and branch to M if result is 0
            case 8:
                if(stack[SP] == 0)
                    PC = M;
                else
                    PC++;
                SP--;
                break;
            // SIO 1 - Pop the stack and write result to screen
            case 9:
                printf("%d\n", stack[SP]);
                SP--;
                PC++;
                break;
            // SIO 2 - Take user input and push on the stack
            case 10:
                printf("Enter Value:\n");
                SP++;
                scanf("%d", &stack[SP]);
                PC++;
                break;
            // SIO 3 - Terminate
            case 11:
                printf("Halt\n");
                halt = true;
                BP = 0;
                PC = 0;
                SP = 0;
                break;
            default:
                printf("Invalid Opcode\n");
                return 0;
        }

        // Print the stacktrace for the instructions that were just executed
        printStacktrace(ofp, prev, IR, PC, BP, SP, stack);
    }

    // Close
    fclose(ofp);

    return 0;
}

/* Insert instructions into struct array */
void read(FILE *ifp, instruction *instructions)
{
    int i = 0;
    char instructionRow[MAX_LINE_LENGTH];

    // Reads the instructions line by line as a string then casts them as integers
    while(fgets(instructionRow, MAX_LINE_LENGTH, (FILE*)ifp) != NULL)
    {
         instructions[i].OP = (int)atoi((char*)strtok(instructionRow, " "));
         instructions[i].L = (int)atoi((char*)strtok(NULL, " "));
         instructions[i].M = (int)atoi((char*)strtok(NULL, " "));
         i++;
    }
}

/* Print the instructions to be performed to the output file */
void printInstructions(FILE *ofp, instruction* instructions)
{
    int currLine = 0;
    char* codeString = malloc(MAX_CODE_LENGTH);

    sprintf(codeString, "%s", "Line\tOP\tL\tM\n");

    bool halt = false;
    while(halt == false)
    {
        // Check for halt instruction (SIO 0, 3), if found break out of the loop
        if(instructions[currLine].OP == 11 && instructions[currLine].M == 3)
            halt = true;

        sprintf(codeString + strlen(codeString), "%d\t%s\t%d\t%d\n",
                currLine,
                getOpcode(instructions[currLine].OP),
                instructions[currLine].L,
                instructions[currLine].M)
            ;
        currLine++;
    }

    fprintf(ofp, "%s", codeString);
}

/* Print the stacktrace to the outputfile */
void printStacktrace(FILE *ofp, int prevPC, instruction *IR, int PC, int BP, int SP, int *stack)
{
    int i;
    char *stacktraceLine = malloc(MAX_CODE_LENGTH);

    sprintf(stacktraceLine + strlen(stacktraceLine), "%d\t%s\t%d\t%d\t%d\t%d\t%d\t",
            prevPC, getOpcode(IR->OP), IR->L, IR->M, PC, BP, SP);

    for(i = 1; i <= SP; i++)
    {
        if(i == BP && BP != 1)
            sprintf(stacktraceLine + strlen(stacktraceLine), "| ");

        sprintf(stacktraceLine + strlen(stacktraceLine), "%d ", stack[i]);
    }

    sprintf(stacktraceLine + strlen(stacktraceLine), "\n");

    fprintf(ofp, "%s", stacktraceLine);
}

/* Find the base */
int base(int L, int BP, int stack[])
{
    int baseLevel;
    baseLevel = BP;
    while(L > 0)
     {
         baseLevel = stack[baseLevel + 1];
         L--;
     }
     return baseLevel;
}

/* Returns the Opcode name corresponding to the Opcode number */
char *getOpcode(int OP)
{
    switch(OP)
    {
        case 1:
            return "lit";
        case 2:
            return "opr";
        case 3:
            return "lod";
        case 4:
            return "sto";
        case 5:
            return "cal";
        case 6:
            return "inc";
        case 7:
            return "jmp";
        case 8:
            return "jpc";
        case 9:
            return "sio";
        case 10:
            return "sio";
        case 11:
            return "sio";
        default:
            return "Something Went Wrong";
    }
}
