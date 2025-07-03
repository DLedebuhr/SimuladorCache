#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {

    int BitValidade;
    uint32_t Tag;

} LinhaCache;

int FIFO (int conjunto, int *ponteiroFIFO, int associatividade);

int RANDOM (int conjunto, int associatividade);

int LRU (int conjunto, int **lru, int associatividade);

void AtualizarLRU (int **lru, int conjunto, int viaAcesso, int associatividade);

void ApresentarDados (int FlagOut, int TotalMisses, int Acessos, int MissCompulsorio, int MissConflito, int MissCapacidade, int TotalHits, int nsets, int bsize, int assoc, char *subst, int flagOut, int TamanhoCache, int bitsIndice, int bitsOffset, int bitsTag);

void ApresentarDadosArquivoSaida (FILE * ArquivoSaida, int FlagOut, int TotalMisses, int Acessos, int MissCompulsorio, int MissConflito, int MissCapacidade, int TotalHits, int nsets, int bsize, int assoc, char *subst, int flagOut, int TamanhoCache, int bitsIndice, int bitsOffset, int bitsTag);

void ApresentarDadosProcessoArquivo (FILE *ArquivoSaida, int InicioFim, int Acessos, uint32_t Endereco, uint32_t Tag, uint32_t Indice, int HitMiss);

int main ( int argc, char *argv[ ] ) {
  
    if (argc != 7){
        
        printf("-- Numero de argumentos incorreto. Utilize:\n");

        printf("./cache_simulator <nsets> <bsize> <assoc> <substituição> <flag_saida> arquivo_de_entrada\n");

        exit(EXIT_FAILURE);

    }

    if (atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0 || atoi(argv[3]) <= 0 || (argv[4][0] != 'f' && argv[4][0] != 'l' && argv[4][0] != 'r') || (atoi(argv[5]) != 0 && atoi(argv[5]) != 1) || strstr(argv[6], ".bin") == NULL) {

        printf("-- Argumentos com valores invalidos.\n");
        exit(EXIT_FAILURE); 
        
    }

    int nsets = atoi(argv[1]);
    int bsize = atoi(argv[2]);
    int assoc = atoi(argv[3]);
    char *subst = argv[4];
    int flagOut = atoi(argv[5]);
    char *arquivoEntrada = argv[6];
    
    srand(time(NULL));

    char politicaSubst = *subst;

    LinhaCache **Cache = malloc(nsets * sizeof(LinhaCache *));

    if (Cache == NULL) {

        printf("-- Erro ao alocar memória para a cache.\n");
        exit(EXIT_FAILURE);

    }

    for (int i = 0; i < nsets; i++) {

        Cache[i] = malloc(assoc * sizeof(LinhaCache));

        if (Cache[i] == NULL) {

        printf("-- Erro ao alocar memória para a cache.\n");
        exit(EXIT_FAILURE);

        }

        for (int j = 0; j < assoc; j++) {

            Cache[i][j].BitValidade = 0;
            Cache[i][j].Tag = 0;

        }
    }

    int *ponteiroFIFO = calloc(nsets, sizeof(int));

    if (ponteiroFIFO == NULL) {

        printf("-- Erro ao alocar memória para a fila.\n");
        exit(EXIT_FAILURE);

    }

    int **lru = malloc(nsets * sizeof(int *));

    if (lru == NULL) {

        printf("-- Erro ao alocar memória para a LRU.\n");
        exit(EXIT_FAILURE);

    }

    for (int i = 0; i < nsets; i++) {

        lru[i] = calloc(assoc, sizeof(int));

        if (lru[i] == NULL) {

            printf("-- Erro ao alocar memória para a LRU.\n");
            exit(EXIT_FAILURE);

        }
    }

    FILE *Arquivo = fopen(arquivoEntrada, "rb");

    if (Arquivo == NULL) {

        printf("-- Erro ao abrir o arquivorr.\n");
        return 1;

    }

    char NomeArquivoSaida[100];

    time_t Complemento = time(NULL);

    struct tm *TempoReal = localtime(&Complemento);

    strftime(NomeArquivoSaida, sizeof(NomeArquivoSaida), "ArquivoSaida_%d%m%Y_%H%M%S.txt", TempoReal);

    FILE *ArquivoSaida = fopen(NomeArquivoSaida, "w"); 

    if (ArquivoSaida == NULL) {
        
        perror("-- Erro ao abrir o arquivoee");
        return 1;
        
    }

    int Acessos = 0;

    int TotalHits = 0;
    int TotalMisses = 0;

    int MissCompulsorio = 0;
    int MissConflito = 0;
    int MissCapacidade = 0;

    int QuantidadeBlocos = nsets * assoc;
    int TamanhoCache = QuantidadeBlocos * bsize;
    int BlocosVazios = QuantidadeBlocos;

    int bitsIndice = log2(nsets);
    int bitsOffset = log2(bsize);
    int bitsTag = 32 - bitsOffset - bitsIndice;

    uint8_t Bytes[4];
    uint32_t Endereco;

    for (int i = 0; i < 1000; i++) {

        fputc(' ', ArquivoSaida);

    }

    fprintf(ArquivoSaida, "\n");

    ApresentarDadosProcessoArquivo (ArquivoSaida, 1, Acessos, Endereco, 0, 0, 0);
  
    while (fread(Bytes, sizeof(uint8_t), 4, Arquivo) == 4) {

        Endereco = (Bytes[0] << 24) | (Bytes[1] << 16) | (Bytes[2] << 8) | Bytes[3];

        uint32_t Offset = Endereco & ((1 << bitsOffset) - 1);
        uint32_t Indice = (Endereco >> bitsOffset) & ((1 << bitsIndice) - 1);
        uint32_t Tag = Endereco >> (bitsOffset + bitsIndice);

        bool HIT = false;

        int Livre = -1;

        Acessos++;
        
        for (int i = 0; i < assoc; i++) {
            
            if (Cache[Indice][i].BitValidade && Cache[Indice][i].Tag == Tag) {
                
                TotalHits++;
                
                HIT = true;
                
                ApresentarDadosProcessoArquivo (ArquivoSaida, 0, Acessos, Endereco, Tag, Indice, 1);
                
                if (politicaSubst == 'l') {
                    
                    AtualizarLRU(lru, Indice, i, assoc);
                    
                }
                
                break;
                
            }
            
            if (Livre == -1 && Cache[Indice][i].BitValidade == 0) {
                
                    Livre = i;
                
            }
        }

        if (!HIT) {
            
            int viaSubstituir;

            if (Livre != -1) {
                
                MissCompulsorio++;
                viaSubstituir = Livre;
                BlocosVazios--;
                
            } else {
                
                if (BlocosVazios > 0) {
                    
                    MissConflito++;
                    
                } else {
                    
                    MissCapacidade++;
                    
                }

                if (assoc == 1) {
                    
                    viaSubstituir = 0;

                } else {

                    if (politicaSubst == 'f') {
                        
                        viaSubstituir = FIFO (Indice, ponteiroFIFO, assoc);

                    } else if (politicaSubst == 'l') {

                        viaSubstituir = LRU (Indice, lru, assoc);

                    } else if (politicaSubst == 'r') {

                        viaSubstituir = RANDOM (Indice, assoc);

                    } else {

                        fprintf(stderr, "politica invalida\n");
                        exit(EXIT_FAILURE);

                    }
                }
            }

            Cache[Indice][viaSubstituir].BitValidade = 1;
            Cache[Indice][viaSubstituir].Tag = Tag;

            ApresentarDadosProcessoArquivo (ArquivoSaida, 0, Acessos, Endereco, Tag, Indice, 2);

            if (politicaSubst == 'l') {
                
                AtualizarLRU(lru, Indice, viaSubstituir, assoc);
                
            }
        }
    }

    ApresentarDadosProcessoArquivo (ArquivoSaida, 2, Acessos, Endereco, 0, 0, 0);

    fseek(ArquivoSaida, 0, SEEK_SET);

    ApresentarDadosArquivoSaida(ArquivoSaida, flagOut, TotalMisses, Acessos, MissCompulsorio, MissConflito, MissCapacidade, TotalHits, nsets, bsize, assoc, subst, flagOut, TamanhoCache, bitsIndice, bitsOffset, bitsTag);
    
    fclose(Arquivo);
    fclose(ArquivoSaida);

    ApresentarDados(flagOut, TotalMisses, Acessos, MissCompulsorio, MissConflito, MissCapacidade, TotalHits, nsets, bsize, assoc, subst, flagOut, TamanhoCache, bitsIndice, bitsOffset, bitsTag);

    // Liberação de memória

    for (int i = 0; i < nsets; i++) {

        free(lru[i]);
        free(Cache[i]);

    }

    free(lru);
    free(Cache);
    free(ponteiroFIFO);

    return 0;

}

int FIFO (int conjunto, int *ponteiroFIFO, int associatividade) {
    
    if (conjunto < 0 || associatividade <= 0) {

        fprintf(stderr, "parametros invalidos");
        exit(EXIT_FAILURE);

    }

    int viaSubstituir = ponteiroFIFO[conjunto];

    ponteiroFIFO[conjunto] = (ponteiroFIFO[conjunto] + 1) % associatividade;

    return viaSubstituir;

}

int RANDOM (int conjunto, int associatividade) {

    if (conjunto < 0 || associatividade <= 0) {

        fprintf(stderr, "parametros invalidos");
        exit(EXIT_FAILURE);

    }

    return rand() % associatividade;

}

int LRU (int conjunto, int **lru, int associatividade) {

    if (conjunto < 0 || associatividade <= 0 || lru == NULL || lru[conjunto] == NULL) {

        fprintf(stderr, "parametros invalidos");
        exit(EXIT_FAILURE);

    }

    int viaSubstituir = 0;
    int maiorTempo = lru[conjunto][0];

    for (int v = 1; v < associatividade; v++) {

        if (lru[conjunto][v] > maiorTempo) {

            maiorTempo = lru[conjunto][v];

            viaSubstituir = v;

        }
    }

    return viaSubstituir;

}

void AtualizarLRU(int **lru, int conjunto, int viaAcesso, int associatividade) {

    if (conjunto < 0 || viaAcesso < 0 || viaAcesso >= associatividade ||
        associatividade <= 0 || lru == NULL || lru[conjunto] == NULL) {

        fprintf(stderr, "parametros invalidos");
        exit(EXIT_FAILURE);

    } //:)

    for (int i = 0; i < associatividade; i++) {

        if (i == viaAcesso) {

            lru[conjunto][i] = 0;

        } else {

            lru[conjunto][i]++;

        }
    }
}

void ApresentarDados(int FlagOut, int TotalMisses, int Acessos, int MissCompulsorio, int MissConflito, int MissCapacidade, int TotalHits, int nsets, int bsize, int assoc, char *subst, int flagOut, int TamanhoCache, int bitsIndice, int bitsOffset, int bitsTag) {

    float TaxaMissCompulsorio;
    float TaxaMissConflito;
    float TaxaMissCapacidade;
    float TaxaHit;
    float TaxaMiss;

    float PercentualCompulsorio;
    float PercentualConflito;
    float PercentualCapacidade;
    float PercentualHits;
    float PercentualMiss;

    TotalMisses = MissConflito + MissCompulsorio + MissCapacidade;

    if (TotalMisses == 0) {

        TaxaMissCompulsorio = 0;
        TaxaMissConflito = 0;
        TaxaMissCapacidade = 0;
        TaxaHit = (float)Acessos;
        TaxaMiss = 0;

    } else {

        TaxaMissCompulsorio = (float) MissCompulsorio / TotalMisses;
        PercentualCompulsorio = TaxaMissCompulsorio * 100;

        TaxaMissConflito = (float) MissConflito / TotalMisses;
        PercentualConflito = TaxaMissConflito * 100;

        TaxaMissCapacidade = (float) MissCapacidade / TotalMisses;
        PercentualCapacidade = TaxaMissCapacidade * 100;

        TaxaHit = (float) TotalHits / Acessos;
        PercentualHits = TaxaHit * 100;

        TaxaMiss = (float) TotalMisses / Acessos;
        PercentualMiss = TaxaMiss * 100;
    }

    if (FlagOut == 0) {

        printf("-- CARACTERISTICAS DA CACHE\n\n");

        printf("\t| Numero de Conjuntos:               %.3d\n", nsets);
        printf("\t| Tamanho do Bloco:                  %.3d\n", bsize);
        printf("\t| Associatividade:                   %.3d\n", assoc);
        printf("\t| Politica de Substituicao:        %c\n", *subst);
        printf("\t| Flag de Saida:                     %d\n\n", flagOut);

        printf("\t| Tamanho da Cache:                  %.3d bytes\n\n", TamanhoCache);

        printf("\t| Bits de Indice:                    %.3d bits\n", bitsIndice);
        printf("\t| Bits de Offset:                    %.3d bits\n", bitsOffset);
        printf("\t| Bits de Tag:                       %.3d bits\n\n", bitsTag);

        printf("--- RESULTADOS \n\n");

        printf("\t| Acessos:                           %.3d\n", Acessos);
        printf("\t| TotalHits:                         %.3d\n", TotalHits);
        printf("\t| TotalMisses:                       %.3d\n\n", TotalMisses);

        printf("\t| Taxa de Hit:                       %.4f  |  %.0f %%\n", TaxaHit, PercentualHits);
        printf("\t| Taxa de Miss:                      %.4f  |  %.0f %%\n", TaxaMiss, PercentualMiss);
        printf("\t| Taxa de Miss Compulsorio:          %.4f  |  %.0f %%\n", TaxaMissCompulsorio,
            PercentualCompulsorio);
        printf("\t| Taxa de Miss Capacidade:           %.4f  |  %.0f %%\n", TaxaMissCapacidade,
            PercentualCapacidade);
        printf("\t| Taxa de Miss Conflito:             %.4f  |  %.f %%\n", TaxaMissConflito,
            PercentualConflito);

    } else if (FlagOut == 1) {

        printf("%d %.4f %.4f %.4f %.4f %.4f\n", Acessos, TaxaHit, TaxaMiss, TaxaMissCompulsorio, TaxaMissCapacidade, TaxaMissConflito);

    }
}                           

void ApresentarDadosProcessoArquivo (FILE * ArquivoSaida, int InicioFim, int Acessos, uint32_t Endereco, uint32_t Tag, uint32_t Indice, int HitMiss) {

    if (InicioFim == 1) {
        
        fprintf(ArquivoSaida, "\n-- PROCESSAMENTO DOS ENDERECOS\n\n");
        
        fprintf(ArquivoSaida, "|--------------|--------------|--------------|--------------|-------------|\n");
        fprintf(ArquivoSaida, "|    ACESSO    |   HIT/MISS   |   ENDEREÇO   |    INDICE    |     TAG     |\n");
        fprintf(ArquivoSaida, "|--------------|--------------|--------------|--------------|-------------|\n");
        
    } else if (InicioFim == 2) {

        fprintf(ArquivoSaida, "|--------------|--------------|--------------|--------------|-------------|\n");
        
    }

    if (HitMiss == 1) {

        fprintf(ArquivoSaida, "|    %.6d", Acessos);
        fprintf(ArquivoSaida, "    |     Hit");
        fprintf(ArquivoSaida, "      |    %.6d", Endereco);
        fprintf(ArquivoSaida, "    |     %.4d", Tag);
        fprintf(ArquivoSaida, "     |     %.3d     |\n", Indice);
        
    } else if (HitMiss == 2) {

        fprintf(ArquivoSaida, "|    %.6d", Acessos);
        fprintf(ArquivoSaida, "    |     Miss");
        fprintf(ArquivoSaida, "     |    %.6d", Endereco);
        fprintf(ArquivoSaida, "    |     %.4d", Tag);
        fprintf(ArquivoSaida, "     |     %.3d     |\n", Indice);
        
    }
}

void ApresentarDadosArquivoSaida (FILE * ArquivoSaida, int FlagOut, int TotalMisses, int Acessos, int MissCompulsorio, int MissConflito, int MissCapacidade, int TotalHits, int nsets, int bsize, int assoc, char *subst, int flagOut, int TamanhoCache, int bitsIndice, int bitsOffset, int bitsTag) {

    float TaxaMissCompulsorio;
    float TaxaMissConflito;
    float TaxaMissCapacidade;
    float TaxaHit;
    float TaxaMiss;

    float PercentualCompulsorio;
    float PercentualConflito;
    float PercentualCapacidade;
    float PercentualHits;
    float PercentualMiss;

    TotalMisses = MissConflito + MissCompulsorio + MissCapacidade;

    if (TotalMisses == 0) {

        TaxaMissCompulsorio = 0;
        TaxaMissConflito = 0;
        TaxaMissCapacidade = 0;
        TaxaHit = (float) Acessos;
        TaxaMiss = 0;

    } else {

        TaxaMissCompulsorio = (float) MissCompulsorio / TotalMisses;
        PercentualCompulsorio = TaxaMissCompulsorio * 100;

        TaxaMissConflito = (float) MissConflito / TotalMisses;
        PercentualConflito = TaxaMissConflito * 100;

        TaxaMissCapacidade = (float) MissCapacidade / TotalMisses;
        PercentualCapacidade = TaxaMissCapacidade * 100;

        TaxaHit = (float) TotalHits / Acessos;
        PercentualHits = TaxaHit * 100;

        TaxaMiss = (float) TotalMisses / Acessos;
        PercentualMiss = TaxaMiss * 100;
    }

    fprintf(ArquivoSaida, "|-------------------------------------------------------------------------|\n");
    
    fprintf(ArquivoSaida, "\n--- CARACTERISTICAS DA CACHE\n\n");

    fprintf(ArquivoSaida, "\t| Numero de Conjuntos:               %.3d\n", nsets);
    fprintf(ArquivoSaida, "\t| Tamanho do Bloco:                  %.3d\n", bsize);
    fprintf(ArquivoSaida, "\t| Associatividade:                   %.3d\n", assoc);
    fprintf(ArquivoSaida, "\t| Politica de Substituicao:        %c\n", *subst);
    fprintf(ArquivoSaida, "\t| Flag de Saida:                     %d\n\n", flagOut);

    fprintf(ArquivoSaida, "\t| Tamanho da Cache:                  %.3d bytes\n\n", TamanhoCache);

    fprintf(ArquivoSaida, "\t| Bits de Indice:                    %.3d bits\n", bitsIndice);
    fprintf(ArquivoSaida, "\t| Bits de Offset:                    %.3d bits\n", bitsOffset);
    fprintf(ArquivoSaida, "\t| Bits de Tag:                       %.3d bits\n\n", bitsTag);

    fprintf(ArquivoSaida, "--- RESULTADOS \n\n");

    fprintf(ArquivoSaida, "\t| Acessos:                           %.3d\n", Acessos);
    fprintf(ArquivoSaida, "\t| TotalHits:                         %.3d\n", TotalHits);
    fprintf(ArquivoSaida, "\t| TotalMisses:                       %.3d\n\n", TotalMisses);

    fprintf(ArquivoSaida, "\t| Taxa de Hit:                       %.4f  |  %.0f %% \n", TaxaHit, PercentualHits);
    fprintf(ArquivoSaida, "\t| Taxa de Miss:                      %.4f  |  %.0f %% \n", TaxaMiss, PercentualMiss);
    fprintf(ArquivoSaida, "\t| Taxa de Miss Compulsorio:          %.4f  |  %.0f %% \n", TaxaMissCompulsorio, PercentualCompulsorio);
    fprintf(ArquivoSaida, "\t| Taxa de Miss Capacidade:           %.4f  |  %.0f %% \n", TaxaMissCapacidade, PercentualCapacidade);
    fprintf(ArquivoSaida, "\t| Taxa de Miss Conflito:             %.4f  |  %.0f %% \n", TaxaMissConflito, PercentualConflito);

    fprintf(ArquivoSaida, "\n|-------------------------------------------------------------------------|\n");

}