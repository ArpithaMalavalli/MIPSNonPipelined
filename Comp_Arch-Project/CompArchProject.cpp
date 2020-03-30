#include <iostream>
#include<fstream>
#include <string>
#include<stdio.h>
#include<cstring>
#include<ctype.h>

using namespace std;

typedef struct instruction
{
  string op;
  string rs,rt,rd;
  string function;
  string add_imm;

}inst;

inst ins_mem[7];

int v0=5,v1=4,PC=1;
int zero=0,one=1;
bool pcSrc=false;
int n;

  string convertToString(char* a, int size)
  {
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
  }

string findTwoscomplement(string str)
{
    int n = str.length();
    // Traverse the string to get first '1' from
    // the last of string
    int i;
    for (i = n-1 ; i >= 0 ; i--)
        if (str[i] == '1')
            break;

    // If there exists no '1' concatenate 1 at the
    // starting of string
    if (i == -1)
        return '1' + str;

    // Continue traversal after the position of
    // first '1'
    for (int k = i-1 ; k >= 0; k--)
    {
        //Just flip the values
        if (str[k] == '1')
            str[k] = '0';
        else
            str[k] = '1';
    }
    // return the modified string
    return str;
}

int* get_reg(string bin)
{
  if(bin=="00010")
  return &v0;
  else if(bin=="00011")
  return &v1;
  else if(bin=="00000")
  return &zero;
  else if(bin=="00001")
  return &one;
}
int get_value(string bin)
{
  if(bin=="00010")
  return v0;
  else if(bin=="00011")
  return v1;
  else if(bin=="00000")
  return 0;
  else if(bin=="00001")
  return 1;
}

struct data
{
  int value;
  char* address;
};


int clk=1;
class operation
{
public:
  int ex_result=0,mem_result=0;
  int stage=0;
  inst ci;
  int clk_if=-1,clk_id=-1,clk_ex=-1,clk_mem=-1,clk_wb=-1;
  string instruction;
public:
  operation(){}

  void IF_stage()
  {
    stage=1;
    clk_if=clk;
    ci=ins_mem[PC];//ci=>current instruction
    PC=PC+1;
  }

  int ID_stage()
  {
    clk_id=clk_if+1;
    stage=2;
    if(ci.op=="000000")
    return 1;//add and mult

    else if(ci.op=="100011")
    {
      return 2;//load
    }
    else if(ci.op=="101011")
    {
      return 3;//store
    }
    else if(ci.op=="001000")
    {
      return 4;//addi
    }
    else if(ci.op=="000101")
    {
      return 5;//bne
    }
  }
    virtual void EX_stage(){}
    virtual void MEM_stage(){}
    virtual void WB_stage(){}
};

class Rformat:public operation
{
public:
  Rformat(operation& op)
  {
    this->ex_result=op.ex_result;
    this->mem_result=op.mem_result;
    this->stage=op.stage;
    this->ci=op.ci;
    this->clk_if=op.clk_if;
    this->clk_id=op.clk_id;
  }
  void EX_stage()
  {
    clk_ex=clk_id+1;
    stage=3;
  if(ci.function=="100000")
  {
    ex_result=*(get_reg(ci.rs))+*(get_reg(ci.rt));
  }
  else if(ci.function=="011000")
  ex_result=(*get_reg(ci.rs))*(*get_reg(ci.rt));
  }
  void MEM_stage()
  {
      clk_mem=clk_ex+1;
      stage=4;
  }
  void WB_stage()
  {
    clk_wb=clk_mem+1;
    stage=5;
  *(get_reg(ci.rd))=ex_result;
    }
};

class load:public operation
{
public:
  load(operation& op)
  {
    this->ex_result=op.ex_result;
    this->mem_result=op.mem_result;
    this->stage=op.stage;
    this->ci=op.ci;
    this->clk_if=op.clk_if;
    this->clk_id=op.clk_id;
  }
  void EX_stage(){
     stage=3;
     clk_ex=clk_id+1;
  //  ex_result=stoi(ci.rs,0,2);
  }
  void MEM_stage(){
    clk_mem=clk_ex+1;
    stage=4;
    ifstream f;
    f.open("dataMem.dat");
    struct data temp;
    while(f)
    {
      // cout<<" fdas"<<sizeof(temp)<<endl;
      f.read((char*)&temp,sizeof(temp));
      // cout<<temp.address<<endl<<temp.value<<endl;
      if(temp.address==ci.add_imm)
      {
      mem_result=temp.value;
      // cout<<"heel"<<endl;
    }
    }
    f.close();
  }
  void WB_stage(){
    clk_wb=clk_mem+1;
    stage=5;
    *(get_reg(ci.rd))=mem_result;
    }
};

class store:public operation
{
public:
  store(operation& op)
  {
    this->ex_result=op.ex_result;
    this->mem_result=op.mem_result;
    this->stage=op.stage;
    this->ci=op.ci;
    this->clk_if=op.clk_if;
    this->clk_id=op.clk_id;
  }
  void EX_stage(){
    stage=3;
    ex_result=*(get_reg(ci.rs));//do get reg for rs

  }
  void MEM_stage(){
    stage=4;
    ifstream f1;
    ofstream f2;

    f1.open("dataMem.dat");
    f2.open("temp.dat");
    struct data temp1;
    char* a=strdup(ci.add_imm.c_str());
    temp1.address=a;
    temp1.value=ex_result;
    struct data temp2;
    while(f1)
    {
      f1.read((char*)&temp2,sizeof(temp2));
      if(strcmp(temp2.address,a)!=0)
        f2.write((char*)&temp2,sizeof(temp2));
      else
        f2.write((char*)&temp1,sizeof(temp1));

    }
  remove("dataMem.dat");
  rename("temp.dat","dataMem.dat");
  f1.close();
  f2.close();
  }
  void WB_stage(){
  stage=5;
  }
};

class addi:public operation
{
public:
  addi(operation& op)
  {
    this->ex_result=op.ex_result;
    this->mem_result=op.mem_result;
    this->stage=op.stage;
    this->ci=op.ci;
    this->clk_if=op.clk_if;
    this->clk_id=op.clk_id;
  }
  void EX_stage(){
    stage=3;
    ex_result=*(get_reg(ci.rs))-stoi(ci.add_imm,0,2);
  }
  void MEM_stage(){
    stage=4;
  }
  void WB_stage(){
    stage=5;
  *(get_reg(ci.rd))=ex_result;
  }
};


class bne:public operation
{

public:
  bne(operation& op)
  {
    this->ex_result=op.ex_result;
    this->mem_result=op.mem_result;
    this->stage=op.stage;
    this->ci=op.ci;
    this->clk_if=op.clk_if;
    this->clk_id=op.clk_id;
  }
  void EX_stage(){
    stage=3;
  if((get_value(ci.rs))!=(get_value(ci.rt)))
  {
    pcSrc=true;
    if(ci.add_imm[0]==0)
    {
    ex_result=stoi(ci.add_imm,0,2);
  }
    else
    ex_result=-1*(stoi(findTwoscomplement(ci.add_imm),0,2));
  }
  else{
    pcSrc=false;
    ex_result=0;
  }
  if(pcSrc==true)
  {
    PC+=ex_result;
  }
  }
  void MEM_stage(){
    stage=4;
  }
  void WB_stage(){
    stage=5;
  }
};

void PrintData()
{
  ifstream f;
  f.open("dataMem.dat");
  struct data temp;
  int i=0;
  while(i<3)
  {
    // cout<<" fdas"<<sizeof(temp)<<endl;
    f.read((char*)&temp,sizeof(temp));
    cout<<temp.address<<"\t"<<temp.value<<endl;
    i++;
  }
  f.close();
}
void Initialize_Mem()
{

  ins_mem[1].op="100011";
  ins_mem[1].rd="00010";
  ins_mem[1].add_imm="0000000000000"; //Load v0 (i=n)

  ins_mem[2].op="000000";
  ins_mem[2].rs="00000";
  ins_mem[2].rt="00010";
  ins_mem[2].rd="00011";
  ins_mem[2].function="100000"; //Add v0 and 0 and store in v1 (fact=n)

  ins_mem[3].op="001000";
  ins_mem[3].rd="00010";
  ins_mem[3].rs="00010";
  ins_mem[3].add_imm="000000000000001";//v0=v0-1 (i--)

  ins_mem[4].op="000000";
  ins_mem[4].rs="00011";
  ins_mem[4].rt="00010";
  ins_mem[4].rd="00011";
  ins_mem[4].function="011000"; //mult v0 and v1 and store in v1 (fact=fact*i)

  ins_mem[5].op="000101";
  ins_mem[5].rs="00010";
  ins_mem[5].rt="00001";
  ins_mem[5].add_imm="1111111111111101";//bne v1 and $one(i!=1)

  ins_mem[6].op="101011";
  ins_mem[6].rs="00011";
  ins_mem[6].add_imm="0000000000100";//store v1(fact)


  ofstream f;
  struct data d1={5,"0000000000000"};
  struct data d3={0,"0000000000100"};
  struct data d2={4,"0000000001000"};
  d1.value=n;
  f.open("dataMem.dat");
  f.write((char*)&d1,sizeof(d1));
  f.write((char*)&d3,sizeof(d3));
  f.write((char*)&d2,sizeof(d2));
  f.close();
}

int main()
{

  cout<<"enter the number to find factorial"<<endl;
  //should be less than 12;
  cin>>n;
  Initialize_Mem();

  cout<<endl;

  for(int i=1;i<4+((n-1)*3);i++)
  {
  operation o=operation();
  o.IF_stage();
  cout<<o.stage<<" ";
  int c=o.ID_stage();
  operation* op1;
  if(c==1)
    op1=new Rformat(o);
  else if(c==2)
    op1=new load(o);
  else if(c==3)
    op1=new store(o);
  else if(c==4)
    op1=new addi(o);
  else if(c==5)
    op1=new bne(o);
  cout<<op1->stage<<" ";
  op1->EX_stage();
    cout<<op1->stage<<" ";
  op1->MEM_stage();
    cout<<op1->stage<<" ";
  op1->WB_stage();
    cout<<op1->stage<<endl;

    if(i!=3+((n-1)*3))
    {
    for(int j=0;j<10*i;j++)
      cout<<" ";
    }
    clk+=1;
}
cout<<"The factorial is stored in the second memory location"<<endl;
cout<<"Data Memory:"<<endl;
PrintData();

}
