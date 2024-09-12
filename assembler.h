#ifndef MYHEADER_H
#define MYHEADER_H

#include <stdio.h>

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

#endif