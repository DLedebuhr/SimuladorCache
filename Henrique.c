#include <stdio.h>

typedef struct
{

    int Bit_Validade;
    int Tag;
    int Informacao;

} Cache;

int FIFAR(Cache **LinhaCache, int conjunto, int *pFila, int via)
{
    int viaSubstituir = pFila[conjunto];

    pFila[conjunto] = (pFila[conjunto] + 1) % via;
    return viaSubstituir;
}

int LRUAR(Cache **LinhaCache, int conjunto, int **lru, int via)
{
    int viaSubstituir = 0;
    int maiorTempo = -1;

    for (int v = 0; v < via; v++)
    {
        if (lru[conjunto][v] > maiorTempo)
        {
            maiorTempo = lru[conjunto][v];
            viaSubstituir = v;
        }
    }

    return viaSubstituir;
}

void AtualizarLRU(int **lru, int conjunto, int viaAcesso, int via)
{
    for (int i = 0; i < via; i++)
    {
        if (i == viaAcesso)
        {
            lru[conjunto][i] = 0; // mais recente usado
        }
        else if (lru[conjunto][i] < via - 1)
        {
            lru[conjunto][i]++; // adiciona valor oque tiver maior valor vai ser substituido
        }
    }
}

int Randomizar(Cache **LinhaCache, int conjunto, int *pFila, int via) {
    return rand() % via;
}


int main()
{
}