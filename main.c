#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MFIELD 10
#define TAMANHOCAMPO 10 //será quando bits o nome da coluna ocupará

//Em tamanho Header temos o quanto em Bytes o cabeçalho da tabela ocupou na página.
//Assim o tamanho máximo dos slots deve ser 4KB - TamanhoHeader.
//Ainda não considerando o tamanho do vetor de bitmap.
int TamanhoHeader = 0,tmSlots = 0,tmCampos = 0;
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
			// setbuf(stdin, NULL);
			scanf("%d", &tamanho);
		} else if(tipo == 'I'){
			tamanho = sizeof(int);
		}
		//aqui calculamos a soma de todos os tamanhos dos campos
		tmCampos += tamanho;
		char nomeCampo[TAMANHOCAMPO];
		printf("nome do campo: ");
        scanf("%s",&nomeCampo);
		if (nomeCampo[strlen(nomeCampo)-1]!='\n')  c = getchar();
		else nomeCampo[strlen(nomeCampo)-1]=0;
	    fwrite(nomeCampo,TAMANHOCAMPO,1,f);
	    fwrite(&tipo,1,1,f);
	    fwrite(&tamanho,sizeof(int),1,f);
		printf("Continuar (S/N): ");
		//Número de colunas da tabela
		cont++;
		opt = getchar();
	    while((c = getchar()) != '\n' && c != EOF); /// garbage collector
	} while (opt=='S' || opt=='s');
	char fname[TAMANHOCAMPO];
	strcpy(fname,"#");
    fwrite(fname,TAMANHOCAMPO+1+sizeof(int),MFIELD-cont,f);
    // o Cont é o número de colunas que o usuário escolheu, logo
    // é necessário multiplicar pelo TAMANHOCAMPO, tamanho do tipo e tamanho do campo da coluna.
    //Esse sizeof(int) a mais é para o tamanho da header
    TamanhoHeader = ((1+cont)*(TAMANHOCAMPO+1+sizeof(int))) + 2 + sizeof(int)+sizeof(int); 
    // +2 é por causa do colchetes
    
	tmSlots = (4096 - TamanhoHeader) / (tmCampos+8); 
	// +8 dos indexes do slot, dois inteiros um para a pag. e outro o nº do slot.
    tmSlots = (4096 - TamanhoHeader - tmSlots) / (tmCampos+8); 
    // recalculado para discontar o espaço ocupado pelo bitMap.
    fwrite(&TamanhoHeader,sizeof(int),1,f);
	fwrite(&tmSlots,sizeof(int),1,f); 
	// gravar o numero de slots, pois quando só for feita a leitura do arquivo 
	//tem q saber, quandos slots tem a pagina.
	char t = '[';
    fwrite(&t,1,1,f);
    t = '0';
    for(cont = 0; cont < tmSlots; cont++)
    	fwrite(&t,1,1,f);
    t = ']';
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
	int hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int))+ sizeof(int)+sizeof(int)+1;
    fseek(f,hlen,SEEK_SET);
	bitMap = (char *) malloc (tmSlots* sizeof(int)); // alocação do vetor de bitMap
    for (i = 0; i < tmSlots; i++)
    	fread(&bitMap[i],1,1,f);
	fclose(f);
	return th;
}
int procurarPosicaoBitMap () {
	int i;
	for(i = 0; i < tmSlots; i++) {
		  if(bitMap[i] == '0'){ 
				return i;
			}
	}
	return -1;
	//printf("%d - %c -",i,bitMap[i]);
	//printf("\n");
}

void setarPosicaoBitMap(int vlr) {
	int i;
	for(i = 0; i < tmSlots; i++) 
		if(i == vlr){
			bitMap[i] = '1';
		}
}

void gravarBitMap() {
	 FILE *f;
	int i;
	char t;
	f = fopen("agenda.dat","r+");
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
    int hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int))+ sizeof(int)+sizeof(int)+1;
    fseek(f,hlen,SEEK_SET);
	for(i = 0; i < tmSlots;i++) {
		fwrite(&bitMap[i],1,1,f);
	}
	fclose(f);
}

void insertSlots() {
	 FILE *f;
	struct theader *t;
	int i,slotCounter,nrPag = 1,hlen,slotLivre = 0;
	char opt, buf[100],c;
	union tint eint;
	t = readHeader();
	f = fopen("agenda.dat","r+");
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
	hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int));
	fseek(f,hlen,SEEK_SET);
	fread(&TamanhoHeader,sizeof(int),1,f);
	fread(&tmSlots,sizeof(int),1,f);
	//printf("%d %d\n",TamanhoHeader,tmSlots );
	hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int)) + tmSlots + 2 + sizeof(int)+sizeof(int);
    fseek(f,hlen,SEEK_SET);
    
    
    slotCounter= 0;
       do {
       	slotLivre = procurarPosicaoBitMap();
       	if(slotLivre == -1){
       		printf("Cheio, exclua algo.\n"); 
       		break;
       	} 
       	printf("%d livre e\n",slotLivre);
       	i=0;
       	  if(slotLivre  > slotCounter){
       	  	slotCounter++;
       	  	continue;
       	  }

       	if( slotCounter >=tmSlots){
       		printf("O Tamanho máximo de Slots foi atingido.\n");
       		break;
       	}
       	if(t[i].name[0]!='#') {
       		fwrite(&nrPag,sizeof(int),1,f);
       		fwrite(&slotCounter,sizeof(int),1,f);
       	}
       	while (i<10 && t[i].name[0]!='#'){
			printf("\n%s: ",t[i].name);
			setbuf(stdin,NULL);
			//ver estouro de varivael
			switch (t[i].type){
				case 'S': fgets(buf,t[i].len+1,stdin);
				          if (buf[strlen(buf)-1]!='\n')  c = getchar();
				          else buf[strlen(buf)-1]=0;
				          fwrite(buf,t[i].len,1,f);
				        break;
				case 'C': buf[0]=fgetc(stdin);
				          while((c = getchar()) != '\n' && c != EOF); /// garbage collector
				          fwrite(buf,t[i].len,1,f);
				        break;
				case 'I': scanf("%d",&eint.vint);
				          while((c = getchar()) != '\n' && c != EOF); /// garbage collector
				          fwrite (&eint.vint,t[i].len,1,f);
				        break;
		    }
		    i++;
	    }
	    printf("Continuar (S/N): "); opt=getchar();
	    while((c = getchar()) != '\n' && c != EOF); /// garbage collector
		setarPosicaoBitMap(slotCounter);
	    slotCounter++;
	} while (opt=='S' || opt=='s');
	gravarBitMap(); 
	fclose(f);
}

/*
void insert(){
    FILE *f;
	struct theader *t;
	int i;
	char opt, buf[100],c;
	union tint eint;
	t = readHeader();
	f = fopen("agenda.dat","a+");
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
    do {
		i=0;
		while (i<10 && t[i].name[0]!='#'){
			printf("\n%s: ",t[i].name);
			setbuf(stdin,NULL);
			//ver estouro de varivael
			switch (t[i].type){
				case 'S': fgets(buf,t[i].len+1,stdin);
				          if (buf[strlen(buf)-1]!='\n')  c = getchar();
				          else buf[strlen(buf)-1]=0;
				          fwrite(buf,t[i].len,1,f);
				        break;
				case 'C': buf[0]=fgetc(stdin);
				          while((c = getchar()) != '\n' && c != EOF); /// garbage collector
				          fwrite(buf,t[i].len,1,f);
				        break;
				case 'I': scanf("%d",&eint.vint);
				          while((c = getchar()) != '\n' && c != EOF); /// garbage collector
				          fwrite (&eint.vint,t[i].len,1,f);
				        break;
		    }
		    i++;
	    }
	    printf("Continuar (S/N): "); opt=getchar();
	    while((c = getchar()) != '\n' && c != EOF); /// garbage collector
	} while (opt=='S' || opt=='s');
	fclose(f);
}
*/

void selectAll(){
    FILE *f;
	struct theader *t;
	int hlen,nrPag = 0,nrSlot = 0,i = 0,j,space;
	char buf[100];
	union tint eint;
	t = readHeader();
	f = fopen("agenda.dat","r");
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
    while (i<10 && t[i].name[0]!='#'){
        printf("%s ",t[i].name);
        space= TAMANHOCAMPO - strlen(t[i].name);
		for (j=1;j<=space;j++)
          printf(" ");
        i++;
    }
	hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int));
	fseek(f,hlen,SEEK_SET);
	
	fread(&TamanhoHeader,sizeof(int),1,f);
	fread(&tmSlots,sizeof(int),1,f);
	//printf("%d %d\n",TamanhoHeader,tmSlots );
	hlen = MFIELD*(TAMANHOCAMPO+1+sizeof(int)) + tmSlots + 2 + sizeof(int)+sizeof(int);
    fseek(f,hlen,SEEK_SET);
     printf("\n");
    do{
		i=0;
		if(t[i].name[0]!='#' && i<10) {
			fread(&nrPag,sizeof(int),1,f);
			fread(&nrSlot,sizeof(int),1,f);
			printf("%d %d",nrPag,nrSlot);
		}
		while (i<10 && t[i].name[0]!='#'){
  			if (!fread(buf,t[i].len,1,f)) break;
			switch (t[i].type){
			    case 'S': for (j=0;j<t[i].len && buf[j]!=0;j++)
			                 printf("%c",buf[j]);
			              space=t[i].len-j;
                          for (j=0;j<=space;j++)
                              printf(" ");
				        break;
				case 'C': printf("%c ",buf[0]);
				        break;
				case 'I': for (j=0;j< t[i].len;j++) eint.cint[j]=buf[j];
				          printf("%d ",eint.vint);
				        break;
		    }
		    i++;
	    }
	    printf("\n");
    }while(!feof(f));
}

int main(){
	// buildHeader();
	insertSlots();
    selectAll();
	// int i;
	// for (i = 0; i < tmSlots; ++i)
	// 	printf("%c ", bitMap[i]);
	// free(bitMap);
	return 0;
}
