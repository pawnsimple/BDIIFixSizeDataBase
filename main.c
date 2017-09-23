#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* O esquema de dados é organizado da seguinte maneira:
	- Primeiros 15 caracteres bytes representam o contador de nomes de campo \ 0
	- Próximo 1 byte de caractere o tipo do campo (string S, caractere C e inteiro I)
	- Próximos bytes int, o comprimento do campo O esquema está localizado no
	cabeçalho do arquivo de dados e um cabeçalho pode ter no máximo 10 definições de campo.
	No caso de haver menos de 10 definições de campo, a próxima definição vazia terá o caractere # no nome do campo.*/

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
    /*
		TAMANHO - É armazenado o valor que denota o tamanho do campo da coluna.
		--
		TIPO - Armazena o valor que denota o tipo do campo da coluna.
		--
		CONT - Temos o controle do número de colunas da tabela.
	 */
	int tamanho = 0, cont = 0;
	//opt e c são para controle de LOOP
	char opt, c, tipo;
	do{
		printf("tipo: ");
		tipo = getchar();
		if(tipo == 'S'){
			printf("tamanho:");
			// setbuf(stdin, NULL);
			scanf("%d", &tamanho); 
		} else if(tipo == 'I'){
			tamanho = sizeof(int);
		}
		//aqui calculamos a soma de todos os tamanhos dos campos
		tmCampos += tamanho;
		char nomeCampo[TAMANHOCAMPO];
		printf("nome do campo:");
        scanf("%s",&nomeCampo);
		if (nomeCampo[strlen(nomeCampo)-1]!='\n')  c = getchar();
		else nomeCampo[strlen(nomeCampo)-1]=0;
	    fwrite(nomeCampo,TAMANHOCAMPO,1,f);
	    fwrite(&tipo,1,1,f);
	    fwrite(&tamanho,sizeof(int),1,f);
		printf("Continuar (S/N):");
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
    TamanhoHeader = (1+cont)*(TAMANHOCAMPO+1+sizeof(int) + 2);
	tmSlots = (4096 - TamanhoHeader) / (tmCampos+8); // numero de slots
    tmSlots = (4096 - TamanhoHeader - tmSlots) / (tmCampos+8); // numero de slots
    char t = '[';
    fwrite(&t,1,1,f);
    int a;
    t = '0';
    for(a = 0; a < tmSlots; a++) 
    	fwrite(&t,1,1,f);
    printf("---%d\n",tmSlots );
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
    char vetor[tmSlots];
    for (i = 0; i < tmSlots+1; i++){
    	if(i!=0){
    		fread(&vetor[i-1],1,1,f);
    		printf("%c - ", vetor[i-1]);
    	} else
    		fread(&vetor[0],1,1,f);

    }
    bitMap = vetor;
	fclose(f);
	return th;
}

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
			printf("\n%s :",t[i].name);
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

void selectAll(){
    FILE *f;
	struct theader *t;
	int hlen,i,j,space;
	char buf[100];
	union tint eint;
	t = readHeader();
	f = fopen("agenda.dat","r");
	if (f==NULL){
		printf("File not found\n");
		exit(0);
    }
	// hlen=MFIELD*(15+1+sizeof(int));
	// fseek(f,hlen,SEEK_SET);
	// read record a record
    i=0;
    while (i<10 && t[i].name[0]!='#'){
        printf("%s ",t[i].name);
        space= TAMANHOCAMPO - strlen(t[i].name);
		for (j=1;j<=space;j++)
          printf(" ");
        i++;
    }
    printf("\n");
    hlen=MFIELD*(TAMANHOCAMPO+1+sizeof(int));
    fseek(f,hlen+tmSlots+2,SEEK_SET);
    
    do{
		i=0;
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
	buildHeader();
	 insert();
    // selectAll();
	int i;
	for (i = 0; i < tmSlots; ++i)
	{
		printf("%c\n", &bitMap[i]);
	}
	return 0;
}
