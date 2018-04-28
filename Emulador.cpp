#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
using namespace std;

typedef unsigned int palavra;
typedef unsigned char byte;
typedef unsigned long int microinstrucao;

//Registradores
palavra MAR = 0, MDR = 0, PC = 0; 	//ACESSO MEMÓRIA
byte MBR = 0; 						//ACESSO MEMÓRIA
palavra SP = 0, LV = 0, CPP = 0, TOS = 0, OPC = 0, H = 0;//OPERAÇÃO NA ULA

//Barramentos
palavra bA, bB, bC;

//Informações para o desclocador
byte zero = 0, nzero = 0; 

//Separações da microinstrução
byte mi_barramentoB, mi_operacao, mi_pulo, mi_memoria, mi_deslocador = 0; 
palavra mi_gravar, MPC = 0; 

//firmware que controla a ULA
microinstrucao armazenamento[512];
microinstrucao mi;

//Memoria principal do emulador
byte memoria[100000000];

void decodificar_microinstrucao();
void ULA();
void atribuir_barramentoB();
void atribuir_registradores();
void pular();
void operar_memoria();

void exibe_processo();

void carrega_microprograma();

int main(){
	carrega_microprograma();
	//Programa teste
	memoria[1025] = 0x19;	memoria[1026] = 10;
	memoria[1027] = 0x22;	memoria[1028] = 1;
	memoria[1029] = 0x19;	memoria[1030] = 10;
	memoria[1031] = 0x22;	memoria[1032] = 2;
	memoria[1033] = 0x1C;	memoria[1034] = 1;
	memoria[1035] = 0x1C;	memoria[1036] = 2;
	memoria[1037] = 0x02;
	memoria[1038] = 0x22;	memoria[1039] = 3;
	memoria[1040] = 0x1C;	memoria[1041] = 3;
	memoria[1042] = 0x19;	memoria[1043] = 25;
	memoria[1044] = 0x4B;	memoria[1045] = 30;
	memoria[1046] = 0x1C;	memoria[1047] = 1;
	memoria[1048] = 0x19;	memoria[1049] = 1;
	memoria[1050] = 0x05;
	memoria[1051] = 0x22;	memoria[1052] = 1;
	memoria[1053] = 0x3C;	memoria[1054] = 34;
	memoria[1055] = 0x19;	memoria[1056] = 13;
	memoria[1057] = 0x22;	memoria[1058] = 2;
	memoria[1059] = 0x13;	memoria[1060] = 25;
	
	while(true){
		mi = armazenamento[MPC];

		exibe_processo();

		//Conjunto de operações realizadas
		decodificar_microinstrucao();
		atribuir_barramentoB();
		ULA();
		atribuir_registradores();
		operar_memoria();
		pular();

		
	}
	return 0;
}

void carrega_microprograma(){
	FILE *microprograma;
	microprograma = fopen("microprog.rom", "rb");

	if (microprograma != NULL) {
		fread(armazenamento, sizeof(microinstrucao), 512, microprograma);
		fclose(microprograma);

	}
	
	//Inicializa a memória e os registradores
	palavra tmp = 0x1001; //LV
	
	memoria[1] = 0x73; //init
	memoria[4] = 0x0006; //(CPP inicia com o valor 0x0006 guardado na palavra 1 – bytes 4 a 7.)
	memcpy(memoria+8, &tmp, 4); //(LV inicia com o valor de tmp guardado na palavra 2 – bytes 8 a 11)
	tmp = 0x0400; //PC
	memcpy(memoria+12, &tmp, 4); //(PC inicia com o valor de tmp guardado na palavra 3 – bytes 12 a 15)
	tmp = 0x1001 + 3; //SP
	//SP (Stack Pointer) é o ponteiro para o topo da pilha.
	//A base da pilha é LV e ela já começa com algumas variáveis empilhadas (dependendo do programa).
	//Cada variável gasta uma palavra de memória. Por isso a soma de LV com num_of_vars.
	memcpy(memoria+16, &tmp, 4); //(SP inicia com o valor de tmp guardado na palavra 4 – bytes 16 a 19)
}

//Onde será feita a separação da microinstrução e as mi_operacaoções
void decodificar_microinstrucao(){
	mi_barramentoB = mi & 0b1111;			//Qual dos registradores será usado no barramento B
	mi_memoria = (mi >> 4) & 0b111;			//Qual operação será feita com a memoria principal
	mi_gravar = (mi >> 7) & 0b111111111;	//Qual dos registradores será gravado o barramento C
	mi_operacao = (mi >> 16) & 0b111111;	//Qual a operacaoção que será feita na ULA
	mi_deslocador = (mi >> 22) & 0b11;		//Qual será a operação feita pelo deslocador
	mi_pulo = (mi >> 24) & 0b111;			//Se haverá pulo ou não
	MPC = (mi >> 27) & 0b111111111;			//Qual será a próxima instruçãoss
		
}

//Faz a atribuição do barramento B
void atribuir_barramentoB(){
	switch(mi_barramentoB){
		case 0: bB = MDR;										break;
		case 1: bB = PC;										break;
		case 2: bB = MBR;										break;
		case 3: bB = MBR;
			if(MBR & (0b1 << 7)) bB = bB | (0b11111111 << 8);	break;
		case 4: bB = SP;										break;
		case 5: bB = LV;										break;
		case 6: bB = CPP;										break;
		case 7: bB = TOS;										break;
		case 8: bB = OPC;										break;
		default: bB = -1;										break;
	}
	
}

//Grava o resultado através do barramento C
void atribuir_registradores(){
	if(mi_gravar & 1)   MAR = bC;
	if(mi_gravar & 2)   MDR = bC;
	if(mi_gravar & 4)   PC  = bC;
	if(mi_gravar & 8)   SP  = bC;
	if(mi_gravar & 16)  LV  = bC;
	if(mi_gravar & 32)  CPP = bC;
	if(mi_gravar & 64)  TOS = bC;
	if(mi_gravar & 128) OPC = bC;
	if(mi_gravar & 256) H   = bC;
}

//Faz a mi_operacaoção do mi_pulo
void pular(){
	if(mi_pulo & 1) MPC = MPC | (zero << 8);
	if(mi_pulo & 2) MPC = MPC | (nzero << 8);
	if(mi_pulo & 4) MPC = MPC | MBR;

}

//Faz a mi_operacaoção da ULA
void ULA(){
	switch(mi_operacao){
		case 12: bC = H & bB;		break;
		case 17: bC = 1;			break;
		case 18: bC = -1;			break;
		case 20: bC = bB;			break;
		case 24: bC = H;			break;
		case 26: bC = ~H;			break;
		case 28: bC = H | bB;		break;
		case 44: bC = ~bB;			break;
		case 53: bC = bB + 1;		break;
		case 54: bC = bB - 1;		break;
		case 57: bC = H + 1;		break;
		case 59: bC = -H;			break;
		case 60: bC = H + bB;		break;
		case 61: bC = H + bB + 1;	break;
		case 63: bC = bB - H;		break;

		default: break;
	}
	
	//Verifica o resultado do zero e não zero da ULA
	if(bC){
		zero = 0;
		nzero = 1;
	}else{
		zero = 1;
		nzero = 0;
	}
	
	//Faz o deslocamento do mi_deslocador
	switch(mi_deslocador){
		case 1: bC = bC >> 1;		break;
		case 2: bC = bC << 8;		break;
	}
}

//Operações Fetch, Read, Write da memória
void operar_memoria(){
	switch(mi_memoria){
		case 1: MBR = memoria[PC]; 					break;//FEATCH
		case 2: memcpy(&MDR, &memoria[MAR*4], 4);	break;//READ
		case 4: memcpy(&memoria[MAR*4], &MDR, 4); 	break;//WRITE

		default: break;
	}
}

//Responsável por printar as informaçoes da ULA
void exibe_processo(){
	system("clear");
	cout << "\nMAR: " << MAR;
	cout << "\nMDR: " << MDR;
	cout << "\nPC : " << PC;
	cout << "\nMBR: " << (palavra)MBR;
	cout << "\nSP : " << SP;
	cout << "\nLV : " << LV;
	cout << "\nCPP: " << CPP;
	cout << "\nTOS: " << TOS;
	cout << "\nOPC: " << OPC;
	cout << "\nH  : " << H;
	cout << "\n";
	cout << "\nProxima instrucao MPC: " << MPC;
	cout << "\n";
	cout << "\nDividendo: " << (int)memoria[25*4];
	cout << "\nDivisor: " << (int)memoria[26*4];
	cout << "\nQuociente: " << (int)memoria[27*4];
	cout << "\nResto: " << (int)memoria[28*4];

	cout << "\nAux_resto: " << (int)memoria[30*4];
	cout << "\nAux_divisor: " << (int)memoria[31*4];

	cout << "\n";
	getchar();
}
