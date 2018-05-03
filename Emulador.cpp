#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
//100000000
#define TAMANHO_RAM 100000000

using namespace std;

typedef unsigned int palavra; 				//32 bits
typedef unsigned char byte;					//8 bits
typedef unsigned long int microinstrucao;	//64 bits

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
byte memoria[TAMANHO_RAM];

void decodificar_microinstrucao();
void ULA();
void atribuir_barramentoB();
void atribuir_registradores();
void pular();
void operar_memoria();

void exibe_processo();

void carrega_microprograma();
void carrega_programa(const char *arquivo);

void binario(void *, int);

int main(int argc, const char *argv[]){
	carrega_microprograma();
	carrega_programa(argv[1]);

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
}

void carrega_programa(const char *arquivo){
	FILE *prog;
	palavra tamanho;
	byte tam_arquivo[4];

	prog = fopen(arquivo, "rb");
	
	if (prog != NULL) {
		fread(tam_arquivo, sizeof(byte), 4, prog);
		memcpy(&tamanho, tam_arquivo, 4);

		if (tamanho <= TAMANHO_RAM) {
			fread(memoria, sizeof(byte), 20, prog);
			fread(&memoria[0x0401], sizeof(byte), tamanho-20, prog);
		} else {
			cout << "Instale mais memória!!!\n";
			exit(1);
		}

		fclose(prog);

	}
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

	if(mi_memoria & 1) MBR = memoria[PC];					//FEATCH
	if(mi_memoria & 2) memcpy(&MDR, &memoria[MAR*4], 4);	//READ
	if(mi_memoria & 4) {									//WRITE
		if((SP*4)+3 < TAMANHO_RAM) memcpy(&memoria[MAR*4], &MDR, 4);
		else {
			cout << "Instale mais memória!!!\n";
			exit(1);
		}	
	}
}

//Responsável por printar as informaçoes da ULA
void exibe_processo(){
	system("clear");
	
	cout << "\n  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
	cout << "\n  ~~~~~~~~~~~~~~    EMULADOR IJVM-1    ~~~~~~~~~~~~~";
	cout << "\n  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";	
	
	int base;
	if (LV && SP) {

		cout << "\n\t-------     PILHA DE EXECUÇÃO     -------\n";
		cout << "\n\t\tBINÁRIO\t\t        PALAVRA   INT\n";
		for (int i = SP; i >= LV; i--) {
			palavra valor;
			memcpy(&valor, &memoria[i*4], 4);

			binario(&valor , 1); cout << "\t "<< i; cout << "\t  " << (int)valor; cout << "\n";
		}
		cout << "\n\t------- FIM DA PILHA DE EXECUÇÃO -------\n";
	}
	cout << "\n\n    :::::::::      REGISTRADORES      :::::::::\n";
	cout << "\n\t\t\tBINÁRIO\t\t         INT";
	cout << "\n  MAR :  ";		  binario(&MAR , 3); cout << "      " << MAR;
	cout << "\n  MDR :  ";   	  binario(&MDR , 3); cout << "      " << MDR;
	cout << "\n  PC  :  "; 		  binario(&PC  , 3); cout << "      " << PC;
	cout << "\n  MBR :   \t\t\t ";binario(&MBR , 2); cout << "      " << (palavra)MBR;
 	cout << "\n  SP  :  ";		  binario(&SP  , 3); cout << "      " << SP;
	cout << "\n  LV  :  ";		  binario(&LV  , 3); cout << "      " << LV;
	cout << "\n  CPP :  ";		  binario(&CPP , 3); cout << "      " << CPP;
	cout << "\n  TOS :  ";		  binario(&TOS , 3); cout << "      " << TOS;
	cout << "\n  OPC :  ";	  	  binario(&OPC , 3); cout << "      " << OPC;
	cout << "\n  H   :  ";		  binario(&H   , 3); cout << "      " << H;

	cout << "\n\n    :::::::::   FIM DOS REGISTRADORES  :::::::::\n";
	cout << "\n";
	cout << "\n\t\t  Próxima instrução\n  MPC :  "; binario(armazenamento + MPC , 4); cout << "  "<< MPC;

	cout << "\n\n\n  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	cout << "  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

	getchar();
}


//Imprime o valor de uma palavra em binário
//tipo 1: Imprime o binário de 4 bytes seguidos
//tipo 2: Imprime o binário de apenas um byte
//tipo 3: Imprime o binário de uma palavra
//tipo 4: Imprime o binário de uma microinstrução

void binario(void *valor, int tipo){
	
	printf("  ");
	if(tipo == 1){
		byte aux;
		byte* valorAux = (byte*)valor;
			
		for(int i = 3; i >= 0; i--){
			aux = *(valorAux + i);
			for(int j = 0; j < 8; j++){
				printf("%d", (aux >> 7) & 0b1);
				aux = aux << 1;
			}
			printf(" ");
		}
	}

	else if(tipo == 2){
		byte aux;
		
		aux = *((byte*)(valor));
		for(int j = 0; j < 8; j++){
			printf("%d", (aux >> 7) & 0b1);
			aux = aux << 1;
		}
	}

	else if(tipo == 3){
		palavra aux;
		
		aux = *((palavra*)(valor));
		for(int j = 0; j < 32; j++){
			printf("%d", (aux >> 31) & 0b1);
			aux = aux << 1;
		}
	}

	else if(tipo == 4){
		microinstrucao aux;
		
		aux = *((microinstrucao*)(valor));
		for(int j = 0; j < 36; j++){
			printf("%ld", (aux >> 35) & 0b1);
			aux = aux << 1;
		}
	}
}
