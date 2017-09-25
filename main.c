#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MFIELD 10
#define TAMANHOCAMPO 10 //será quando bits o nome da coluna ocupará

//Em tamanho Header temos o quanto em Bytes o cabeçalho da tabela ocupou na página.
//Assim o tamanho máximo dos slots deve ser 4KB - TamanhoHeader.
//Ainda não considerando o tamanho do vetor de bitmap.
int TamanhoHeader = 0,numeroSlots = 0,tmCampos = 0;
char *bitMap;

void buildHeader(){
	FILE *f;
	f = fopen("agenda.dat","w+");
	if (f==NULL){
		printf("File could not be created\n");
		exit(0);
    }
    /*	TAMANHO - É armazenado o valor que denota o tamanho do campo da coluna.
		--
		TIPO - Armazena o valor que denota o tipo do campo da coluna.
		--
		CONT - Temos o controle do número de colunas da tabela.*/
	int tamanho = 0, cont = 0;
	//opt e c são para controle de LOOP
	char opt, c, tipo;
	do{
		printf("tipo: ");
		tipo = getchar();
		if(tipo == 'S'){
			printf("tamanho: ");
			scanf("%d", &tamanho);
		} else if(tipo == 'I'){
			tamanho = sizeof(int);
		}
		//aqui calculamos a soma de todos os tamanhos dos campos
		tmCampos += tamanho;
		char nomeCampo[TAMANHOCAMPO];
		printf("nome do campo: ");
        scanf("%s",&nomeCampo);
		if (nomeCampo[strlen(nomeCampo)-1]!='\n')
			c = getchar();
		else
			nomeCampo[strlen(nomeCampo)-1]=0;
	    fwrite(nomeCampo,TAMANHOCAMPO,1,f);
	    fwrite(&tipo,1,1,f);
	    fwrite(&tamanho,sizeof(int),1,f);
		printf("Continuar (S/N): ");
		//Número de colunas da tabela
		cont++;
		opt = getchar();
	    while((c = getchar()) != '\n' && c != EOF); /// garbage collector
	}while(opt=='S' || opt=='s');
	char fname[TAMANHOCAMPO];
	strcpy(fname,"#");
    fwrite(fname,TAMANHOCAMPO+1+sizeof(int),MFIELD-cont,f);
    // o Cont é o número de colunas que o usuário escolheu, logo
    // é necessário multiplicar pelo TAMANHOCAMPO, tamanho do tipo e tamanho do campo da coluna.
    //Esse sizeof(int) a mais é para o tamanho da header
    TamanhoHeader = ((1+cont)*(TAMANHOCAMPO+1+sizeof(int))) + sizeof(int);
	tmCampos += 8;
	fwrite(&tmCampos,sizeof(int),1,f);
	numeroSlots = (4096 - TamanhoHeader) / (tmCampos);
	// +8 dos indexes do slot, dois inteiros um para a pag. e outro o nº do slot.
    numeroSlots = (4096 - TamanhoHeader - numeroSlots) / (tmCampos);
    // recalculado para discontar o espaço ocupado pelo bitMap.
	fwrite(&numeroSlots,sizeof(int),1,f);
	// gravar o numero de slots, pois quando só for feita a leitura do arquivo
	//tem q saber, quandos slots tem a pagina.
    char t = '0';
    for(cont = 0; cont < numeroSlots; cont++)
    	fwrite(&t,1,1,f);
    fclose(f);
}

struct theader {
	char name[TAMANHOCAMPO];
	char type;
	int  len;
};

union tint {
	char cint[sizeof(int)];
	int vint;
};

struct theader *readHeader(){
	FILE *f;
	struct theader *th=(struct theader*) malloc(sizeof(struct theader)*MFIELD);
	int i;
	f = fopen("agenda.dat","r");
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
	for (i=0;i<MFIELD;i++){
		fread(th[i].name,TAMANHOCAMPO,1,f);
		fread(&th[i].type,1,1,f);
		fread(&th[i].len,sizeof(int),1,f);
	}
	fread(&tmCampos,sizeof(int),1,f);
	fread(&numeroSlots,sizeof(int),1,f);
	// printf("nº slots: %d, tamanho campo: %d\n", numeroSlots, tmCampos);
	bitMap = (char *) malloc (numeroSlots* sizeof(int)); // alocação do vetor de bitMap
    for (i = 0; i < numeroSlots; i++)
    	fread(&bitMap[i],1,1,f);
	fclose(f);
	// printf("bitMap: ");
	// for (i = 0; i < numeroSlots; i++)
		// printf("%c - ", bitMap[i]);
	// printf("\n");
	return th;
}

int procurarPosicaoBitMap (){
	int i;
	for(i = 0; i < numeroSlots; i++)
		  if(bitMap[i] == '0')
				return i;
	return -1;
}

void setarPosicaoBitMap(int position, char valor){
	int i;
	for(i = 0; i < numeroSlots; i++)
		if(i == position)
			bitMap[i] = valor;
}

void gravarBitMap(){
	FILE *f;
	int i;
	char t;
	f = fopen("agenda.dat","r+");
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
    int hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int)) + sizeof(int)*2;
    fseek(f,hlen,SEEK_SET);
	for(i = 0; i < numeroSlots;i++)
		fwrite(&bitMap[i],1,1,f);
	fclose(f);
}

void insertSlots(){
	FILE *f;
	f = fopen("agenda.dat","r+");
	struct theader *t;
	t = readHeader();
	int i, nrPag = 1, hlen, slotLivre = 0;
	char conteudoSlot[tmCampos-8], c;
	union tint eint;
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
	slotLivre = procurarPosicaoBitMap();
	// sizeof(int)*2 = 1 é para o inteiro tmCampos e outro para o inteiro numeroSlots.
	hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int)) + numeroSlots + sizeof(int)*2 + slotLivre*tmCampos;
    fseek(f,hlen,SEEK_SET);
	if(slotLivre == -1){
		printf("Cheio, exclua algo.\n");
	} else {
		printf("(escrever)Slot nº %d\n",slotLivre);
		i = 0;
		if(t[i].name[0]!='#') {
			fwrite(&nrPag,sizeof(int),1,f);
			fwrite(&slotLivre,sizeof(int),1,f);
		}
		while (i<10 && t[i].name[0]!='#'){
			printf("\n%s: ",t[i].name);
			setbuf(stdin,NULL);
			//ver estouro de varivael
			switch (t[i].type){
				case 'S': fgets(conteudoSlot,t[i].len+1,stdin);
						  if (conteudoSlot[strlen(conteudoSlot)-1]!='\n')  c = getchar();
						  else conteudoSlot[strlen(conteudoSlot)-1]=0;
						  fwrite(conteudoSlot,t[i].len,1,f);
						break;
				case 'I': scanf("%d",&eint.vint);
						  while((c = getchar()) != '\n' && c != EOF); /// garbage collector
						  fwrite (&eint.vint,t[i].len,1,f);
						break;
			}
			i++;
		}
		setarPosicaoBitMap(slotLivre, '1');
	}
	gravarBitMap();
	fclose(f);
}

void removeSlots(){
	FILE *f;
	f = fopen("agenda.dat","r+");
	struct theader *t;
	t = readHeader();
	int hlen, slot, i, w, zero = 0;
	char limpa = ' ';
	union tint eint;
	if (f==NULL){
		printf("File not found\n");
		exit(0);
	}
	printf("nº do slot: ");
	scanf("%d", &slot);
	if(slot > numeroSlots){
		printf("slot invalido!");
	} else {
		// char conteudo[tmCampos-8];
		hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int)) + numeroSlots + sizeof(int)*2 + slot*tmCampos + 8;
		fseek(f,hlen,SEEK_SET);
		w = i = 0;
		while (i<10 && t[i].name[0]!='#'){
			switch (t[i].type){
				case 'S':
						for(w = 0; w < t[i].len; w++){
							fwrite(&limpa,1,1,f);
						}
						break;
				case 'I':
						fwrite(&zero,t[i].len,1,f);
						break;
			}
			i++;
		}
		setarPosicaoBitMap(slot, '0');
	}
	gravarBitMap();
	fclose(f);
}

void selectAll(){
    FILE *f;
	struct theader *t;
	int hlen, nrPag = 0, nrSlot = 0, i = 0, j, space;
	union tint eint;
	t = readHeader();
	f = fopen("agenda.dat","r");
	char conteudoSlot[tmCampos-8];
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
	int slot = 0, cont = 0;
	printf("nº do slot: ");
	scanf("%d", &slot); //12345 - é o comando para imprimir todos.
	hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int)) + numeroSlots + sizeof(int)*2;
	printf("P S ");
    while (i<10 && t[i].name[0]!='#'){
        printf("%s ",t[i].name);
        space = TAMANHOCAMPO - strlen(t[i].name);
		for (j=1;j<=space;j++)
          printf(" ");
        i++;
    }
	printf("\n");
    fseek(f,hlen,SEEK_SET);
	// printf("\n\n\n ---- Slots: %d\n --- Tamanho Slot: %d\n", numeroSlots, tmCampos);
    while(!feof(f)){
		i = 0;
		while (i<10 && t[i].name[0]!='#'){
			if(i == 0){
				fread(&nrPag,sizeof(int),1,f);
				fread(&nrSlot,sizeof(int),1,f);
				if(cont == slot || slot == 12345)
					printf("%d %d ",nrPag,nrSlot);
			}
  			if (!fread(conteudoSlot,t[i].len,1,f))
				break;
			if(cont == slot || slot == 12345)
				switch (t[i].type){
				    case 'S': for (j=0;j<t[i].len && conteudoSlot[j]!=0;j++)
				                 printf("%c",conteudoSlot[j]);
				              space=t[i].len-j;
	                          for (j=0;j<=space;j++)
	                              printf(" ");
					        break;
					case 'I': for (j=0;j< t[i].len;j++) eint.cint[j]=conteudoSlot[j];
					          printf("%d ",eint.vint);
					        break;
			    }
		    i++;
	    }
		if(cont == slot || slot == 12345) printf("\n");
		cont++;
    }
    fclose(f);
}

int main(){
	int opt;
	do{
		printf("0 - Nova Estrutura\n1 - Insert\n2-  Select\n3 - Remove\n** OUTRO VALOR P/ SAIR! **\nopt: ");
		scanf("%d", &opt);
		if(opt == 0)
			buildHeader();
		else if(opt == 1)
			insertSlots();
	    else if(opt == 2)
			selectAll();
		else if(opt == 3)
			removeSlots();
		else
			break;
		printf("\n\n\n---------------------\n");
	}while(1);
	free(bitMap);
	return 0;
}
