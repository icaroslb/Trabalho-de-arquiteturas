#include <iostream>
using namespace std;

typedef unsigned int palavra;
typedef unsigned char byte;
typedef unsigned long int microinstrucao;

palavra MAR=0, MDR=0, PC=0, SP=0, LV=0, CPP=0, TOS=0, OPC=0, H=0, bA, bB, bC; //Registradores e barramentos
byte MBR=0, deslocador=0, zero=0, nzero=0; //Informações para o MPC e desclocador
byte posB, opera, pulo; //Separação da microinstrução
palavra gravar, MPC; //Separação da microinstrução

void interpretador(microinstrucao []);
void ULA();

int main(){
	microinstrucao memoria[512];
	MPC=0;
	
	//Microinstrução que vai contar de 1 a 4 no LV e somar 4 ao LV
	memoria[0]=0b000000001000001101010000100000101;
	memoria[1]=0b000000010000001101010000100000101;
	memoria[2]=0b000000011000001101010000100000101;
	memoria[3]=0b000000100000001101010000100000101;
	memoria[4]=0b000000101000000101001000000000101;
	memoria[5]=0b000000101000001111000000100000101;
		
	while(true){
		interpretador(memoria);
		
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
		getchar();
	}
	return 0;
}

//Onde será feita a separação da microinstrução e as operações
void interpretador(microinstrucao memoria[]){
	posB=memoria[MPC] & 0b1111;					//Qual dos registradores será usado no barramento B
	gravar=(memoria[MPC] >> 4) & 0b111111111;	//Qual dos registradores será gravado o barramento C
	opera=(memoria[MPC] >> 13) & 0b111111;		//Qual a operação que será feita na ULA
	deslocador=(memoria[MPC] >> 19) & 0b11;		//Qual será a operação feita pelo deslocador
	pulo=(memoria[MPC] >> 21) & 111;			//Se haverá pulo ou não
	MPC=(memoria[MPC] >> 24) & 0b111111111;		//Qual será a próxima instrução
	
	//Faz a atribuição do barramento B
	switch(posB){
		case 0: bB=MDR;			break;
		case 1: bB=PC;			break;
		case 2: bB=MBR;			break;
		case 3: bB=MBR;			break;
		case 4: bB=SP;			break;
		case 5: bB=LV;			break;
		case 6: bB=CPP;			break;
		case 7: bB=TOS;			break;
		case 8: bB=OPC;			break;
		default: bB=-1;			break;
	}
	
	ULA();
	
	//Faz a operação do pulo
	if(pulo & 1) MPC=MPC | zero << 7;
	if(pulo & 2) MPC=MPC | nzero << 7;
	if(pulo & 4) MPC=MPC | MBR;
	
	//Grava o resultado através do barramento C
	if(gravar & 1) MAR=bC;
	if(gravar & 2) MDR=bC;
	if(gravar & 4) PC=bC;
	if(gravar & 8) SP=bC;
	if(gravar & 16) LV=bC;
	if(gravar & 32) CPP=bC;
	if(gravar & 64) TOS=bC;
	if(gravar & 128) OPC=bC;
	if(gravar & 256) H=bC;
}

//Faz a operação da ULA
void ULA(){
	switch(opera){
		case 24: bC=H;			break;
		case 20: ;				break;
		case 26: ;				break;
		case 44: ;				break;
		case 60: bC=H+bB;		break;
		case 61: bC=H+bB+1;		break;
		case 57: bC=H+1;		break;
		case 53: bC=bB+1;		break;
		case 63: bC=bB-H;		break;
		case 54: bC=bB-1;		break;
		case 59: bC=-H;			break;
		case 12: bC=H & bB;		break;
		case 28: bC=H | bB;		break;
		case 17: bC=1;			break;
		case 18: bC=-1;			break;
		default: break;
	}
	
	//Verifica o resultado do zero e não zero da ULA
	if(bC){
		zero=0;
		nzero=1;
	}else{
		zero=1;
		nzero=0;
	}
	
	//Faz o deslocamento do deslocador
	switch(deslocador){
		case 1: bC=bC >> 1;
		case 2: bC=bC << 8;
	}
}
