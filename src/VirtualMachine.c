/* Joseph Leavitt
 * PM/0 Virtual Machine
 * 09/20/15
 */

#include "VirtualMachine.h"
#include <stdbool.h>

int main(int argc, char *argv[1])
{
    // Initialize read and write files
    FILE *ifp = fopen(argv[1], "r");
    FILE *ofp = fopen("mcode_stacktrace.txt", "w+");

    // Register architecture
    int BP = 1; // Base Pointer
    int SP = 0; // Stack Pointer
    int PC = 0; // Program Counter
    instruction *IR;

     // Create and intialize an array of structs
    instruction instructions[MAX_CODE_LENGTH];
    read(ifp, instructions);

    int data[MAX_STACK_HEIGHT];
    int activationRecord;

    // Initialize stack
    int stack[MAX_STACK_HEIGHT];
    memset(stack, 0, sizeof(int));

    // Exit program if a file was not opened
    if(ifp == NULL)
        printf("File Not Found");  return 0;

    // Close input file
    fclose(ifp);

    // Print the instructions to be executed
    printInstructions(ofp, instructions);

    // Print headers for the stacktrace
    fprintf(ofp, "\n\n\t\t\t\t\tPC\tBP\tSP\tStack \nInitial Values \t\t\t\t%d\t%d\t%d\t%d\n", PC, BP, SP, stack[0]);

    // Fetch -> Execute Cycle
    bool halt = false;
    while(halt == false)
    {
        int prevPC = PC;

        // Get instruction
        IR = &instructions[PC];

        // Set instructions for cleaner usage
        int OP = IR->OP;
        int L = IR->L;
        int M = IR->M;

        switch(OP)
        {
            // LIT - Push value M onto the stack
            case 1:
                SP += 1;
                stack[SP] = M;
                break;
            /* OPR - Return from a procedure call
             *       or do an ALU op, specified by M */
            case 2:
                switch(M)
                {
                    // Return from a procedure call
                    case 0:
                       // if(BP == 1) return 0;
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
                    case 8:
                        SP--;
                        stack[SP] = stack[SP] == stack[SP + 1];
                    case 9:
                        SP--;
                        stack[SP] = stack[SP] != stack[SP + 1];
                        break;
                    case 10:
                        SP--;
                        stack[SP] = stack[SP] < stack[SP + 1];
                        break;
                    case 11:
                        SP--;
                        stack[SP] = stack[SP] <= stack[SP + 1];
                        break;
                    case 12:
                        SP--;
                        stack[SP] = stack[SP] > stack[SP + 1];
                    case 13:
                        SP--;
                        stack[SP] = stack[SP] >= stack[SP + 1];
                }
                break;
            // LOD - Read the value at offset M, L levels down, and push onto stack
            case 3:
                SP += 1;
                stack[SP] = stack[L + M];
                break;
            // STO - Pop the stack and write value into offset M, L levels down
            case 4:
                stack[L + M] = stack[SP];
                SP -= 1;
            // CAL - Call the procedure at M
            case 5:
                stack[SP + 1] = 0;
               // stack[SP + 2] = base();
                stack[SP + 3] = BP;
                stack[SP + 4] = PC;
                BP = SP + 1;
                PC = M;
                break;
            // INC - Allocate space for M local variables, will always allocate atleast 4
            case 6:
                SP = SP + M;
                break;
            // JMP - Branch to M
            case 7:
                PC = M;
                break;
            // JPC - Pop the stack and branch to M if result is 0
            case 8:
                break;
            // SIO 1 - Pop the stack and write result to screen
            case 9:
                printf("%d\n", stack[SP]);
                SP += 1;
                break;
            // SIO 2 - Take user input and push on the stack
            case 10:
                printf("SIO 2 instruction\nplease enter Value:\n");
                scanf("%d", &stack[++SP]);
                break;
            // SIO 3 - Terminate
            case 11:
                halt = true;
                break;
            default:
                printf("Invalid Opcode\n");
                return 0;
        }

        PC += 1;

        printStacktrace(prevPC, IR, PC, BP, SP, stack, ofp);
    }

    // Close output file
    fclose(ofp);

    return 0;
}

void read(FILE *ifp, instruction *instructions)
{
    int i = 0;
    char instructionRow[MAX_LINE_LENGTH];

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
    char codeString[MAX_CODE_LENGTH];

    sprintf(codeString, "%s", "Line\tOP\tL\tM\n");

    bool halt = false;
    while(halt == false)
    {
        int currLine = 0;
        sprintf(codeString + strlen(codeString), "%d\t%s\t%d\t%d\n",
                currLine,
                getOpcode(instructions[currLine].OP),
                instructions[currLine].L,
                instructions[currLine].M);
        currLine++;

        // Check for halt instruction (SIO 0, 3), if found break out of the loop
        if(instructions[currLine].OP == 1 && instructions[currLine].M == 3)
            halt = true;
    }

    fprintf(ofp, "%s", codeString);
}

/* Print the stacktrace to the outputfile */
void printStacktrace(int prevPC, instruction *IR, int PC, int BP, int SP, int *stack, FILE *ofp)
{
    int i;
    char stacktraceLine[200];

    sprintf(stacktraceLine, "%d\t%s\t%d\t%d\t%d\t%d\t%d\t",
            prevPC, getOpcode(IR->OP), IR->L, IR->M, PC, BP, SP);

    for(i = 1; i <= SP; i++)
        sprintf(stacktraceLine, "%d ", stack[i]);

    fprintf(ofp, "%s", stacktraceLine);
}

/* Find the base */
// Not sure if im going to use this yet
int base(int l, int base, int *stack)
{
     int b1 = base;
     while(1 > 0)
     {
         b1 = stack[b1 + 1];
         l--;
     }
     return b1;
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
            return "inv";
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
