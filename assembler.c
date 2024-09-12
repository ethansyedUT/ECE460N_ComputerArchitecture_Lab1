#include <stdio.h>  /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h>  /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

struct Symbol
{
    char *label;
    int addr;
};

// Contents
// Function prototype
void assembleFile(FILE *ASM, FILE *output, char *lLine,
                  char *lLabel, char *lOpcode, char *lArg1,
                  char *lArg2, char *lArg3, char *lArg4);

void printSymTable(struct Symbol *symbolTable, int currentTableSize);
int findinSymbolTable(struct Symbol *symbolTable, int currentTableSize, char *label);

int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4);
int isOpcode(char *label);
int getOpcode(char *str);
int isRegister(char *str);
int insToMachineCode();
void buildSymbolTable(FILE *ASM, struct Symbol *symbolTable, int *symTabSize, int *currentTabSize, char lLine[],
                      char *lLabel, char *lOpcode, char *lArg1,
                      char *lArg2, char *lArg3, char *lArg4);
int toNum(char *pStr);

enum opcodes
{
    add = 1,

        and = 5,

    br = 0,
    brn = 0,
    brz = 0,
    brp = 0,
    brzp = 0,
    brnz = 0,
    brnp = 0,
    brnzp = 0,

    nop = 0,

    halt = 15,

    jmp = 12,

    jsr = 4,
    jsrr = 4,

    ldb = 2,

    ldw = 6,

    lea = 14,

    not = 9,

    ret = 12,

    lshf = 13,
    rshfl = 13,
    rshfa = 13,

    rti = 8,

    stb = 3,

    stw = 7,

    trap = 15,

    xor = 9
};
enum psuedoOpcodes
{
    ORIG,
    FILL,
    BLKW,
    STRINGZ,
    END
};
#define MAX_LINE_LENGTH 255
enum
{
    DONE,
    OK,
    EMPTY_LINE
};

#define defSymTableSize 255

int main(int argc, char *argv[])
{

    char *prgName = NULL;
    prgName = argv[0];

    // open asm file for reading
    FILE *fileRead = fopen(argv[1], "r");
    FILE *outfile = fopen(argv[2], "w");

    if (!fileRead)
    {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile)
    {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }

    char lLine[MAX_LINE_LENGTH + 1],
        *lLabel, *lOpcode, *lArg1,
        *lArg2, *lArg3, *lArg4;

    assembleFile(fileRead, outfile, lLine,
                 lLabel, lOpcode, lArg1,
                 lArg2, lArg3, lArg4);

    fclose(fileRead);
    fclose(outfile);

    return 0;
}

void assembleFile(FILE *ASM, FILE *output, char lLine[MAX_LINE_LENGTH + 1],
                  char *lLabel, char *lOpcode, char *lArg1,
                  char *lArg2, char *lArg3, char *lArg4)
{

    struct Symbol *symbolTable = malloc(defSymTableSize * sizeof(struct Symbol));
    int realTableSize = defSymTableSize;

    int currentTableSize = 0;
    int addr = 0;

    // if more than 10 label found; amortized doubling is implemented
    // buildSymbolTable(ASM, symbolTable, &realTableSize, &currentTableSize, lLine, lLabel,
    //                  lOpcode, lArg1, lArg2, lArg3, lArg4);

    // first pass build symbol table
    enum nzp
    {
        n = 2048,
        z = 1024,
        p = 512,
    };
    int lRet;
    do
    {
        lRet = readAndParse(ASM, lLine, &lLabel,
                            &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
        if (lRet != DONE && lRet != EMPTY_LINE)
        {
            // set initial address
            if (strcmp(lOpcode, ".orig") == 0)
            {
                addr = toNum(lArg1);
            }
            // continue through asm
            else
            {
                addr++;
                // if label found add to symbol table
                if (strcmp(lLabel, "") != 0)
                {

                    char *temp = malloc(strlen(lLabel) + 1);
                    strcpy(temp, lLabel);
                    temp[strlen(lLabel)] = '\0';

                    struct Symbol label = {temp, addr};
                    symbolTable[currentTableSize] = label;
                    (currentTableSize)++;

                    // check if sym table at max capacity; amortize if needed
                    if (currentTableSize == realTableSize)
                    {
                        // deep copy and double size
                        struct Symbol *buffer = malloc(2 * (realTableSize) * sizeof(struct Symbol));

                        // Copy over original elements
                        for (int i = 0; i < (currentTableSize); i++)
                        {
                            struct Symbol temp = {strdup(symbolTable[i].label), symbolTable[i].addr};
                            buffer[i] = temp;
                            free(symbolTable[i].label);
                        }

                        // Initialize new elements
                        for (int i = currentTableSize; i < 2 * (realTableSize); i++)
                        {
                            struct Symbol temp = {NULL, -1};
                            buffer[i] = temp;
                        }
                        free(symbolTable);
                        symbolTable = buffer;
                        realTableSize *= 2;
                    }
                }
                // increment by word per instruction
            }
        }
    } while (lRet != DONE);

    lRet = OK;
    rewind(ASM);
    // second pass
    int addr2 = 0;
    do
    {
        lRet = readAndParse(ASM, lLine, &lLabel,
                            &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
        if (lRet != DONE && lRet != EMPTY_LINE)
        {
            //
            if (strcmp(lOpcode, ".orig") == 0)
            {
                addr2 = toNum(lArg1);
                fprintf(output, "0x%.4X\n", addr2);
            }
            else if (strcmp(lOpcode, ".fill") == 0)
            {
                addr2++;
                int temp = (toNum(lArg1) & 0xFFFF);
                fprintf(output, "0x%.4X\n", temp);
            }
            else if (strcmp(lOpcode, ".end") == 0)
            {
                break;
            }
            else
            {
                addr2++;
                // int converted = insToMachineCode();

                // gives first hex
                int ins = getOpcode(lOpcode);
                ins *= 4096;

                // ret
                if (strcmp(lOpcode, "ret") == 0)
                {
                    ins += 448;
                }
                else if (strcmp(lOpcode, "add") == 0 || strcmp(lOpcode, "and") == 0)
                {
                    if (strcmp(lArg1, "") != 0 && isRegister(lArg1) == 1)
                    {
                        int regNum = lArg1[1] - '0';
                        regNum *= 512;
                        ins += regNum;
                    }
                    if (strcmp(lArg2, "") != 0 && isRegister(lArg2) == 1)
                    {
                        int regNum = lArg2[1] - '0';
                        regNum *= 64;
                        ins += regNum;
                    }
                    int mask = ins;
                    if (isRegister(lArg3) == 1)
                    {
                        int regNum = lArg3[1] - '0';
                        ins += regNum;
                    }
                    else
                    {
                        int imVal5 = toNum(lArg3);

                        if (imVal5 < 0)
                        {
                            int insPreserve = ins;
                            int mask = 0xFFE0;

                            int invMask = ~0xFFE0;

                            insPreserve = insPreserve & mask;

                            ins += imVal5;
                            ins = ins & invMask;
                            ins = insPreserve | ins;
                        }
                        else
                        {
                            ins += imVal5;
                        }
                        ins = ins | (1 << 5);
                    }
                }
                else if (strcmp(lOpcode, "jmp") == 0 || strcmp(lOpcode, "jsrr") == 0)
                {
                    // doesn't check if reg / assumes
                    int regNum = lArg1[1] - '0';
                    regNum *= 64;
                    ins += regNum;
                }
                else if (strcmp(lOpcode, "jsr") == 0)
                {
                    ins += 2048;
                    int insPreserve = ins;
                    int loc = findinSymbolTable(symbolTable, currentTableSize, lArg1);
                    int calc11 = (loc - addr2) - 1;

                    if (calc11 < 0)
                    {
                        int insPreserve = ins;
                        int mask = 0xF800;

                        int invMask = ~0xF800;

                        insPreserve = insPreserve & mask;

                        ins += calc11;
                        ins = ins & invMask;
                        ins = insPreserve | ins;
                    }
                    else
                    {
                        ins += calc11;
                    }
                }
                else if (strcmp(lOpcode, "ldb") == 0 || strcmp(lOpcode, "ldw") == 0)
                {
                    if (strcmp(lArg1, "") != 0 && isRegister(lArg1) == 1)
                    {
                        int regNum = lArg1[1] - '0';
                        regNum *= 512;
                        ins += regNum;
                    }
                    if (strcmp(lArg2, "") != 0 && isRegister(lArg2) == 1)
                    {
                        int regNum = lArg2[1] - '0';
                        regNum *= 64;
                        ins += regNum;
                    }
                    if (strcmp(lArg3, "") != 0)
                    {
                        // is an immediate
                        int imVal6 = toNum(lArg3);
                        if (imVal6 < 0)
                        {
                            int insPreserve = ins;
                            int mask = 0xFFC0;

                            int invMask = ~0xFFC0;

                            insPreserve = insPreserve & mask;

                            ins += imVal6;
                            ins = ins & invMask;
                            ins = insPreserve | ins;
                        }
                        else
                        {
                            ins += imVal6;
                        }
                    }
                }
                else if (strcmp(lOpcode, "lea") == 0)
                {
                    if (strcmp(lArg1, "") != 0 && isRegister(lArg1) == 1)
                    {
                        int regNum = lArg1[1] - '0';
                        regNum = regNum << 9;
                        ins += regNum;
                    }

                    int insPreserve = ins;

                    int loc = findinSymbolTable(symbolTable, currentTableSize, lArg2);

                    int calc9 = ((loc - addr2)) - 1;
                    if (calc9 < 0)
                    {
                        int insPreserve = ins;
                        int mask = 0xFE00;

                        int invMask = ~0xFE00;

                        insPreserve = insPreserve & mask;

                        ins += calc9;
                        ins = ins & invMask;
                        ins = insPreserve | ins;
                    }
                    else
                    {
                        ins += calc9;
                    }
                }
                else if (strcmp(lOpcode, "not") == 0 || strcmp(lOpcode, "xor") == 0)
                {
                    if (strcmp(lArg1, "") != 0 && isRegister(lArg1) == 1)
                    {
                        int regNum = lArg1[1] - '0';
                        regNum *= 512;
                        ins += regNum;
                    }
                    if (strcmp(lArg2, "") != 0 && isRegister(lArg2) == 1)
                    {
                        int regNum = lArg2[1] - '0';
                        regNum *= 64;
                        ins += regNum;
                    }
                    if (strcmp(lOpcode, "not") == 0)
                    {
                        ins += 63;
                    }
                    else if (strcmp(lOpcode, "xor") == 0 && isRegister(lArg3) == 1)
                    {
                        int regNum = lArg3[1] - '0';
                        ins += regNum;
                    }
                    else if (strcmp(lOpcode, "xor") == 0)
                    {
                        int imVal5 = toNum(lArg3);
                        if (imVal5 < 0)
                        {
                            int insPreserve = ins;
                            int mask = 0xFFE0;

                            int invMask = ~0xFFE0;

                            insPreserve = insPreserve & mask;

                            ins += imVal5;
                            ins = ins & invMask;
                            ins = insPreserve | ins;
                        }
                        else
                        {
                            ins += imVal5;
                        }
                        ins = ins | (1 << 5);
                    }
                }
                else if (strcmp(lOpcode, "lshf") == 0 || strcmp(lOpcode, "rshfl") == 0 || strcmp(lOpcode, "rshfa") == 0)
                {
                    if (strcmp(lArg1, "") != 0 && isRegister(lArg1) == 1)
                    {
                        int regNum = lArg1[1] - '0';
                        regNum *= 512;
                        ins += regNum;
                    }
                    if (strcmp(lArg2, "") != 0 && isRegister(lArg2) == 1)
                    {
                        int regNum = lArg2[1] - '0';
                        regNum *= 64;
                        ins += regNum;
                    }

                    if (strcmp(lOpcode, "rshfl") == 0)
                    {
                        ins += 16;
                    }
                    else if (strcmp(lOpcode, "rshfa") == 0)
                    {
                        ins += 48;
                    }

                    if (strcmp(lArg3, "") != 0)
                    {
                        // is an immediate

                        int imVal9 = toNum(lArg3);
                        if (imVal9 < 0)
                        {
                            int insPreserve = ins;
                            int mask = 0xFE00;

                            int invMask = ~0xFE00;

                            insPreserve = insPreserve & mask;

                            ins += imVal9;
                            ins = ins & invMask;
                            ins = insPreserve | ins;
                        }
                        else
                        {
                            ins += imVal9;
                        }
                    }
                }
                else if (strcmp(lOpcode, "stb") == 0 || strcmp(lOpcode, "stw") == 0)
                {
                    if (strcmp(lArg1, "") != 0 && isRegister(lArg1) == 1)
                    {
                        int regNum = lArg1[1] - '0';
                        regNum *= 512;
                        ins += regNum;
                    }
                    if (strcmp(lArg2, "") != 0 && isRegister(lArg2) == 1)
                    {
                        int regNum = lArg2[1] - '0';
                        regNum *= 64;
                        ins += regNum;
                    }
                    if (strcmp(lArg3, "") != 0)
                    {
                        // is an immediate
                        int imVal6 = toNum(lArg3);
                        if (imVal6 < 0)
                        {
                            int insPreserve = ins;
                            int mask = 0xFFC0;

                            int invMask = ~0xFFC0;

                            insPreserve = insPreserve & mask;

                            ins += imVal6;
                            ins = ins & invMask;
                            ins = insPreserve | ins;
                        }
                        else
                        {
                            ins += imVal6;
                        }
                    }
                }
                else if (strcmp(lOpcode, "trap") == 0 || strcmp(lOpcode, "halt") == 0)
                {
                    // if there is an arg1
                    if (strcmp(lArg1, "") != 0)
                    {
                        ins += toNum(lArg1);
                    }
                    else
                    {
                        ins += toNum("x25");
                    }
                }
                else if (strcmp(lOpcode, "br") == 0 || strcmp(lOpcode, "brn") == 0 || strcmp(lOpcode, "brz") == 0 || strcmp(lOpcode, "brp") == 0 ||
                         strcmp(lOpcode, "brnz") == 0 || strcmp(lOpcode, "brnp") == 0 || strcmp(lOpcode, "brzp") == 0 || strcmp(lOpcode, "brnzp") == 0)
                {
                    if (strcmp(lOpcode, "br") == 0)
                    {
                        ins += n;
                        ins += z;
                        ins += p;
                    }
                    if (strchr(lOpcode, 'n') != NULL)
                    {
                        ins += n;
                    }
                    if (strchr(lOpcode, 'z') != NULL)
                    {
                        ins += z;
                    }
                    if (strchr(lOpcode, 'p') != NULL)
                    {
                        ins += p;
                    }

                    int insPreserve = ins;
                    int loc = findinSymbolTable(symbolTable, currentTableSize, lArg1);
                    int calc9 = ((loc - addr2)) - 1;

                    if (calc9 < 0)
                    {
                        int insPreserve = ins;
                        int mask = 0xFE00;

                        int invMask = ~0xFE00;

                        insPreserve = insPreserve & mask;

                        ins += calc9;
                        ins = ins & invMask;
                        ins = insPreserve | ins;
                    }
                    else
                    {
                        ins += calc9;
                    }
                }
                // nop processed as x0000 automatically

                fprintf(output, "0x%.4X\n", ins);
            }
        }
    } while (lRet != DONE);

    // printSymTable(symbolTable, currentTableSize);

    free(symbolTable);
}

int findinSymbolTable(struct Symbol *symbolTable, int currentTableSize, char *label)
{
    for (int i = 0; i < currentTableSize; i++)
    {
        if (strcmp(symbolTable[i].label, label) == 0)
        {
            return symbolTable[i].addr;
        }
    }
    return -1;
}

int getOpcode(char *str)
{
    if (strcmp(str, "and") == 0)
    {
        return and;
    }
    else if (strcmp(str, "add") == 0)
    {
        return add;
    }
    else if (strcmp(str, "br") == 0)
    {
        return br;
    }
    else if (strcmp(str, "brn") == 0)
    {
        return brn;
    }
    else if (strcmp(str, "brz") == 0)
    {
        return brz;
    }
    else if (strcmp(str, "brp") == 0)
    {
        return brp;
    }
    else if (strcmp(str, "brzp") == 0)
    {
        return brzp;
    }
    else if (strcmp(str, "brnz") == 0)
    {
        return brnz;
    }
    else if (strcmp(str, "brnp") == 0)
    {
        return brnp;
    }
    else if (strcmp(str, "brnzp") == 0)
    {
        return brnzp;
    }
    else if (strcmp(str, "halt") == 0)
    {
        return halt;
    }
    else if (strcmp(str, "jmp") == 0)
    {
        return jmp;
    }
    else if (strcmp(str, "jsr") == 0)
    {
        return jsr;
    }
    else if (strcmp(str, "jsrr") == 0)
    {
        return jsrr;
    }
    else if (strcmp(str, "ldb") == 0)
    {
        return ldb;
    }
    else if (strcmp(str, "ldw") == 0)
    {
        return ldw;
    }
    else if (strcmp(str, "lea") == 0)
    {
        return lea;
    }
    else if (strcmp(str, "nop") == 0)
    {
        return nop;
    }
    else if (strcmp(str, "not") == 0)
    {
        return not ;
    }
    else if (strcmp(str, "ret") == 0)
    {
        return ret;
    }
    else if (strcmp(str, "lshf") == 0)
    {
        return lshf;
    }
    else if (strcmp(str, "rshfl") == 0)
    {
        return rshfl;
    }
    else if (strcmp(str, "rshfa") == 0)
    {
        return rshfa;
    }
    else if (strcmp(str, "rti") == 0)
    {
        return rti;
    }
    else if (strcmp(str, "stb") == 0)
    {
        return stb;
    }
    else if (strcmp(str, "stw") == 0)
    {
        return stw;
    }
    else if (strcmp(str, "trap") == 0)
    {
        return trap;
    }
    else if (strcmp(str, "xor") == 0)
    {
        return xor;
    }
    else
    {
        return -1;
    }
}

void printSymTable(struct Symbol *symbolTable, int currentTableSize)
{
    for (int i = 0; i < currentTableSize; i++)
    {
        printf("\nSymbol : %s\n", symbolTable[i].label);
        printf("Addr : %d\n", symbolTable[i].addr);
    }
}

int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4)
{
    char *lRet, *lPtr;
    int i;
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
        return (DONE);
    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);

    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while (*lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n')
        lPtr++;

    *lPtr = '\0';
    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return (EMPTY_LINE);

    if (isOpcode(lPtr) == -1 && lPtr[0] != '.') /* found a label */
    {
        *pLabel = lPtr;
        if (!(lPtr = strtok(NULL, "\t\n ,")))
            return (OK);
    }

    *pOpcode = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg1 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg2 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg3 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg4 = lPtr;

    return (OK);
}

int insToMachineCode()
{

    // opcode multiply by 4096
    // operand multiply by however many to left shift
    return 0;
}
int isOpcode(char *label)
{
    char *Opcodes[] = {
        "add", "and", "br", "brn", "brz", "brp", "brzp",
        "brnz", "brnp", "brnzp", "halt", "jmp",
        "jsr", "jsrr", "ldb", "ldw", "lea", "nop",
        "not", "ret", "lshf", "rshfl", "rshfa", "rti",
        "stb", "stw", "trap", "xor"};
    int opcodeNum = sizeof(Opcodes) / sizeof(char *);
    int found = -1;

    for (int i = 0; i < opcodeNum; i++)
    {
        if (strcmp(label, Opcodes[i]) == 0)
        {
            found = 1;
        }
    }

    return found;
}
int isRegister(char *str)
{
    char *reg[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    int regNum = sizeof(reg) / sizeof(char *);
    int found = -1;

    for (int i = 0; i < regNum; i++)
    {
        if (strcmp(str, reg[i]) == 0)
        {
            found = 1;
        }
    }
    return found;
}
// input : decimal or hex string
// output : int conversion
int toNum(char *pStr)
{
    char *t_ptr;
    char *orig_pStr;
    int t_length, k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if (*pStr == '#') /* decimal */
    {
        pStr++;
        if (*pStr == '-') /* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++)
        {
            if (!isdigit(*t_ptr))
            {
                printf("Error: invalid decimal operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    }
    else if (*pStr == 'x') /* hex     */
    {
        pStr++;
        if (*pStr == '-') /* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++)
        {
            if (!isxdigit(*t_ptr))
            {
                printf("Error: invalid hex operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16); /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
        if (lNeg)
            lNum = -lNum;
        return lNum;
    }
    else
    {
        printf("Error: invalid operand, %s\n", orig_pStr);
        exit(4); /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}
