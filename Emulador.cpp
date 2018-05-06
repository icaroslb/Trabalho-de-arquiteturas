#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#define TAMANHO_RAM 100000000
#define TRUE 1
#define FALSE 0

using namespace std;

typedef unsigned int palavra; 				//32 bits
typedef unsigned char byte;					//8 bits
typedef unsigned long int microinstrucao;	//64 bits

//Registradores
palavra MAR = 0, MDR = 0, PC = 0; 	//ACESSO MEMÓRIA
byte MBR = 0; 						//ACESSO MEMÓRIA
palavra SP = 0, LV = 0, CPP = 0, TOS = 0, OPC = 0, H = 0;//OPERAÇÃO NA ULA

//Barramentos
palavra bB, bC;

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

void decodificar_microinstrucao();	//Separa a microinstrução e atribui as variáveis de apoio
void atribuir_barramentoB();		//Envia para o barramento B o registrador solicitado na microinstrução
void ULA();							//Realiza as operações da ULA
void atribuir_registradores();		//Envaminha o resultado da ULA para os registradores
void pular();						//Realiza os pulos se zero, não zero e MPC caso seja solicitado
void operar_memoria();				//Realiza as operações FEATCH, READ E WRITE na memória

void exibe_processo();				//Exibe as informações que são atualizadas no Emulador

void carrega_microprograma();		//Lê o arquivo microprog.rom e carrega o microprograma para o armazenamento
void carrega_programa(const char *arquivo); //Lê o arquivo passado como argumento e carrega o programa na memória

void binario(void *valor, int tipo); //Mostra os valores em binário

int main(int argc, const char *argv[]){
	carrega_microprograma();
	carrega_programa(argv[1]);

	while(true){
		exibe_processo();
		//Atualiza a microinstrução atual
		mi = armazenamento[MPC];

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
		//Carrega os primeiros 4 bytes que contém o tamanho do arquivo para um vetor e depois carrega esse vetor na variável tamanho.
		fread(tam_arquivo, sizeof(byte), 4, prog);
		memcpy(&tamanho, tam_arquivo, 4);

		//Carrega os 20 primeiros bytes que contém a inicialização do programa para os primeiros 20 bytes da memória
		fread(memoria, sizeof(byte), 20, prog);

		//Carrega o programa na memória a partir da posição PC
		fread(&memoria[0x0401], sizeof(byte), tamanho-20, prog);
	
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
	//Carrega um registrador para o barramento B
	switch(mi_barramentoB){
		case 0: bB = MDR;										break;
		case 1: bB = PC;										break;
		case 2: bB = MBR;										break;
		//O caso 3 carrega o MBR com sinal fazendo a extensão de sinal, ou seja, copia-se o bit mais significativo do MBR para as 24 posições mais significativas do barramento B.
		case 3: bB = MBR;
			if(MBR & (0b10000000))
				bB = bB | (0b111111111111111111111111 << 8);	
																break;
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
	//Pode atribuir vários registradores ao mesmo tempo dependendo se mi_gravar possui bit alto para o registrador correspondente
	if(mi_gravar & 0b000000001)   MAR = bC;
	if(mi_gravar & 0b000000010)   MDR = bC;
	if(mi_gravar & 0b000000100)   PC  = bC;
	if(mi_gravar & 0b000001000)   SP  = bC;
	if(mi_gravar & 0b000010000)   LV  = bC;
	if(mi_gravar & 0b000100000)   CPP = bC;
	if(mi_gravar & 0b001000000)   TOS = bC;
	if(mi_gravar & 0b010000000)   OPC = bC;
	if(mi_gravar & 0b100000000)   H   = bC;
}

//Faz a mi_operacaoção do mi_pulo
void pular(){
	//Realiza o pulo se a saída da ULA for zero
	if(mi_pulo & 0b001) MPC = MPC | (zero << 8);
	//Realiza o pulo se a saída da ula for diferente de zero
	if(mi_pulo & 0b010) MPC = MPC | (nzero << 8);
	//Pula para a posição do MBR
	if(mi_pulo & 0b100) MPC = MPC | MBR;

}

//Faz a mi_operacaoção da ULA
void ULA(){
	switch(mi_operacao){
		//Cada operação da ULA é representado pela sequencia dos bits de operação. Cada operação válida foi convertida para inteiro para facilitar a escrita do switch
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
	
	//Verifica o resultado da ula e atribui as variáveis zero e nzero
	
	if(bC) { //Se bC for diferente de zero
		zero = FALSE;
		nzero = TRUE;
	} else { //Se bC for igual a zero
		zero = TRUE;
		nzero = FALSE;
	}
	
	//Faz o deslocamento do mi_deslocador
	switch(mi_deslocador){
		//Faz o deslocamento em um bit para direita
		case 1: bC = bC >> 1;		break;
		//Faz o deslocamento em 8 bits para a esquerda
		case 2: bC = bC << 8;		break;
	}
}

//Operações Fetch, Read, Write da memória
void operar_memoria(){
	if(mi_memoria & 0b001) MBR = memoria[PC];					//FEATCH
	//MDR recebe os 4 bytes referente a palavra MAR 
	if(mi_memoria & 0b010) memcpy(&MDR, &memoria[MAR*4], 4);	//READ
	//Os 4 bytes na memória da palavra MAR recebem o valor de MDR
	if(mi_memoria & 0b100) memcpy(&memoria[MAR*4], &MDR, 4);	//WRITE

}

//Responsável por printar as informaçoes da ULA
void exibe_processo(){
	system("clear");

	cout <<   " ███████████████████████████████████████████████████████";	
	cout << "\n ███████████████████  EMULADOR IJVM  ███████████████████";
	cout << "\n ███████████████████████████████████████████████████████\n";	
	
	int base;

	//Exibe a pilha de operandos quando o emulador já realizou a inicialização
	if (LV && SP) {
		cout << "\n                  ╔════════════════════╗";
		cout << "\n   ═══════════════╣ PILHA DE OPERANDOS ╠═════════════";
		cout << "\n                  ╚════════════════════╝";
		cout << "\n\t\t\t\t       ENDEREÇO";
		cout << "\n\t\t BINÁRIO\t\t  DE      INT";
		cout << "\n\t\t        \t\tPALAVRA\n";

		//Exibe a área delimitada por SP e LV para mostrar a pilha de operandos
		for (int i = SP; i >= LV; i--) {
			palavra valor;
			memcpy(&valor, &memoria[i*4], 4);

			binario(&valor , 1); cout << "\t "<< i; cout << "\t  " << (int)valor; cout << "\n";
		}
		cout << "   ══════════════════════════════════════════════════\n";

	}

	//Exibe a área do programa quando o Emulador já realizou a inicialização
	if (PC >= 0x0401) {
		cout << "\n                  ╭──────────────────╮";
		cout << "\n   ───────────────┤ ÁREA DO PROGRAMA ├───────────────";
		cout << "\n                  ╰──────────────────╯";
		cout << "\n\t\t                       ENDEREÇO";
		cout << "\n\t\t BINÁRIO        HEXA      DE      INT";
		cout << "\n\t\t                         BYTE\n";

		//Exibe a área ao redor de PC para mostrar trechos do programa 
		//que o Emulador está executando no momento
		for (int i = PC-2; i <= PC+3; i++) {
			if (i == PC) cout << "  Em execução ►";
			else cout << "\t       ";
			
			binario(&memoria[i], 2);
			printf("\t0x%02X", memoria[i]); 
			cout << "\t "<< i; 
			cout << "\t  " << (int)memoria[i];
			cout << "\n";
		}

		cout << "   ───────────────────────────────────────────────────\n";
	}

	//Exibe os registradores
	cout << "\n                   ▄■■■■■■■■■■■■■■■■■▄";
	cout << "\n   ■■■■■■■■■■■■■■■█   REGISTRADORES   █■■■■■■■■■■■■■■■■";
	cout << "\n                   ▀■■■■■■■■■■■■■■■■■▀";
	cout << "\n\n\t\t\t  BINÁRIO\t           INT\n";
	cout << "\n    MAR :  ";		  binario(&MAR , 3); cout << "      " << MAR;
	cout << "\n    MDR :  ";   	 	  binario(&MDR , 3); cout << "      " << MDR;
	cout << "\n    PC  :  "; 	 	  binario(&PC  , 3); cout << "      " << PC;
	cout << "\n    MBR :\t\t\t   ";   binario(&MBR , 2); cout << "      " << (palavra)MBR;
 	cout << "\n    SP  :  ";		  binario(&SP  , 3); cout << "      " << SP;
	cout << "\n    LV  :  ";		  binario(&LV  , 3); cout << "      " << LV;
	cout << "\n    CPP :  ";		  binario(&CPP , 3); cout << "      " << CPP;
	cout << "\n    TOS :  ";		  binario(&TOS , 3); cout << "      " << TOS;
	cout << "\n    OPC :  ";	  	  binario(&OPC , 3); cout << "      " << OPC;
	cout << "\n    H   :  ";		  binario(&H   , 3); cout << "      " << H;

	cout << "\n\n            ENDEREÇO DA PRÓXIMA MICROINSTRUÇÃO";
	cout << "\n    MPC :\t\t\t  ";	  binario(&MPC , 5); cout << "      "<< MPC;

	cout << "\n   ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n";

	//Exibe a microinstrução que a ula está operando atualmente
	cout << "\n              ◄♦♦♦ MICROINSTRUÇÃO ATUAL ♦♦♦►";  
	cout << "\n        Addr    JAM    ULA         C      Mem   B";
	cout << "\n   "; binario(&mi, 4);

	cout << "\n\n ███████████████████████████████████████████████████████\n";
	cout << " ███████████████████████████████████████████████████████  ";

	getchar();
}


//Imprime o valor de uma palavra em binário
//tipo 1: Imprime o binário de 4 bytes seguidos
//tipo 2: Imprime o binário de apenas um byte
//tipo 3: Imprime o binário de uma palavra
//tipo 4: Imprime o binário de uma microinstrução
//tipo 5: Imprime os 9 bits do MPC

void binario(void *valor, int tipo){
	printf("  ");
	switch (tipo) {
		case 1: {
			printf(" ");
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
		break;

		case 2: {
			byte aux;
			
			aux = *((byte*)(valor));
			for(int j = 0; j < 8; j++){
				printf("%d", (aux >> 7) & 0b1);
				aux = aux << 1;
			}
		}
		break;
		
		case 3: {
			palavra aux;
			
			aux = *((palavra*)(valor));
			for(int j = 0; j < 32; j++){
				printf("%d", (aux >> 31) & 0b1);
				aux = aux << 1;
			}
		}
		break;
		
		case 4: {
			microinstrucao aux;
		
			aux = *((microinstrucao*)(valor));
			for(int j = 0; j < 36; j++){
				if ( j == 9 || j == 12 || j == 20 || j == 29 || j == 32) cout << "  ";

				printf("%ld", (aux >> 35) & 0b1);
				aux = aux << 1;
			}
		}
		break;

		case 5: {
			palavra aux;
		
			aux = *((palavra*)(valor)) << 23;
			for(int j = 0; j < 9; j++){
				printf("%d", (aux >> 31) & 0b1);
				aux = aux << 1;
			}
		}
		break;

	}
}
