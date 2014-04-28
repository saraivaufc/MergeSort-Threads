#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define pause sleep(100000);

using namespace std;

FILE * desordenado;
FILE * ordenado;

void imprime_arquivos(FILE *arq,char *modo, char *nome, int * vet, int tam){
	arq=fopen(nome,modo);
	for(int i=0 ; i<tam ;i++){
		fprintf(arq,"[%d] ",vet[i]);
	};
	fclose(arq);
};

int quant_nucleos;

int cont=1;

void retorna_nucleos_unix(){
	system("cat /proc/cpuinfo | grep processor | wc -l  >> nucleos.txt");
	FILE * nucleos;
	nucleos=fopen("nucleos.txt" , "r+");
	fscanf(nucleos,"%d",&quant_nucleos);
	fclose(nucleos);
	system("rm nucleos.txt");
}

void retorna_nucleos_dos(){

}

#ifdef __unix__


#define SO 1

#elif defined(_WIN32) || defined(WIN32)

#define SO 2

#endif

typedef struct{
	int *vet;
	int inicio;
	int fim;
	int tam;
}Vetor;

void swap(int *a, int *b){
	*a=*a^*b;
	*b=*a^*b;
	*a=*a^*b;
};

void quicksort(int *vetor, int tamanho){
	if(tamanho <= 1)
		return ;
	else{
		int pivo=vetor[0];
		int inicio=1;
		int fim=tamanho-1;
	do{
		while(inicio < tamanho && vetor[inicio] <= pivo)
			inicio++;
		while(vetor[fim] > pivo)
			fim--;
		if(inicio < fim){
			swap(&vetor[inicio] , &vetor[fim]);
			inicio++;
			fim--;
		}
	}while(inicio<=fim);

	vetor[0]=vetor[fim];
	vetor[fim]=pivo;

	quicksort(vetor, fim);
	quicksort(&vetor[inicio] , tamanho-inicio);
	}
}

void merge(Vetor *l,int quant_partes, int *tam_partes){
	int ** matriz=(int **) malloc(quant_partes * sizeof(int *));
	for(int i=0;i<quant_partes;i++){
		matriz[i]=(int *)malloc(tam_partes[i] * sizeof(int));
	}

	for(int i=0;i<quant_partes;i++){
		for(int k=0;k<tam_partes[i];k++){
			matriz[i][k]=l->vet[k+(i*tam_partes[0])];
		}
	}

	for(int i=0;i<quant_partes;i++){
		for(int k=0;k<tam_partes[i];k++){
			cout<<"["<<matriz[i][k]<<"]";
		}
		cout<<endl;
	}



}


int threads_criadas=1;
void * entrelaca(void *l){
	Vetor * aux=(Vetor *)l;
	int inicio=aux->inicio;
	int tamanho=aux->fim;

	cout<<endl<<threads_criadas++<<"ª Threads criada..."<<endl;
	quicksort(&aux->vet[inicio],tamanho);
	return (void *)l;
}

void merge_t(Vetor *l,pthread_t *vet_threads){
	int pedaco=l->tam/quant_nucleos;

	int tam_partes[quant_nucleos];//vetor que armazenara o tamanho de cada parte
	
	l->inicio=0;
	l->fim=pedaco;
	cout<<"Pedacos do tamanho:"<<pedaco<<endl;
	int i;
	for(i=0;i<quant_nucleos-1;i++){
		tam_partes[i]=l->fim;

		pthread_create(&(vet_threads[i]), NULL, entrelaca,(void *)l);
		sleep(2);//depois verificar isso
		l->inicio+=pedaco;
		if(i == quant_nucleos-2){
			i++;
			break;
		}
	}
	l->fim=l->tam - l->inicio;
	tam_partes[i]=l->fim;
	pthread_create(&(vet_threads[i]), NULL, entrelaca,(void *)l);

	for(int i=0; i<quant_nucleos; i++) {
		pthread_join(vet_threads[i], NULL);
	}

	merge(l,quant_nucleos,tam_partes);
}

int main(){
	srand(time(NULL));
	if(SO == 1){
		retorna_nucleos_unix();
	}else{
		retorna_nucleos_dos();
	}
	//Após isso eu terei uma variavel Global <nucleo> referênte a quantidade de nucleos te processador
	cout<<"Voce Possui "<<quant_nucleos<<" Nucleos em sua CPU..."<<endl;
	pthread_t threads[quant_nucleos];
	Vetor *L=(Vetor *) malloc(sizeof(Vetor));
	int tam=10;
	L->vet=(int *) malloc(tam*sizeof(int));
	L->tam=tam;
	for(int i=0 ; i<L->tam;i++){
		L->vet[i]=rand()%L->tam;
	}
	cout<<"Desordenado:";
	char des[]="Vetor_Desordenado.txt";
	char ord[]="Vetor_Ordenado.txt";
	char modo[]="w+";
	imprime_arquivos(desordenado,&modo[0],&des[0],L->vet,L->tam);

	merge_t(L,threads);
	imprime_arquivos(ordenado,&modo[0],&ord[0],L->vet,L->tam);
	cout<<endl;
	return 0;
}
