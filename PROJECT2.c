#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef VALID
#define VALID 1
#endif
#ifndef INVALID
#define INVALID 0
#endif
#ifndef max_instructions
#define max_instructions 100
#endif

long data_Memory[1000];
int pc = 0;
int instr_line_Number = 0;

void iq();
void LSQ();
void ROB();

int prf_available();
int find_new_prf();
void old_instance_prf(int );
int find_existing_prf(int, int);

void FETCH_STAGE();
void DECODE_RF_STAGE();
void INT1_FU_STAGE();
void INT2_FU_STAG();
void memory();
void mul1();
void mul2();
void mul3();
void bz_fu();
void intialize();
void simulate(char file_name[]);
void display();



//ARF structure
typedef struct {
    long value;
    int status;
    int ins_id;
    int prf_id;
}registers;

registers arch_Reg_File[16] = {};

//PRF structure
typedef struct {
    long value;
    int status;
    int ins_id;
    int arf_id;
    int busy;
    int old_instance;
}prf;

prf physical_Reg_File[24];

//Instructions structure
typedef struct {

  int address;
  char opcode[25];
  int src1;
  int src2;
  int dest;
  int literal;
  long result;
  int index;
  int branch;
  int id;
  int status;
} Instructions;

Instructions instruction[max_instructions];
Instructions *ptr_instruction = instruction;

Instructions iqueue[12];
Instructions rob[12];
Instructions lsq[6];

int lst_arithm_index = -1;
int lst_arithm_instruction = -1;
long lst_arithm_resultset = -10;

const Instructions nop = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions fetch_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions decode_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions int_fun1_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions int_fun2_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions memory_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions mul_fun1_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions mul_fun2_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions mul_fun3_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};
Instructions branch_fun_input = {0, "nop", 0, 0, -1, 0, 0, 0, 0, 0, 0};

int ind = 0;
int bflag = 0;
int hflag = 0;
int mflag = 0;
int dflag = 0;
int id = 1;
int ch = 0;
int bzflag = 0;
int jflag = 0;
int jalflag = 0;

int dummy1 = 0;
int dummy2 = 0;
int dummy3 = 0;

int iq_add_index = 0;
int iq_rem_index = 0;
int iq_full_index = 0;

int rob_add_index = 0;
int rob_com_index = 0;
int rob_full_index = 0;

int lsq_add_index = 0;
int lsq_rem_index = 0;
int lsq_full_index = 0;

int main(){

    char file_name[20];

    while (1)
    {
        printf("\n Enter '1' for 'INITIALIZATION', '2' to 'SIMULATE', '3' to 'DISPLAY', and '0' to TERMINATE\n\t");
        scanf("%d", &ch);

        if (ch == 1)
            intialize();
        else if (ch == 2)
        {
            printf("\n Enter the filename you want to simulate : \t ");
            scanf("%s", file_name);
            simulate(file_name);
        }
        else if (ch == 3)
            display();
        else if (ch == 0)
            break;
        else
            printf("\n Enter a Valid Command");
    }
}

void simulate(char file_name[])
{

    printf("\n\t Enter the no. of clock cycles: ");
    int cycles = 0;
    scanf("%d", &cycles);

    FILE *ptr_File = fopen (file_name,"r");

    char line[255];

    while( feof( ptr_File ) == 0)
    {

        fgets (line, 255, ptr_File);
        printf("\n%d", instr_line_Number);
        printf("\t%s", line);
        instr_line_Number++;
        sscanf(line,"%[^,]", ptr_instruction->opcode);

        if (!(strcmp(ptr_instruction->opcode, "MOVC"))){
            sscanf(line, "%[^,],R%d,#%d", ptr_instruction->opcode, &ptr_instruction->dest, &ptr_instruction->literal);
            ptr_instruction->id = id;
            id++;
            printf("\t%s R%d %d", ptr_instruction->opcode, ptr_instruction->dest, ptr_instruction->literal);
        }
        else if ((!(strcmp(ptr_instruction->opcode, "MUL")) || !(strcmp(ptr_instruction->opcode, "ADD")) || !(strcmp(ptr_instruction->opcode, "SUB")) || !(strcmp(ptr_instruction->opcode, "OR")) || !(strcmp(ptr_instruction->opcode, "AND")) || !(strcmp(ptr_instruction->opcode, "EX-OR")))){
            sscanf(line, "%[^,],R%d,R%d,R%d", ptr_instruction->opcode, &ptr_instruction->dest, &ptr_instruction->src1, &ptr_instruction->src2);
            printf("\t%s R%d R%d R%d", ptr_instruction->opcode, ptr_instruction->dest, ptr_instruction->src1, ptr_instruction->src2);
            ptr_instruction->id = id;
            id++;
        }
        else if (!(strcmp(ptr_instruction->opcode, "ADDL"))){
            sscanf(line, "%[^,],R%d,R%d,#%d", ptr_instruction->opcode, &ptr_instruction->dest, &ptr_instruction->src1, &ptr_instruction->literal);
            printf("\t%s R%d R%d %d", ptr_instruction->opcode, ptr_instruction->dest, ptr_instruction->src1, ptr_instruction->literal);
            ptr_instruction->id = id;
            id++;
        }
        else if (!(strcmp(ptr_instruction->opcode, "SUBL"))){
            sscanf(line, "%[^,],R%d,R%d,#%d", ptr_instruction->opcode, &ptr_instruction->dest, &ptr_instruction->src1, &ptr_instruction->literal);
            printf("\t%s R%d R%d %d", ptr_instruction->opcode, ptr_instruction->dest, ptr_instruction->src1, ptr_instruction->literal);
            ptr_instruction->id = id;
            id++;
        }
        else if (!(strcmp(ptr_instruction->opcode, "LOAD"))){
            sscanf(line, "%[^,],R%d,R%d,#%d", ptr_instruction->opcode, &ptr_instruction->dest, &ptr_instruction->src1, &ptr_instruction->literal);
            printf("\t%s R%d R%d %d", ptr_instruction->opcode, ptr_instruction->dest, ptr_instruction->src1, ptr_instruction->literal);
            ptr_instruction->id = id;
            id++;
        }
        else if (!(strcmp(ptr_instruction->opcode, "STORE"))){
            sscanf(line, "%[^,],R%d,R%d,#%d", ptr_instruction->opcode, &ptr_instruction->src1, &ptr_instruction->src2, &ptr_instruction->literal);
            printf("\t%s R%d R%d %d", ptr_instruction->opcode, ptr_instruction->dest, ptr_instruction->src1, ptr_instruction->literal);
            ptr_instruction->id = id;
            id++;
        }
        else if (!(strcmp(ptr_instruction->opcode, "HALT"))){
            sscanf(line, "%[^,]", ptr_instruction->opcode);
            printf("\t%s", ptr_instruction->opcode);
            ptr_instruction->id = id;
            id++;
            ptr_instruction->dest = -1;
        }
        else if (!(strcmp(ptr_instruction->opcode, "JUMP"))){
            sscanf(line, "%[^,],R%d,#%d", ptr_instruction->opcode, &ptr_instruction->src1, &ptr_instruction->literal);
            printf("\t%s R%d %d", ptr_instruction->opcode, ptr_instruction->src1, ptr_instruction->literal);
            ptr_instruction->id = id;
            id++;
            ptr_instruction->dest = -1;
        }
        else if (!(strcmp(ptr_instruction->opcode, "BZ"))){
            sscanf(line, "%[^,],#%d", ptr_instruction->opcode, &ptr_instruction->literal);
            printf("\t%s %d", ptr_instruction->opcode, ptr_instruction->literal);
            ptr_instruction->id = id;
            id++;
            ptr_instruction->dest = -1;
        }
        else{
         printf("\t%s", "Not a valid opcode");
       }
       ptr_instruction++;

    }

    fclose(ptr_File);
    for (int i = 1; i <= cycles; i++)
    {
        printf("\n--------------------------Cycle No. = %d-------------------------", i);
        ROB();
        ROB();

        if (mflag == 1)
        {
            mul3();
            mul2();
            mul1();
        }
        bz_fu();
        INT2_FU_STAG();
        INT1_FU_STAGE();
        iq();
        iq();
        memory();
        LSQ();
        DECODE_RF_STAGE();
        FETCH_STAGE();
        if (hflag == 100)
            break;
    }
}


void FETCH_STAGE(){
  if((pc <= instr_line_Number) && hflag == 0 && bzflag == 0 && jflag == 0 && jalflag == 0)
  {
    fetch_input = instruction[pc];
    if (!(strcmp(fetch_input.opcode, "MOVC")))
    {
        if(!(strcmp(decode_input.opcode, "nop")))
        {
            if (bflag == 0)
            {
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d %d", fetch_input.opcode, fetch_input.dest, fetch_input.literal);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n FETCH_STAGE : \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d %d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.literal);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "ADD")))
    {
        if(!(strcmp(decode_input.opcode, "nop")))
        {
            if (bflag == 0)
            {
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d ", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n FETCH_STAGE : \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "SUB")))
    {
        if(!(strcmp(decode_input.opcode, "nop")))
        {
            if (bflag == 0)
            {
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n FETCH_STAGE : \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "AND")))
    {
        if(!(strcmp(decode_input.opcode, "nop")))
        {
            if (bflag == 0)
            {
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n FETCH_STAGE : \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "MUL")))
    {

      //printf("i am in multiplication n" );
        if(!(strcmp(decode_input.opcode, "nop")))
        {
            if (bflag == 0)
            {
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d ", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n FETCH_STAGE : \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "LOAD")))
    {
        if(!(strcmp(decode_input.opcode, "nop"))){
            if (bflag == 0){
                printf("\n Instruction at FETCH_STAGE ---> \t\t %s R%d R%d %d", fetch_input.opcode, fetch_input.dest, fetch_input.src1, fetch_input.literal);
                pc++;
                decode_input = fetch_input;
                fetch_input = nop;
            }
            else{
                bflag = 0;
                printf("\n Instruction at FETCH_STAGE --->: \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t\t %s R%d R%d %d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1, fetch_input.literal);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "STORE")))
    {
        if(!(strcmp(decode_input.opcode, "nop"))){
            if (bflag == 0){
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d %d", fetch_input.opcode, fetch_input.src1, fetch_input.src2, fetch_input.literal);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n FInstruction at FETCH_STAGE ---> \t\t idle");
            }
        }
        else{
            printf("\n Fetch stage : \t\t %s R%d R%d %d stalled", fetch_input.opcode, fetch_input.src1, fetch_input.src2, fetch_input.literal);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "ADDL")))
    {
        if(!(strcmp(decode_input.opcode, "nop"))){
            if (bflag == 0){
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d %d", fetch_input.opcode, fetch_input.dest, fetch_input.src1, fetch_input.literal);
                pc++;
                decode_input = fetch_input;
                fetch_input = nop;
            }
            else{
                bflag = 0;
                printf("\n Instruction at FETCH_STAGE ---> \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d %d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1, fetch_input.literal);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "SUBL")))
    {
        if(!(strcmp(decode_input.opcode, "nop"))){
            if (bflag == 0){
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d %d", fetch_input.opcode, fetch_input.dest, fetch_input.src1, fetch_input.literal);
                pc++;
                decode_input = fetch_input;
                fetch_input = nop;
            }
            else{
                bflag = 0;
                printf("\n Instruction at FETCH_STAGE --->: \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d %d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1, fetch_input.literal);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "OR")))
    {
        if(!(strcmp(decode_input.opcode, "nop")))
        {
            if (bflag == 0)
            {
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n Instruction at FETCH_STAGE ---> \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "EX-OR")))
    {
        if(!(strcmp(decode_input.opcode, "nop")))
        {
            if (bflag == 0)
            {
                printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n Instruction at FETCH_STAGE ---> \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s R%d R%d R%d stalled", fetch_input.opcode, fetch_input.dest, fetch_input.src1,fetch_input.src2);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "HALT")))
    {
        if(!(strcmp(decode_input.opcode, "nop"))){
            if (bflag == 0){
                printf("\n Instruction at FETCH_STAGE ---> \t\t %s", fetch_input.opcode);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n Instruction at FETCH_STAGE --->: \t\t idle");
            }
        }

        else{
            printf("\n Instruction at FETCH_STAGE --->: \t\t %s stalled", fetch_input.opcode);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "JUMP")))
    {

        if(!(strcmp(decode_input.opcode, "nop"))){
            if (bflag == 0){
                printf("\n Instruction at FETCH_STAGE ---> \t\t %s R%d %d", fetch_input.opcode, fetch_input.src1, fetch_input.literal);
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n Instruction at FETCH_STAGE ---> \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE --->: \t\t %s R%d %d stalled", fetch_input.opcode, fetch_input.src1, fetch_input.literal);
        }
    }
    else if (!(strcmp(fetch_input.opcode, "BZ")))
    {
        if(!(strcmp(decode_input.opcode, "nop"))){
            if (bflag == 0){
                printf("\n Instruction at FETCH_STAGE ---> \t\t %s %d", fetch_input.opcode, fetch_input.literal);
                fetch_input.index = pc;
                decode_input = fetch_input;
                fetch_input = nop;
                pc++;
            }
            else{
                bflag = 0;
                printf("\n Instruction at FETCH_STAGE ---> \t\t idle");
            }
        }
        else{
            printf("\n Instruction at FETCH_STAGE ---> \t %s %d stalled", fetch_input.opcode, fetch_input.literal);
        }
    }

  }
    else
    printf("\n Instruction at FETCH_STAGE ---> \t\t idle");
}

void DECODE_RF_STAGE(){
    int dest = -1;

  if((strcmp(decode_input.opcode, "nop")))
  {
    if (!(strcmp(decode_input.opcode, "MOVC")))
    {
        if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0)
        {
          printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d %d", decode_input.opcode, decode_input.dest, decode_input.literal);

          rob[rob_add_index] = decode_input;
          rob_add_index++;

          dummy1 = decode_input.dest;
          old_instance_prf(decode_input.dest);
          dest = find_new_prf();
          decode_input.dest = dest;
          physical_Reg_File[dest].status = INVALID;
          physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
          physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
          physical_Reg_File[dest].arf_id = dummy1;

          printf("\n Details of RENAME TABLE State --> \t %s P%d %d", decode_input.opcode, decode_input.dest, decode_input.literal);
          //printf("\n stat %d ins_id %d busy %d arf_id %d\n", physical_Reg_File[dest].status, physical_Reg_File[dest].ins_id, physical_Reg_File[dest].busy, physical_Reg_File[dest].arf_id );

          iqueue[iq_add_index] = decode_input;
          decode_input = nop;
          iq_add_index++;                               // can instead use in IQUEUE function
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d %d stalled", decode_input.opcode, decode_input.dest, decode_input.literal);
    }
    else if (!(strcmp(decode_input.opcode, "ADD")))
    {
        if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0)
        {
          printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);

          decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);
          decode_input.src2 = find_existing_prf(decode_input.src2, decode_input.id);

          dummy1 = decode_input.dest;
          old_instance_prf(decode_input.dest);
          dest = find_new_prf();
          decode_input.dest = dest;
          physical_Reg_File[dest].status = INVALID;
          physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
          physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
          physical_Reg_File[dest].arf_id = dummy1;

          printf("\n Details of RENAME TABLE State --> \t %s P%d P%d P%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
          //printf("\n stat %d ins_id %d busy %d arf_id %d\n", physical_Reg_File[dest].status, physical_Reg_File[dest].ins_id, physical_Reg_File[dest].busy, physical_Reg_File[dest].arf_id );

          rob[rob_add_index] = decode_input;
          rob[rob_add_index].dest = dummy1;
          rob_add_index++;

          iqueue[iq_add_index] = decode_input;
          decode_input = nop;
          iq_add_index++;                               // can instead use in IQUEUE function
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d stalled", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
    }
    else if (!(strcmp(decode_input.opcode, "SUB")))
    {
        if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0)
        {
          printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);

          decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);
          decode_input.src2 = find_existing_prf(decode_input.src2, decode_input.id);

          dummy1 = decode_input.dest;
          old_instance_prf(decode_input.dest);
          dest = find_new_prf();
          decode_input.dest = dest;
          physical_Reg_File[dest].status = INVALID;
          physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
          physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
          physical_Reg_File[dest].arf_id = dummy1;

          printf("\n Details of RENAME TABLE State --> \t %s P%d P%d P%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
          //printf("\n stat %d ins_id %d busy %d arf_id %d\n", physical_Reg_File[dest].status, physical_Reg_File[dest].ins_id, physical_Reg_File[dest].busy, physical_Reg_File[dest].arf_id );

          rob[rob_add_index] = decode_input;
          rob[rob_add_index].dest = dummy1;
          rob_add_index++;

          iqueue[iq_add_index] = decode_input;
          decode_input = nop;
          iq_add_index++;                               // can instead use in IQUEUE function
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d stalled", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
    }
    else if (!(strcmp(decode_input.opcode, "AND")))
    {
        if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0)
        {
          printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);

          decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);
          decode_input.src2 = find_existing_prf(decode_input.src2, decode_input.id);

          dummy1 = decode_input.dest;
          old_instance_prf(decode_input.dest);
          dest = find_new_prf();
          decode_input.dest = dest;
          physical_Reg_File[dest].status = INVALID;
          physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
          physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
          physical_Reg_File[dest].arf_id = dummy1;

          printf("\n Details of RENAME TABLE State --> \t %s P%d P%d P%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
          //printf("\n stat %d ins_id %d busy %d arf_id %d\n", physical_Reg_File[dest].status, physical_Reg_File[dest].ins_id, physical_Reg_File[dest].busy, physical_Reg_File[dest].arf_id );

          rob[rob_add_index] = decode_input;
          rob[rob_add_index].dest = dummy1;
          rob_add_index++;

          iqueue[iq_add_index] = decode_input;
          decode_input = nop;
          iq_add_index++;                               // can instead use in IQUEUE function
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d stalled", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
    }
    else if (!(strcmp(decode_input.opcode, "MUL")))
    {

        if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0)
        {

          printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);

          decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);
          decode_input.src2 = find_existing_prf(decode_input.src2, decode_input.id);
          //printf("I am decode MUL %d %d\n", decode_input.src1, decode_input.src2);
          dummy1 = decode_input.dest;
          old_instance_prf(decode_input.dest);
          dest = find_new_prf();
          decode_input.dest = dest;
          physical_Reg_File[dest].status = INVALID;
          physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
          physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
          physical_Reg_File[dest].arf_id = dummy1;

          printf("\n Details of RENAME TABLE State --> \t %s P%d P%d P%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
          //printf("\n stat %d ins_id %d busy %d arf_id %d\n", physical_Reg_File[dest].status, physical_Reg_File[dest].ins_id, physical_Reg_File[dest].busy, physical_Reg_File[dest].arf_id );

          rob[rob_add_index] = decode_input;
          rob[rob_add_index].dest = dummy1;
          rob_add_index++;

          iqueue[iq_add_index] = decode_input;
          decode_input = nop;
          iq_add_index++;                               // can instead use in IQUEUE function
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d stalled", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
    }
    else if (!(strcmp(decode_input.opcode, "LOAD")))
    {
            if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0 && lsq_full_index == 0){
                printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d R%d %d", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);

                decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);

                dummy1 = decode_input.dest;
                old_instance_prf(decode_input.dest);
                dest = find_new_prf();
                decode_input.dest = dest;
                physical_Reg_File[dest].status = INVALID;
                physical_Reg_File[dest].ins_id = decode_input.id;
                physical_Reg_File[dest].busy = 1;
                physical_Reg_File[dest].arf_id = dummy1;

                printf("\n Details of RENAME TABLE State --> \t %s P%d P%d %d", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);

                rob[rob_add_index] = decode_input;
                rob[rob_add_index].dest = dummy1;
                rob_add_index++;

                lsq[lsq_add_index] = decode_input;
                lsq_add_index++;

                iqueue[iq_add_index] = decode_input;
                decode_input = nop;
                iq_add_index++;
            }
            else
                printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d R%d %d stalled", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);
    }
    else if (!(strcmp(decode_input.opcode, "STORE")))
    {
            if(iq_full_index == 0 && rob_full_index == 0 && lsq_full_index == 0){
                printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d R%d %d", decode_input.opcode, decode_input.src1, decode_input.src2, decode_input.literal);

                decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);
                decode_input.src2 = find_existing_prf(decode_input.src2, decode_input.id);

                printf("\n Details of RENAME TABLE State --> \t %s P%d P%d %d", decode_input.opcode, decode_input.src1, decode_input.src2, decode_input.literal);

                rob[rob_add_index] = decode_input;
                rob_add_index++;

                lsq[lsq_add_index] = decode_input;
                lsq_add_index++;

                iqueue[iq_add_index] = decode_input;
                decode_input = nop;
                iq_add_index++;
            }
            else
                printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d R%d %d stalled", decode_input.opcode, decode_input.src1, decode_input.src2, decode_input.literal);
    }
    else if (!(strcmp(decode_input.opcode, "ADDL")))
    {
            if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0 && lsq_full_index == 0){
                printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d R%d %d", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);

                decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);

                dummy1 = decode_input.dest;
                old_instance_prf(decode_input.dest);
                dest = find_new_prf();
                decode_input.dest = dest;
                physical_Reg_File[dest].status = INVALID;
                physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
                physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
                physical_Reg_File[dest].arf_id = dummy1;

                printf("\n Details of RENAME TABLE State --> \t %s P%d P%d %d", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);

                rob[rob_add_index] = decode_input;
                rob[rob_add_index].dest = dummy1;
                rob_add_index++;

                iqueue[iq_add_index] = decode_input;
                decode_input = nop;
                iq_add_index++;                               // can instead use in IQUEUE function
            }
            else
                printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d R%d %d stalled", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);
    }
    else if (!(strcmp(decode_input.opcode, "SUBL")))
    {
            if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0 && lsq_full_index == 0){
                printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d R%d %d", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);

                decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);

                dummy1 = decode_input.dest;
                old_instance_prf(decode_input.dest);
                dest = find_new_prf();
                decode_input.dest = dest;
                physical_Reg_File[dest].status = INVALID;
                physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
                physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
                physical_Reg_File[dest].arf_id = dummy1;

                printf("\n Details of RENAME TABLE State --> \t %s P%d P%d %d", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);

                rob[rob_add_index] = decode_input;
                rob[rob_add_index].dest = dummy1;
                rob_add_index++;

                iqueue[iq_add_index] = decode_input;
                decode_input = nop;
                iq_add_index++;                               // can instead use in IQUEUE function
            }
            else
                printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d R%d %d stalled", decode_input.opcode, decode_input.dest, decode_input.src1, decode_input.literal);
    }
    else if (!(strcmp(decode_input.opcode, "OR")))
    {
        if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0)
        {
          printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);

          decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);
          decode_input.src2 = find_existing_prf(decode_input.src2, decode_input.id);

          dummy1 = decode_input.dest;
          old_instance_prf(decode_input.dest);
          dest = find_new_prf();
          decode_input.dest = dest;
          physical_Reg_File[dest].status = INVALID;
          physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
          physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
          physical_Reg_File[dest].arf_id = dummy1;

          printf("\n Details of RENAME TABLE State --> \t %s P%d P%d P%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
          //printf("\n stat %d ins_id %d busy %d arf_id %d\n", physical_Reg_File[dest].status, physical_Reg_File[dest].ins_id, physical_Reg_File[dest].busy, physical_Reg_File[dest].arf_id );

          rob[rob_add_index] = decode_input;
          rob[rob_add_index].dest = dummy1;
          rob_add_index++;

          iqueue[iq_add_index] = decode_input;
          decode_input = nop;
          iq_add_index++;                               // can instead use in IQUEUE function
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d stalled", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
    }
    else if (!(strcmp(decode_input.opcode, "EX-OR")))
    {
        if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0)
        {
          printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);

          decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);
          decode_input.src2 = find_existing_prf(decode_input.src2, decode_input.id);

          dummy1 = decode_input.dest;
          old_instance_prf(decode_input.dest);
          dest = find_new_prf();
          decode_input.dest = dest;
          physical_Reg_File[dest].status = INVALID;
          physical_Reg_File[dest].ins_id = decode_input.id;         // for later use in rob, iq
          physical_Reg_File[dest].busy = 1;                   // make 0 in rob when the inst id of prf mathces the index of instruction being commited
          physical_Reg_File[dest].arf_id = dummy1;

          printf("\n Details of RENAME TABLE State --> \t %s P%d P%d P%d", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
          //printf("\n stat %d ins_id %d busy %d arf_id %d\n", physical_Reg_File[dest].status, physical_Reg_File[dest].ins_id, physical_Reg_File[dest].busy, physical_Reg_File[dest].arf_id );

          rob[rob_add_index] = decode_input;
          rob[rob_add_index].dest = dummy1;
          rob_add_index++;

          iqueue[iq_add_index] = decode_input;
          decode_input = nop;
          iq_add_index++;                               // can instead use in IQUEUE function
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE --->  \t %s R%d R%d R%d stalled", decode_input.opcode, decode_input.dest, decode_input.src1,decode_input.src2);
    }
    else if (!(strcmp(decode_input.opcode, "HALT")))
    {
        if(prf_available() == 1 && iq_full_index == 0 && rob_full_index == 0){
            printf("\n Instruction at DECODE_RF_STAGE ---> \t %s ", decode_input.opcode);

            rob[rob_add_index] = decode_input;
            rob_add_index++;
            //iqueue[iq_add_index] = decode_input;
            //iq_add_index++;

            fetch_input = nop;
            hflag = 1;   // HALT flag
            decode_input = nop;
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE ---> \t %s stalled", decode_input.opcode);
    }
    else if (!(strcmp(decode_input.opcode, "JUMP")))
    {
        if(iq_full_index == 0 && rob_full_index == 0){
            printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d %d ", decode_input.opcode, decode_input.src1, decode_input.literal);

            decode_input.src1 = find_existing_prf(decode_input.src1, decode_input.id);
            printf("\n Details of RENAME TABLE State ---> \t %s P%d %d ", decode_input.opcode, decode_input.src1, decode_input.literal);

            rob[rob_add_index] = decode_input;
            rob_add_index++;
            iqueue[iq_add_index] = decode_input;
            iq_add_index++;

            fetch_input = nop;
            jflag = 1;          //set bflag = 1 in rob when commiting JUMP ins.
            decode_input = nop;
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE ---> \t %s R%d %d stalled", decode_input.opcode, decode_input.src1, decode_input.literal);
    }
    else if (!(strcmp(decode_input.opcode, "BZ")))
    {
        if(iq_full_index == 0 && rob_full_index == 0){
            printf("\n Instruction at DECODE_RF_STAGE --->: \t %s %d ", decode_input.opcode, decode_input.literal);

            rob[rob_add_index] = decode_input;
            rob_add_index++;
            iqueue[iq_add_index] = decode_input;
            iq_add_index++;

            decode_input = nop;
        }
        else
            printf("\n Instruction at DECODE_RF_STAGE ---> \t %s %d stalled", decode_input.opcode, decode_input.literal);
    }
 }
 else
  printf("\n Instruction at DECODE_RF_STAGE ---> \t idle");
    if(abs(iq_add_index - iq_rem_index) >= 11)
        iq_full_index = 1;
    else
        iq_full_index = 0;
    if(abs(rob_add_index - rob_com_index) >= 11)
        rob_full_index = 1;
    else
        rob_full_index = 0;
    if(abs(lsq_add_index - lsq_rem_index) >= 5)
        lsq_full_index = 1;
    else
        lsq_full_index = 0;

    if(iq_add_index > 11){
        for(int i = 0; i < 12; i++){
            if(!(strcmp(iqueue[i].opcode, "nop")))
               iq_add_index = i;
        }
    }
    if(lsq_add_index > 5){
        for(int i = 0; i < 6; i++){
            if(!(strcmp(lsq[i].opcode, "nop")))
               lsq_add_index = i;
        }
    }
    if(rob_add_index > 11){
        for(int i = 0; i < 12; i++){
            if(!(strcmp(rob[i].opcode, "nop")))
               rob_add_index = i;
        }
    }
}

void INT1_FU_STAGE(){
  //printf("viranchi %s\n", int_fun1_input.opcode);
    if((strcmp(int_fun1_input.opcode, "nop")))
    {
      if (!(strcmp(int_fun1_input.opcode, "MOVC")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d %d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.literal);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "ADD")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d P%d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.src2);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "SUB")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d P%d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.src2);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "AND")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d P%d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.src2);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "MUL")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d P%d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.src2);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "LOAD")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d %d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.literal);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "STORE")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d %d", int_fun1_input.opcode, int_fun1_input.src1,int_fun1_input.src2,int_fun1_input.literal);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "ADDL")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d P%d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.literal);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "SUBL")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d P%d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.literal);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "OR")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d P%d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.src2);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "EX-OR")))
        {
            printf("\n Instruction at INT1_FU_STAGE ---> \t %s P%d P%d P%d", int_fun1_input.opcode, int_fun1_input.dest, int_fun1_input.src1,int_fun1_input.src2);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
      else if (!(strcmp(int_fun1_input.opcode, "JUMP")))
        {
            printf("\n Execute stage : \t %s P%d %d", int_fun1_input.opcode, int_fun1_input.src1, int_fun1_input.literal);
            int_fun2_input = int_fun1_input;
            int_fun1_input = nop;
        }
  }

  else
    printf("\n Instruction at INT1_FU_STAGE ---> \t idle");
}

void INT2_FU_STAG(){
  if((strcmp(int_fun2_input.opcode, "nop")))
  {
    if (!(strcmp(int_fun2_input.opcode, "MOVC")))
    {
        printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d %d", int_fun2_input.opcode, int_fun2_input.dest, int_fun2_input.literal);
        int_fun2_input.result = int_fun2_input.literal;
        physical_Reg_File[int_fun2_input.dest].status = VALID;
        physical_Reg_File[int_fun2_input.dest].value = int_fun2_input.result;

        for (int i = 0; i != 12; i++)
        {
          //printf("\n");
          //printf("kumudini %d\n", i);
          //printf("check id : %d : %d\n", rob[i].id, int_fun2_input.id);
          if(rob[i].id == int_fun2_input.id)
            {
                rob[i].result = int_fun2_input.result;
                rob[i].status = VALID;
            }
        }

    int_fun2_input = nop;
  }
    else if (!(strcmp(int_fun2_input.opcode, "ADD")))
    {
          printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d P%d P%d", int_fun2_input.opcode, int_fun2_input.dest, int_fun2_input.src1,int_fun2_input.src2);
          int_fun2_input.result = physical_Reg_File[int_fun2_input.src1].value + physical_Reg_File[int_fun2_input.src2].value;
          physical_Reg_File[int_fun2_input.dest].status = VALID;
          physical_Reg_File[int_fun2_input.dest].value = int_fun2_input.result;
          //printf("fu2 add result is %ld\n", int_fun2_input.result);
          //Forward the result to rob entry using instruction id
          for (int i = 0; i != 12; i++)
          {
            //printf("\n");
            //printf("kumudini %d\n", i);
            //printf("check id : %d : %d\n", rob[i].id, int_fun2_input.id);
            if(rob[i].id == int_fun2_input.id)
              {
                  rob[i].result = int_fun2_input.result;
                  rob[i].status = VALID;
              }
          }

      int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "SUB")))
    {
          printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d P%d P%d", int_fun2_input.opcode, int_fun2_input.dest, int_fun2_input.src1,int_fun2_input.src2);
          int_fun2_input.result = physical_Reg_File[int_fun2_input.src1].value - physical_Reg_File[int_fun2_input.src2].value;
          physical_Reg_File[int_fun2_input.dest].status = VALID;
          physical_Reg_File[int_fun2_input.dest].value = int_fun2_input.result;
          //Forward the result to rob entry using instruction id
          for (int i = 0; i != 12; i++)
          {
            //printf("\n");
            //printf("kumudini %d\n", i);
            //printf("check id : %d : %d\n", rob[i].id, int_fun2_input.id);
            if(rob[i].id == int_fun2_input.id)
              {
                  rob[i].result = int_fun2_input.result;
                  rob[i].status = VALID;
              }
          }

      int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "AND")))
    {
          printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d P%d P%d", int_fun2_input.opcode, int_fun2_input.dest, int_fun2_input.src1,int_fun2_input.src2);
          int_fun2_input.result = physical_Reg_File[int_fun2_input.src1].value & physical_Reg_File[int_fun2_input.src2].value;
          physical_Reg_File[int_fun2_input.dest].status = VALID;
          physical_Reg_File[int_fun2_input.dest].value = int_fun2_input.result;
          //Forward the result to rob entry using instruction id
          for (int i = 0; i != 12; i++)
          {
            //printf("\n");
            //printf("kumudini %d\n", i);
            //printf("check id : %d : %d\n", rob[i].id, int_fun2_input.id);
            if(rob[i].id == int_fun2_input.id)
              {
                  rob[i].result = int_fun2_input.result;
                  rob[i].status = VALID;
              }
          }

      int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "LOAD")))
    {
        printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d P%d %d", int_fun2_input.opcode, int_fun2_input.dest, int_fun2_input.src1, int_fun2_input.literal);
        int_fun2_input.address = (physical_Reg_File[int_fun2_input.src1].value + int_fun2_input.literal)/4;


        for (int i = 0; i < 6; i++){
            if (lsq[i].id == int_fun2_input.id){
                lsq[i].address = int_fun2_input.address;
                lsq[i].status = VALID;
            }
        }
        int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "STORE")))
    {
        printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d P%d %d", int_fun2_input.opcode, int_fun2_input.src1, int_fun2_input.src2, int_fun2_input.literal);
        int_fun2_input.address = (physical_Reg_File[int_fun2_input.src2].value + int_fun2_input.literal)/4;


        for (int i = 0; i < 6; i++){
            if (lsq[i].id == int_fun2_input.id){
                lsq[i].address = int_fun2_input.address;
                lsq[i].status = VALID;
                //printf("LSQ status : %d\n",lsq[i].status);
            }
        }
        int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "ADDL")))
    {
        printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d %d", int_fun2_input.opcode, int_fun2_input.src1,int_fun2_input.literal);
        int_fun2_input.result = (physical_Reg_File[int_fun2_input.src1].value + int_fun2_input.literal);
        //printf(" in fu2 \n");

        physical_Reg_File[int_fun2_input.dest].status = VALID;
        physical_Reg_File[int_fun2_input.dest].value = int_fun2_input.result;

        for (int i = 0; i != 12; i++)
        {
          //printf("\n");
          //printf("kumudini %d\n", i);
          //printf("check id : %d : %d\n", rob[i].id, int_fun2_input.id);
          if(rob[i].id == int_fun2_input.id)
            {
                rob[i].result = int_fun2_input.result;
                rob[i].status = VALID;
            }
        }
        int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "SUBL")))
    {
        printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d %d", int_fun2_input.opcode, int_fun2_input.src1,int_fun2_input.literal);
        int_fun2_input.result = (physical_Reg_File[int_fun2_input.src1].value - int_fun2_input.literal);

        //printf("result of subl is %ld\n", int_fun2_input.result);

        physical_Reg_File[int_fun2_input.dest].status = VALID;
        physical_Reg_File[int_fun2_input.dest].value = int_fun2_input.result;

        for (int i = 0; i != 12; i++)
        {
          //printf("\n");
          //printf("kumudini %d\n", i);
          //printf("check id : %d : %d\n", rob[i].id, int_fun2_input.id);
          if(rob[i].id == int_fun2_input.id)
            {
                rob[i].result = int_fun2_input.result;
                rob[i].status = VALID;
            }
        }
        int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "OR")))
    {
          printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d P%d P%d", int_fun2_input.opcode, int_fun2_input.dest, int_fun2_input.src1,int_fun2_input.src2);
          int_fun2_input.result = physical_Reg_File[int_fun2_input.src1].value || physical_Reg_File[int_fun2_input.src2].value;
          physical_Reg_File[int_fun2_input.dest].status = VALID;
          physical_Reg_File[int_fun2_input.dest].value = int_fun2_input.result;
          for (int i = 0; i != 12; i++)
          {
            //printf("\n");
            //printf("kumudini %d\n", i);
            //printf("check id : %d : %d\n", rob[i].id, int_fun2_input.id);
            if(rob[i].id == int_fun2_input.id)
              {
                  rob[i].result = int_fun2_input.result;
                  rob[i].status = VALID;
              }
          }

      int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "EX-OR")))
    {
          printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d P%d P%d", int_fun2_input.opcode, int_fun2_input.dest, int_fun2_input.src1,int_fun2_input.src2);
          int_fun2_input.result = physical_Reg_File[int_fun2_input.src1].value ^ physical_Reg_File[int_fun2_input.src2].value;
          physical_Reg_File[int_fun2_input.dest].status = VALID;
          physical_Reg_File[int_fun2_input.dest].value = int_fun2_input.result;

          for (int i = 0; i != 12; i++)
          {
            //printf("\n");
            //printf("kumudini %d\n", i);
            //printf("check id : %d : %d\n", rob[i].id, int_fun2_input.id);
            if(rob[i].id == int_fun2_input.id)
              {
                  rob[i].result = int_fun2_input.result;
                  rob[i].status = VALID;
              }
          }

      int_fun2_input = nop;
    }
    else if (!(strcmp(int_fun2_input.opcode, "JUMP")))
    {
        printf("\n Instruction at INT2_FU_STAGE ---> \t %s P%d %d", int_fun2_input.opcode, int_fun2_input.src1, int_fun2_input.literal);
        int_fun2_input.result = (physical_Reg_File[int_fun2_input.src1].value + int_fun2_input.literal - 4000)/4;

        for (int i = 0; i != 12; i++){
            if(rob[i].id == int_fun2_input.id){
                rob[i].result = int_fun2_input.result;
                rob[i].status = VALID;
            }
        }
        bflag = 1;
        int_fun2_input = nop;
    }

  }
  else
      printf("\n Instruction at INT2_FU_STAGE ---> \t idle");
}

void bz_fu(){

if((strcmp(branch_fun_input.opcode, "nop"))){

    if (!(strcmp(branch_fun_input.opcode, "BZ"))){
      printf("\n Branch_FU stage ---> \t\t\t %s %d", branch_fun_input.opcode, branch_fun_input.literal);
      branch_fun_input.result = (branch_fun_input.index + (branch_fun_input.literal/4));
      //printf("branch result %ld \n",branch_fun_input.result);
      //Forward the result to rob entry using instruction id
      for (int i = 0; i != 12; i++){
          if(rob[i].id == branch_fun_input.id){
              rob[i].result = branch_fun_input.result;
              rob[i].branch = branch_fun_input.branch;
              rob[i].status = VALID;
          }
      }
      branch_fun_input = nop;
  }
  else
      printf("\n Branch_FU stage ---> \t idle");
  }
}

void memory(){

    if((strcmp(memory_input.opcode, "nop")))
    {
      if(!(strcmp(memory_input.opcode, "LOAD")))
        {
          printf("\nInstruction at MEM_FU_STAGE ---> \t %s P%d P%d %d", memory_input.opcode, memory_input.dest, memory_input.src1, memory_input.literal);
          memory_input.result = data_Memory[memory_input.address];
          physical_Reg_File[memory_input.dest].value = memory_input.result;
          physical_Reg_File[memory_input.dest].status = VALID;

          for (int i = 0; i < 12; i++){
              if(rob[i].id == memory_input.id){
                  rob[i].result = memory_input.result;
                  rob[i].status = VALID;
              }
          }
          memory_input = nop;
      }
        else if(!(strcmp(memory_input.opcode, "STORE")))
        {
            printf("\n Instruction at MEM_FU_STAGE --->  \t %s P%d P%d %d", memory_input.opcode, memory_input.src1, memory_input.src2, memory_input.literal);
            data_Memory[memory_input.address] = physical_Reg_File[memory_input.src1].value;

            for (int i = 0; i < 12; i++)
            {
                if(rob[i].id == memory_input.id)
                {
                    rob[i].status = VALID;
                }
            }
            memory_input = nop;
        }
    }
    printf("\n Instruction at MEM_FU_STAGE ---> \t idle");
}

void mul1(){

  if((strcmp(mul_fun1_input.opcode, "nop"))){
    if (!(strcmp(mul_fun2_input.opcode, "nop"))){
        printf("\n Instruction at MUL1_FU_STAGE ---> \t\t %s P%d P%d P%d", mul_fun1_input.opcode, mul_fun1_input.dest, mul_fun1_input.src1, mul_fun1_input.src2);
        mul_fun2_input = mul_fun1_input;
        mul_fun1_input = nop;
      }
    else
      printf("\n Instruction at MUL1_FU_STAGE ---> \t\t %s P%d P%d P%d stalled", mul_fun1_input.opcode, mul_fun1_input.dest, mul_fun1_input.src1, mul_fun1_input.src2);
  }
}

void mul2(){
  if((strcmp(mul_fun2_input.opcode, "nop")))
  {
      if (!(strcmp(mul_fun3_input.opcode, "nop")))
      {
        printf("\n Instruction at MUL2_FU_STAGE ---> \t %s P%d P%d P%d", mul_fun2_input.opcode, mul_fun2_input.dest, mul_fun2_input.src1, mul_fun2_input.src2);
        mul_fun3_input = mul_fun2_input;
        mul_fun2_input = nop;
      }
      else
        printf("\n Instruction at MUL2_FU_STAGE ---> \t %s P%d P%d P%d stalled", mul_fun2_input.opcode, mul_fun2_input.dest, mul_fun2_input.src1, mul_fun2_input.src2);
  }
}

void mul3(){
  if((strcmp(mul_fun3_input.opcode, "nop")))
  {
      //printf("I am in mul 3 \n");
      printf("\n Instruction at MUL3_FU_STAGE ---> \t %s P%d P%d P%d", mul_fun3_input.opcode, mul_fun3_input.dest, mul_fun3_input.src1, mul_fun3_input.src2);
      mul_fun3_input.result = physical_Reg_File[mul_fun3_input.src1].value * physical_Reg_File[mul_fun3_input.src2].value;

      physical_Reg_File[mul_fun3_input.dest].status = VALID;
      physical_Reg_File[mul_fun3_input.dest].value = mul_fun3_input.result;
      lst_arithm_instruction = mul_fun3_input.id;
      lst_arithm_resultset = mul_fun3_input.result;
      //printf("architecture register  update %ld \n", lst_arithm_resultset);
      //Forward the result to rob entry using instruction id
      for (int i = 0; i != 12; i++)
      {
          if(rob[i].id == mul_fun3_input.id)
          {
              rob[i].result = mul_fun3_input.result;
              rob[i].status = VALID;
              //printf(" status of ROB %d\n", rob[i].status);
          }
      }
      mul_fun3_input = nop;
      if (!(strcmp(mul_fun2_input.opcode, "nop")))
          mflag = 0;
  }
}

void intialize(){
  for (int i = 0; i < 16; i++)
  {
      arch_Reg_File[i].status = VALID;
      arch_Reg_File[i].value = 0;
      arch_Reg_File[i].ins_id = 0;
  }
  for (int i = 0; i < 24; i++)
  {
      physical_Reg_File[i].value = 0;
      physical_Reg_File[i].ins_id = -1;
      physical_Reg_File[i].status = 0;
      physical_Reg_File[i].busy = 0;
      physical_Reg_File[i].old_instance = 0;
  }
  for (int i = 0; i < 12; i++)
  {
      iqueue[i] = nop;
  }
  for (int i = 0; i < 12; i++)
  {
      rob[i] = nop;
  }
  for (int i = 0; i < 6; i++)
  {
      lsq[i] = nop;
  }
}

void display(){
  printf("\n---------Architecture Register File-----------\n");
  for(int i =0; i<=15; i++)
  {
      printf("R%d = %ld \n", i, arch_Reg_File[i].value);
  }
  printf("\n---------Physical Register File-----------\n");
  for(int i =0; i<=23; i++)
  {
      printf("P%d = %ld \n", i, physical_Reg_File[i].value);
  }
  printf("\n---------Data Memory-------------\n");
  for(int i =0; i < 25; i++)
  {
      printf("data_mem[%d] = %ld \n", i*4, data_Memory[i]);
  }

}

int prf_available(){
  for (int i = 0; i < 24;)
  {
      if (physical_Reg_File[i].busy == 0)
          return 1;
      else
          i++;
  }
  return 0;
}

int find_new_prf(){
  for (int i = 0; i < 24;)
  {
      if (physical_Reg_File[i].busy == 0)
      {
          return i;
      }
      else {
          i++;
      }
  }
  return -1;
}

int find_existing_prf(int p, int q){
    for (int i = 0; i < 24; )
    {
      if ((physical_Reg_File[i].arf_id == p) && (physical_Reg_File[i].busy == 1) && (physical_Reg_File[i].old_instance == 0)){
          physical_Reg_File[i].ins_id = q;
          return i;
      }
      else
          i++;
    }
    return -1;
}

void old_instance_prf(int p){
  for (int i = 0; i < 24; )
  {
      if ((physical_Reg_File[i].arf_id == p) && (physical_Reg_File[i].busy == 1))
      {
          physical_Reg_File[i].old_instance = 1;
          break;
      }
      else
          i++;
  }
}


void iq(){

    if((strcmp(iqueue[iq_rem_index].opcode, "nop"))){
        if(!(strcmp(int_fun1_input.opcode, "nop"))){
            if(!(strcmp(iqueue[iq_rem_index].opcode, "MOVC"))){
                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d %d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].literal);
                int_fun1_input = iqueue[iq_rem_index];
                //printf("\nshantanu %s", int_fun1_input.opcode);
                iqueue[iq_rem_index] = nop;
                iq_rem_index++;
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "ADD"))){
                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d P%d P%d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].src2);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID && physical_Reg_File[iqueue[iq_rem_index].src2].status == VALID){
                    lst_arithm_index = iqueue[iq_rem_index].id;
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "SUB"))){

                printf("\n Details of IQ (Issue Queue) State –> \t %s P%d P%d P%d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].src2);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID && physical_Reg_File[iqueue[iq_rem_index].src2].status == VALID){
                    lst_arithm_index = iqueue[iq_rem_index].id;
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "AND"))){

                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d P%d P%d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].src2);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID && physical_Reg_File[iqueue[iq_rem_index].src2].status == VALID){
                    lst_arithm_index = iqueue[iq_rem_index].id;
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "MUL"))){

                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d P%d P%d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].src2);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID && physical_Reg_File[iqueue[iq_rem_index].src2].status == VALID){
                    lst_arithm_index = iqueue[iq_rem_index].id;
                    mul_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                    mflag = 1;
                  }
              }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "LOAD"))){
                //printf("I am in IQ for Load \n");
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID){
                    printf("\n IQ      : \t\t %s P%d P%d %d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].literal);
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "STORE"))){
                //printf("I am in IQ of Store %d \n",physical_Reg_File[iqueue[iq_rem_index].src2].status);
                if (physical_Reg_File[iqueue[iq_rem_index].src2].status == VALID){
                    printf("\n Details of IQ (Issue Queue) State –> \t\t %s P%d P%d %d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].src2, iqueue[iq_rem_index].literal);
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "ADDL"))){
                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d P%d %d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].literal);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID ){
                    lst_arithm_index = iqueue[iq_rem_index].id; //compare lst_arithm_index with last_aritmetic_ins(updated in ex), for BRANCH.
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "SUBL"))){
                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d P%d %d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].literal);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID ){
                    lst_arithm_index = iqueue[iq_rem_index].id;
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "OR"))){

                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d P%d P%d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].src2);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID && physical_Reg_File[iqueue[iq_rem_index].src2].status == VALID){
                    lst_arithm_index = iqueue[iq_rem_index].id;
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "EX-OR"))){

                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d P%d P%d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].dest, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].src2);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID && physical_Reg_File[iqueue[iq_rem_index].src2].status == VALID){
                    lst_arithm_index = iqueue[iq_rem_index].id;
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "JUMP"))){

                printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d %d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].src1, iqueue[iq_rem_index].literal);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID){
                    int_fun1_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                    bflag = 1;
                }
            }
            else if(!(strcmp(iqueue[iq_rem_index].opcode, "BZ"))){
                //printf("I am in IQ for BZ \n");
                printf("\n Details of IQ (Issue Queue) State –>  \t %s %d", iqueue[iq_rem_index].opcode, iqueue[iq_rem_index].literal);
                if (physical_Reg_File[iqueue[iq_rem_index].src1].status == VALID && physical_Reg_File[iqueue[iq_rem_index].src2].status == VALID){
                    lst_arithm_index = iqueue[iq_rem_index].id;
                    branch_fun_input = iqueue[iq_rem_index];
                    iqueue[iq_rem_index] = nop;
                    iq_rem_index++;
                }
            }

          }
      }

    if(iq_rem_index > 11){
      //printf("I am in IQ when it is full\n");
        for(int i = 0; i < 12; i++){
          //printf("\n i m inside loop %s", rob[i].opcode);
            if((strcmp(rob[i].opcode, "nop"))){
              iq_rem_index = i;
              break;
            }

        }
    }

    if (iq_add_index == iq_rem_index){
      //printf("limit of issue \n");
        if((strcmp(iqueue[iq_rem_index].opcode, "nop"))){
          iq_full_index = 1;
        }
    }
}


void LSQ(){

if((strcmp(lsq[lsq_rem_index].opcode, "nop")))
 {
    if(!(strcmp(memory_input.opcode, "nop")))
    {
      if(!(strcmp(lsq[lsq_rem_index].opcode, "LOAD")))
      {
          if(lsq[lsq_rem_index].status == VALID)
          {
              printf("\n Details of LSQ (Load-Store Queue) State --> \t %s P%d P%d %d", lsq[lsq_rem_index].opcode, lsq[lsq_rem_index].dest, lsq[lsq_rem_index].src1, lsq[lsq_rem_index].literal);
              memory_input = lsq[lsq_rem_index];
              lsq[lsq_rem_index] = nop;
              lsq_rem_index++;
          }
          else
              printf("\n Details of LSQ (Load-Store Queue) State --> \t %s P%d P%d %d stalled", lsq[lsq_rem_index].opcode, lsq[lsq_rem_index].dest, lsq[lsq_rem_index].src1, lsq[lsq_rem_index].literal);
      }
      else if(!(strcmp(lsq[lsq_rem_index].opcode, "STORE")))
      {
            //printf("Status bits : %d %d \n", lsq[lsq_rem_index].status, physical_Reg_File[lsq[lsq_rem_index].src1].status);
            if(physical_Reg_File[lsq[lsq_rem_index].src1].status == VALID && lsq[lsq_rem_index].status == VALID)
            {
                printf("\n Details of LSQ (Load-Store Queue) State --> \t %s P%d P%d %d", lsq[lsq_rem_index].opcode, lsq[lsq_rem_index].src1, lsq[lsq_rem_index].src2, lsq[lsq_rem_index].literal);
                memory_input = lsq[lsq_rem_index];
                lsq[lsq_rem_index] = nop;
                lsq_rem_index++;
            }
            else
                printf("\n Details of LSQ (Load-Store Queue) State --> \t %s P%d P%d %d stalled", lsq[lsq_rem_index].opcode, lsq[lsq_rem_index].src1, lsq[lsq_rem_index].src2, lsq[lsq_rem_index].literal);
        }
    }
 }
  if (lsq_rem_index > 5)
  {
      lsq_rem_index = 0;
  }
  if (lsq_add_index == lsq_rem_index)
  {
      if((strcmp(lsq[lsq_add_index].opcode, "nop")))
          lsq_full_index = 1;
  }
}


void ROB(){
  int i = rob_com_index;
  //printf("\n");
  //printf("kumudini ROB %d : %s: %d\n", i, rob[i].opcode, rob[i].status);
  if((strcmp(rob[i].opcode, "nop")))
  {
    //printf("index %d \n", i);
      if(!(strcmp(rob[i].opcode, "MOVC"))){
          printf("\n Details of ROB  State --> \t\t  %s R%d %d", rob[i].opcode, rob[i].dest, rob[i].literal);
          if (rob[i].status == VALID)
          {
              //printf("I m in ROB move for %d\n", i);
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "ADD"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d P%d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].src2);
          if (rob[i].status == VALID){
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              if (physical_Reg_File[rob[i].src2].ins_id == rob[i].id && physical_Reg_File[rob[i].src2].old_instance == 1){
                  physical_Reg_File[rob[i].src2].busy = 0;
                  physical_Reg_File[rob[i].src2].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "SUB"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d P%d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].src2);
          if (rob[i].status == VALID){
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              if (physical_Reg_File[rob[i].src2].ins_id == rob[i].id && physical_Reg_File[rob[i].src2].old_instance == 1){
                  physical_Reg_File[rob[i].src2].busy = 0;
                  physical_Reg_File[rob[i].src2].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "AND"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d P%d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].src2);
          if (rob[i].status == VALID){
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              if (physical_Reg_File[rob[i].src2].ins_id == rob[i].id && physical_Reg_File[rob[i].src2].old_instance == 1){
                  physical_Reg_File[rob[i].src2].busy = 0;
                  physical_Reg_File[rob[i].src2].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "MUL"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d P%d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].src2);
          if (rob[i].status == VALID){
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              //printf("IN ROB FOR archi regist %ld \n", arch_Reg_File[rob[i].dest].value);
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              if (physical_Reg_File[rob[i].src2].ins_id == rob[i].id && physical_Reg_File[rob[i].src2].old_instance == 1){
                  physical_Reg_File[rob[i].src2].busy = 0;
                  physical_Reg_File[rob[i].src2].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "LOAD"))){
          if (rob[i].status == VALID){
              printf("\n Details of ROB  State --> \t\t %s R%d P%d %d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].literal);
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "STORE"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d %d", rob[i].opcode, rob[i].src1, rob[i].src2, rob[i].literal);
          if (rob[i].status == VALID){
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              if (physical_Reg_File[rob[i].src2].ins_id == rob[i].id && physical_Reg_File[rob[i].src2].old_instance == 1){
                  physical_Reg_File[rob[i].src2].busy = 0;
                  physical_Reg_File[rob[i].src2].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "ADDL"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d %d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].literal);
          if (rob[i].status == VALID){
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "SUBL"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d %d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].literal);
          if (rob[i].status == VALID){
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "OR"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d P%d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].src2);
          if (rob[i].status == VALID){
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              if (physical_Reg_File[rob[i].src2].ins_id == rob[i].id && physical_Reg_File[rob[i].src2].old_instance == 1){
                  physical_Reg_File[rob[i].src2].busy = 0;
                  physical_Reg_File[rob[i].src2].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "EX-OR"))){
          printf("\n Details of ROB  State --> \t\t %s R%d P%d P%d", rob[i].opcode, rob[i].dest, rob[i].src1, rob[i].src2);
          if (rob[i].status == VALID){
              arch_Reg_File[rob[i].dest].value = rob[i].result;
              if (physical_Reg_File[rob[i].src1].ins_id == rob[i].id && physical_Reg_File[rob[i].src1].old_instance == 1){
                  physical_Reg_File[rob[i].src1].busy = 0;
                  physical_Reg_File[rob[i].src1].old_instance = 0;
              }
              if (physical_Reg_File[rob[i].src2].ins_id == rob[i].id && physical_Reg_File[rob[i].src2].old_instance == 1){
                  physical_Reg_File[rob[i].src2].busy = 0;
                  physical_Reg_File[rob[i].src2].old_instance = 0;
              }
              rob_com_index++;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "HALT"))){
          printf("\n Details of ROB  State --> \t\t %s ", rob[i].opcode);
          if (rob[i].status == VALID){
              rob_com_index++;
              rob[i] = nop;
              hflag = 100;
          }
      }
      else if(!(strcmp(rob[i].opcode, "JUMP"))){
          printf("\n Details of ROB  State --> \t\t %s P%d %d ", rob[i].opcode, rob[i].src1, rob[i].literal);
          if (rob[i].status == VALID){
              rob_com_index++;
              bflag = 1;
              pc = rob[i].result;
              jflag = 0;
              rob[i] = nop;
          }
      }
      else if(!(strcmp(rob[i].opcode, "BZ"))){
        //printf("I am ROB for BZ \n");
          bzflag = 0;
          if (rob[i].status == VALID){
              if(rob[i].branch == 1){
                  printf("\n Details of ROB  State --> \t\t %s %d ", rob[i].opcode, rob[i].literal);
                  bflag = 1;
                  //hflag = 0;
                  pc = rob[i].result;
                  for (int j = 0; j < 12; j++){
                      if (rob[j].id > rob[i].id)
                          rob[j] = nop;
                  }
                  rob_com_index++;
                  rob_add_index = rob_com_index;
                  rob[i] = nop;
              }
              else {
                  printf("\n Details of ROB  State --> \t\t %s %d ", rob[i].opcode, rob[i].literal);
                  rob[i] = nop;
                  rob_com_index++;
              }
          }
      }
  }
  //printf("\n testing %d", rob_com_index );
  if(rob_com_index > 12)
  {
      for(int i = 0; i < 12; i++)
      {
          if((strcmp(rob[i].opcode, "nop")))
            rob_com_index = i;
      }
  }
  if (rob_add_index == rob_com_index)
  {
      if((strcmp(rob[rob_add_index].opcode, "nop")))
          rob_full_index = 1;
      else
          rob_full_index = 0;
  }

}
