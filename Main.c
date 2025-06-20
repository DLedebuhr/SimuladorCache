#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {

  int BitValidade;
  uint32_t Tag;

} Cache;

int DefinirMisses(Cache **LinhaCache, int Indice, int assoc, int nsets);

int FIFO(Cache **LinhaCache, int conjunto, int *pFila, int via);

int LRU(Cache **LinhaCache, int conjunto, int **lru, int via);

int Random(Cache **LinhaCache, int conjunto, int *pFila, int via);

void atualizarLRU(int **lru, int conjunto, int viaAcesso, int via);

//int main(int argc, char *argv[]) {
int main(){
  // argc (argument count) → número de argumentos passados pela linha de
  // comando.

  // argv[] (argument vector) → vetor de strings que contém os argumentos.

  //if (argc != 7) {

    //printf("Numero de argumentos incorreto. Utilize:\n");

    //printf("./cache_simulator <nsets> <bsize> <assoc> <substituição> "
      //     "<flag_saida> arquivo_de_entrada\n");

    //return 1;
  //}

  //int nsets = atoi(argv[1]);
  //int bsize = atoi(argv[2]);
  //int assoc = atoi(argv[3]);
  //int flagOut = atoi(argv[5]);

  //char *subst = argv[4];
  //char *arquivoEntrada = argv[6];

  int nsets = 256;
  int bsize = 4;
  int assoc = 1;
  int flagOut = 0;

  char *subst = "r";
  char *arquivoEntrada = "bin_100.bin";

  int bitsIndice = log2(nsets);

  int bitsOffset = log2(bsize);

  int bitsTag = 32 - bitsOffset - bitsIndice;

  int tamanhoCache = nsets * bsize * assoc;

  Cache **LinhaCache = malloc(nsets * sizeof(Cache));

  if (LinhaCache == NULL) {

    printf("Erro ao alocar memória para a cache.\n");

    return 1;
  }

  for (int i = 0; i < nsets; i++) {

    LinhaCache[i] = malloc(assoc * sizeof(Cache *));

    if (LinhaCache[i] == NULL) {

      printf("Erro ao alocar memória para a cache.\n");

      return 1;
    }

    for (int j = 0; j < assoc; j++) {

      LinhaCache[i][j].BitValidade = 0;
      LinhaCache[i][j].Tag = 0;
    }
  }

  FILE *Arquivo = fopen(arquivoEntrada, "rb");

  if (Arquivo == NULL) {

    printf("Erro ao abrir o arquivo.\n");
    return 1;
  }

  uint32_t Endereco;

  int Acessos = 0;
  int HIT = 0;
  int TotalHit = 0;
  int TotalMisses = 0;
  int Compulsorio = 0;
  int Conflito = 0;
  int Capacidade = 0;

  int *pFila = calloc(nsets, sizeof(int));

  int **lru = malloc(nsets * sizeof(int *));

  for (int i = 0; i < nsets; i++) {

    lru[i] = calloc(assoc, sizeof(int));
  }

  while (fread(&Endereco, sizeof(uint32_t), 1, Arquivo) == 1) {

    Acessos++;

    // ( o`-´o ) ~

    // << deslocamento para esquerda

    uint32_t offset = Endereco & ((1 << bitsOffset) - 1);
    // offset = Endereco % bsize;

    //   10110110   (Endereco)
    // & 00000111   (máscara)
    // = 00000110   (0x06, decimal 6)

    uint32_t indice = (Endereco >> bitsOffset) & ((1 << bitsIndice) - 1);
    // indice = (Endereco / bsize) % nsets;

    // Endereco >> bitsOffset = desloca o endereço pra direita, tirando os bits
    // do offset.

    //    10110110   (Endereco)
    // >> 00010110   (Deslocamento de 3 bits) ex

    // ((1 << bitsIndice) - 1) = máscara para pegar só os bits do índice.

    //    00010110   (Endereço restante)
    //  & 00001111   (máscara)
    //  = 00000110   (0x06, decimal 6)

    uint32_t tag = Endereco >> (bitsOffset + bitsIndice);
    // tag = Endereco / (bsize * nsets);

    // Desloca o endereço para a direita, tirando os bits de índice + offset.

    //    10110110
    // >> 00000010

    for (int i = 0; i < assoc; i++) {

      if (LinhaCache[indice][i].BitValidade == 1 &&
          LinhaCache[indice][i].Tag == tag) {

        HIT = 1;
        TotalHit++;
        break;
      }
    }

    if (!HIT) {

      TotalMisses++;

      int TipoMISS = 0;

      TipoMISS = DefinirMisses(LinhaCache, indice, assoc, nsets);

      if (TipoMISS == 0) {

        Compulsorio++;

      } else if (TipoMISS == 1) {

        Conflito++;

      } else {

        Capacidade++;
      }

      int viaSubstituir = 0;

      // FIFO
      if (subst[0] == 'f') {

        viaSubstituir = FIFO(LinhaCache, indice, pFila, assoc);
      } else if (subst[0] == 'l') {

        viaSubstituir = LRU(LinhaCache, indice, lru, assoc);
      } else if (subst[0] == 'r') {

        viaSubstituir = Random(LinhaCache, indice, pFila, assoc);
      } else {
        printf("Substituição inválida\n");
        exit(1);
      }

      LinhaCache[indice][viaSubstituir].Tag = tag;
      LinhaCache[indice][viaSubstituir].BitValidade = 1;

      if (subst[0] == 'l') {
        atualizarLRU(lru, indice, viaSubstituir, assoc);
      }
    }
  }

  fclose(Arquivo);

  float TaxaMissCompulsorio = (float)Compulsorio / TotalMisses;
  float TaxaMissConflito = (float)Conflito / TotalMisses;
  float TaxaMissCapacidade = (float)Capacidade / TotalMisses;

  float TaxaHit = (float)HIT / Acessos;
  float TaxaMiss = (float)TotalMisses / Acessos;

  if (flagOut == 0) {

    printf("-- Informações da Cache:\n");

    // printf("\tNome da Cache: %d\n", nsets);
    printf("\tNumero de Conjuntos: %d bytes\n", nsets);
    printf("\tTamanho do Bloco: %d bytes\n", bsize);
    printf("\tAssociatividade: %d\n", assoc);
    printf("\tSubstituição: %c\n", *subst);
    printf("\tFlag de Saida: %d\n", flagOut);
    // printf("\tArquivo de Entrada: %d bytes\n", tamanhoCache);

    printf("\tTamanho da Cache: %d bytes\n", tamanhoCache);
    printf("\tBits de Indice: %d\n", bitsIndice);
    printf("\tBits de Offset: %d\n", bitsOffset);
    printf("\tBits de Tag: %d\n", bitsTag);

    printf("\n");

    printf("-- Informações do Acesso:\n");

    printf("\tAcessos: %d\n", Acessos);
    printf("\tTotalHits: %d\n", TotalHit);
    printf("\tTotalMisses: %d\n", TotalMisses);

    printf("Taxa de Hit: %.4f\n", TaxaHit);
    printf("Taxa de Miss: %.4f\n", TaxaMiss);

    printf("Taxa de Miss Compulsorio: %.4f\n", TaxaMissCompulsorio);
    printf("Taxa de Miss Capacidade: %.4f\n", TaxaMissCapacidade);
    printf("Taxa de Miss Conflito: %.4f\n", TaxaMissConflito);

  } else if (flagOut == 1) {

    printf("%d %.4f %.4f %.4f %.4f %.4f\n", Acessos, TaxaHit, TaxaMiss,
           TaxaMissCompulsorio, TaxaMissCapacidade, TaxaMissConflito);
  }

  free(pFila);
  for (int i = 0; i < nsets; i++) {
    free(LinhaCache[i]);
    free(lru[i]);
  }
  free(LinhaCache);
  free(lru);
  // FIM

  // /ᐠ｡ꞈ｡ᐟ\ // // >////< //
}

int FIFO(Cache **LinhaCache, int conjunto, int *pFila, int via) {
  int viaSubstituir = pFila[conjunto];

  pFila[conjunto] = (pFila[conjunto] + 1) % via;
  return viaSubstituir;
}

int LRU(Cache **LinhaCache, int conjunto, int **lru, int via) {
  int viaSubstituir = 0;
  int maiorTempo = -1;

  for (int v = 0; v < via; v++) {
    if (lru[conjunto][v] > maiorTempo) {
      maiorTempo = lru[conjunto][v];
      viaSubstituir = v;
    }
  }

  return viaSubstituir;
}

void atualizarLRU(int **lru, int conjunto, int viaAcesso, int via) {
  for (int i = 0; i < via; i++) {
    if (i == viaAcesso) {
      lru[conjunto][i] = 0; // mais recente usado
    } else if (lru[conjunto][i] < via - 1) {
      lru[conjunto]
         [i]++; // adiciona valor oque tiver maior valor vai ser substituido
    }
  }
}

int Random(Cache **LinhaCache, int conjunto, int *pFila, int via) {
  return rand() % via;
}

int DefinirMisses(Cache **LinhaCache, int Indice, int assoc, int nsets) {

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