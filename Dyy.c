
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct A {

    int Tag;
    int BitValidade;
    int Info;

}AA;


int main( int argc, char *argv[ ] ) {

    
    
    // argc (argument count) → número de argumentos passados pela linha de comando.

    // argv[] (argument vector) → vetor de strings que contém os argumentos.
    
	if (argc != 7){

        printf("Numero de argumentos incorreto. Utilize:\n");
        
		printf("./cache_simulator <nsets> <bsize> <assoc> <substituição> <flag_saida> arquivo_de_entrada\n");
        
		exit(EXIT_FAILURE);
        
	}
    
	int nsets = atoi(argv[1]);
	int bsize = atoi(argv[2]);
	int assoc = atoi(argv[3]);
	char *subst = argv[4];
	int flagOut = atoi(argv[5]);
	char *arquivoEntrada = argv[6];

	printf("nsets = %d\n", nsets);
	printf("bsize = %d\n", bsize);
	printf("assoc = %d\n", assoc);
	printf("subst = %s\n", subst);
	printf("flagOut = %d\n", flagOut);
	printf("arquivo = %s\n", arquivoEntrada);

    int tamanhoCache = nsets * bsize * assoc;
   
    //  Interpretação:

    // Cada conjunto tem assoc vias.

    // Cada via armazena 1 bloco de bsize bytes.

    // A cache tem nsets conjuntos → logo, ela tem nsets × assoc vias no total.


    int bitIndice = log2(nsets);

    int bitOffset = log2(bsize);

    int bitTag = 32 - bitOffset - bitIndice;


    int hit = 0;

    int miss = 0;

    // Aloca espaço para cada conjunto
    AA ** LinhaCache = malloc(nsets * sizeof(AA*));

    for (int i = 0; i < nsets; i++) {
        
        // Aloca espaço para cada via
        LinhaCache[i] = malloc (assoc * sizeof(AA));

        for (int j = 0; j < assoc; j++) {

            // Inicializa cada bloco

            LinhaCache[i][j].BitValidade = 0;
            LinhaCache[i][j].Tag = 0;
            LinhaCache[i][j].Info = 0;

        }
    }

    for (int i = 0; i < nsets; i++) {

        free(LinhaCache[i]);

    }

    free(LinhaCache);

	return 0;



    
    int i = 0;
    int HIT = 0;
    int TotalMisses = 0;
    int Compulsorio = 0;
    int Conflito = 0;
    int Capacidade = 0;

    for (i = 0; i < assoc; i++) {

        if (LinhaCache[Indice][i].BitValidade == 1 && LinhaCache[Indice][i].Tag == Tag) {

            HIT = 1;
            break;

        } 
    }

    if (!HIT) {

        TotalMisses++;

        int TipoMISS = 0;

        TipoMISS = DefinirMisses (LinhaCache, Indice, assoc, nsets);

        if (TipoMISS == 0) {

            Compulsorio++;

        } else if (TipoMISS == 1) {

            Conflito++;

        } else {

            Capacidade++;

        }
    } 

    float TaxaMissCompulsorio = (float) Compulsorio / TotalMisses;
    float TaxaMissConflito = (float) Conflito / TotalMisses;
    float TaxaMissCapacidade = (float) Capacidade / TotalMisses;

}

int DefinirMisses (AA**LinhaCache, int Indice, int assoc, int nsets){

    // Miss Compulsório
        // Ocorre quando a linha (via) ainda nunca foi usada.

    // Miss de Capacidade
        // Ocorre quando a cache não tem espaço suficiente para manter 
        // todos os blocos usados, mesmo que a associatividade fosse infinita.

    // Miss de Conflito
        // Ocorre quando há espaço na cache, 
        // mas o bloco precisa cair em um conjunto fixo e esse conjunto está ocupado.

    int ViaVazia = 0;
    int i = 0;

    for (i = 0; i < assoc; i++) {

        if (LinhaCache[Indice][i].BitValidade == 0) {

            ViaVazia = 1;

        }
    }

    if (ViaVazia) {

        return 0;

    } else {

        if (assoc < nsets) {

            return 1;

        } else {

            return 2;

        }
    }
}


