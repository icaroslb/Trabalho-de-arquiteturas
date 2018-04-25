#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
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
byte memoria[1100];

void decodificar_microinstrucao();
void ULA();
void atribuir_barramentoB();
void atribuir_registradores();
void pular();
void operar_memoria();

void exibe_processo();

void binario(byte *, int);

int main(){
	//Microinstrução que vai contar de 1 a 4 no LV e somar 4 ao LV
	//armazenamento[0] = 0b000000001000001101010000100000000101;
	//armazenamento[1] = 0b000000010000001101010000100000000101;
	//armazenamento[2] = 0b000000011000001101010000100000000101;
	//armazenamento[3] = 0b000000100000001101010000100000000101;
	//armazenamento[4] = 0b000000101000000101001000000000000101;
	//armazenamento[5] = 0b000000101000001111000000100000000101;

	//PROGRAMA DA MEMORIA

	memoria[1] = 2;			memoria[2] = 25;
	memoria[3] = 6;			memoria[4] = 28;
	memoria[5] = 13;		memoria[6] = 25;
	memoria[7] = 2;			memoria[8] = 28;
	memoria[9] = 11;		memoria[10] = 27;
	memoria[11] = 13;		memoria[12] = 26;
	memoria[13] = 6;		memoria[14] = 28;
	memoria[15] = 13;		memoria[16] = 28;
	memoria[17] = 2;		memoria[18] = 27;
	memoria[19] = 2;		memoria[20] = 29;
	memoria[21] = 6;		memoria[22] = 27;
	memoria[23] = 13;		memoria[24] = 27;
	memoria[25] = 9;		memoria[26] = 7;
	memoria[27] = 9;		memoria[28] = 27;

	memoria[25*4] = 100;
	memoria[26*4] = 50;
	memoria[27*4] = 0;
	memoria[28*4] = 0;
	memoria[29*4] = 1;

	//FIRMWARE
	//MAIN
	armazenamento[0] = 0b000000000100001101010000001000010001; //PC <- PC + 1; fetch; GOTO MBR;

	//OPC = OPC + memory[end_word];  (Linguagem de máquina: Byte 2; Sintaxe assembly: ADD OPC, [END])
	armazenamento[2] = 0b000000011000001101010000001000010001; //PC <- PC + 1; fetch;
	armazenamento[3] = 0b000000100000000101000000000010100010; //MAR <- MBR; read;
	armazenamento[4] = 0b000000101000000101001000000000000000; //H <- MDR;
	armazenamento[5] = 0b000000000000001111000100000000001000; //OPC <- OPC + H; GOTO MAIN;

	//memory[end_word] = OPC;  (Linguagem de maquina: Byte 6; Sintaxe assembly: MOV OPC, [END])
	armazenamento[6] = 0b000000111000001101010000001000010001; //PC <- PC + 1; fetch;
	armazenamento[7] = 0b000001000000000101000000000010000010; //MAR <- MBR;
	armazenamento[8] = 0b000000000000000101000000000101001000; //MDR <- OPC; write; GOTO MAIN;

	//goto endereco_comando_programa; (Linguagem de mÃ¡quina: Byte 9; Sintaxe assembly: GOTO byte)
	armazenamento[9]  = 0b000001010000001101010000001000010001; //PC <- PC + 1; fetch;
	armazenamento[10] = 0b000000000100000101000000001000010010; //PC <- MBR; fetch; GOTO MBR;

	//if OPC = 0 goto endereco_comando_programa else goto proxima_linha; (Linguagem de mÃ¡quina: Byte 11; Sintaxe assembly: JZ OPC, byte)
	armazenamento[11]  = 0b000001100001000101000100000000001000; //OPC <- OPC; IF ALU = 0 GOTO 268 (100001100) ELSE GOTO 12 (000001100);
	armazenamento[12]  = 0b000000000000001101010000001000000001; //PC <- PC + 1; GOTO MAIN;
	armazenamento[268] = 0b100001101000001101010000001000010001; //PC <- PC + 1; fetch;
	armazenamento[269] = 0b000000000100000101000000001000010010; //PC <- MBR; fetch; GOTO MBR;

	//OPC = OPC - memory[end_word];  (Linguagem de maquina: Byte 13; Sintaxe assembly: SUB OPC, [END])
	armazenamento[13] = 0b000001110000001101010000001000010001; //PC <- PC + 1; fetch;
	armazenamento[14] = 0b000001111000000101000000000010100010; //MAR <- MBR; read;
	armazenamento[15] = 0b000010000000000101001000000000000000; //H <- MDR;
	armazenamento[16] = 0b000000000000001111110100000000001000; //OPC <- OPC - H; GOTO MAIN;


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
		case 0: bB = MDR;			break;
		case 1: bB = PC;			break;
		case 2: bB = MBR;			break;
		case 3: bB = MBR;			break;
		case 4: bB = SP;			break;
		case 5: bB = LV;			break;
		case 6: bB = CPP;			break;
		case 7: bB = TOS;			break;
		case 8: bB = OPC;			break;
		default: bB = -1;			break;
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

	getchar();
}

//Imprime o valor de uma palavra em binário
void binario(byte *valor, int tam){
	byte aux;
	
	for(int i = tam-1; i >= 0; i--){
		aux = *(valor+tam);
		for(int j = 0; j < 8; j++){
			printf("%d", (aux >> 7) & 0b1);
			aux = aux << 1;
		}
		printf(" ");
	}
}
