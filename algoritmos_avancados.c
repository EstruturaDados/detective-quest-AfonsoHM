#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Detective Quest - Implementação em C
// Níveis:
//  - Novato: Árvore binária fixa representando salas da mansão
//  - Aventureiro: Árvore binária de busca (BST) para armazenar pistas
//  - Mestre: Tabela hash para associar pistas a suspeitos

// ------------------------- Helpers -------------------------
char *strdup_local(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

// ------------------------- Árvore de Salas (Mapa) -------------------------
typedef struct Sala {
    char nome[64];
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

Sala* criarSala(const char *nome) {
    Sala *s = malloc(sizeof(Sala));
    if (!s) return NULL;
    strncpy(s->nome, nome, sizeof(s->nome)-1);
    s->nome[sizeof(s->nome)-1] = '\0';
    s->esquerda = s->direita = NULL;
    return s;
}

void conectarSalas(Sala *pai, Sala *esq, Sala *dir) {
    if (!pai) return;
    pai->esquerda = esq;
    pai->direita = dir;
}

// ------------------------- BST de Pistas -------------------------
typedef struct Pista {
    char *texto;
    struct Pista *esq;
    struct Pista *dir;
} Pista;

Pista* criarPista(const char *texto) {
    Pista *p = malloc(sizeof(Pista));
    if (!p) return NULL;
    p->texto = strdup_local(texto);
    p->esq = p->dir = NULL;
    return p;
}

Pista* inserirPistaBST(Pista *raiz, const char *texto) {
    if (!raiz) return criarPista(texto);
    int cmp = strcmp(texto, raiz->texto);
    if (cmp < 0) raiz->esq = inserirPistaBST(raiz->esq, texto);
    else if (cmp > 0) raiz->dir = inserirPistaBST(raiz->dir, texto);
    // se igual, não insere duplicata
    return raiz;
}

void emOrdemPistas(Pista *raiz) {
    if (!raiz) return;
    emOrdemPistas(raiz->esq);
    printf("- %s\n", raiz->texto);
    emOrdemPistas(raiz->dir);
}

void liberarPistas(Pista *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz->texto);
    free(raiz);
}

// ------------------------- Tabela Hash para Suspeitos -------------------------
#define HASH_SIZE 27 // 26 letras + 1 para outros

typedef struct Suspeito {
    char *nome;
    char **pistas; // lista dinâmica de strings
    int contPistas;
    int capPistas;
    struct Suspeito *prox; // para tratar colisão
} Suspeito;

Suspeito *hashTable[HASH_SIZE];

unsigned int hashNome(const char *s) {
    if (!s || !s[0]) return HASH_SIZE-1;
    char c = s[0];
    if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    if (c >= 'a' && c <= 'z') return (c - 'a') % (HASH_SIZE-1);
    return HASH_SIZE-1; // bucket "outros"
}

Suspeito* buscarSuspeitoBucket(Suspeito *bucket, const char *nome) {
    Suspeito *p = bucket;
    while (p) {
        if (strcmp(p->nome, nome) == 0) return p;
        p = p->prox;
    }
    return NULL;
}

Suspeito* criarSuspeito(const char *nome) {
    Suspeito *s = malloc(sizeof(Suspeito));
    s->nome = strdup_local(nome);
    s->contPistas = 0;
    s->capPistas = 2;
    s->pistas = malloc(sizeof(char*) * s->capPistas);
    s->prox = NULL;
    return s;
}

void adicionarPistaASuspeito(Suspeito *s, const char *pista) {
    if (!s) return;
    if (s->contPistas >= s->capPistas) {
        s->capPistas *= 2;
        s->pistas = realloc(s->pistas, sizeof(char*) * s->capPistas);
    }
    s->pistas[s->contPistas++] = strdup_local(pista);
}

void inserirHash(const char *pista, const char *suspeitoNome) {
    unsigned int idx = hashNome(suspeitoNome);
    Suspeito *bucket = hashTable[idx];
    Suspeito *s = buscarSuspeitoBucket(bucket, suspeitoNome);
    if (!s) {
        // insere no início da lista do bucket
        s = criarSuspeito(suspeitoNome);
        s->prox = hashTable[idx];
        hashTable[idx] = s;
    }
    adicionarPistaASuspeito(s, pista);
}

void listarAssociacoes() {
    printf("\n--- Suspeitos e suas pistas ---\n");
    for (int i = 0; i < HASH_SIZE; ++i) {
        Suspeito *p = hashTable[i];
        while (p) {
            printf("%s (pistas: %d):\n", p->nome, p->contPistas);
            for (int j = 0; j < p->contPistas; ++j) printf("  - %s\n", p->pistas[j]);
            p = p->prox;
        }
    }
}

Suspeito* suspeitoMaisProvavel() {
    Suspeito *best = NULL;
    for (int i = 0; i < HASH_SIZE; ++i) {
        Suspeito *p = hashTable[i];
        while (p) {
            if (!best || p->contPistas > best->contPistas) best = p;
            p = p->prox;
        }
    }
    return best;
}

void liberarHash() {
    for (int i = 0; i < HASH_SIZE; ++i) {
        Suspeito *p = hashTable[i];
        while (p) {
            Suspeito *next = p->prox;
            free(p->nome);
            for (int j = 0; j < p->contPistas; ++j) free(p->pistas[j]);
            free(p->pistas);
            free(p);
            p = next;
        }
        hashTable[i] = NULL;
    }
}

// ------------------------- Sistema de Jogo -------------------------
Pista *raizPistas = NULL;

void associarPistaSuspeito_porSala(const char *salaNome) {
    // mapa simples de pistas automáticas por sala
    if (strcmp(salaNome, "Biblioteca") == 0) {
        raizPistas = inserirPistaBST(raizPistas, "Livros deslocados");
        inserirHash("Livros deslocados", "Joaquim");
    } else if (strcmp(salaNome, "Cozinha") == 0) {
        raizPistas = inserirPistaBST(raizPistas, "Pegadas úmidas na cozinha");
        inserirHash("Pegadas úmidas na cozinha", "Maria");
    } else if (strcmp(salaNome, "Sotao") == 0 || strcmp(salaNome, "Sótão") == 0) {
        raizPistas = inserirPistaBST(raizPistas, "Carta rasgada encontrada");
        inserirHash("Carta rasgada encontrada", "Carlos");
    } else if (strcmp(salaNome, "Hall de Entrada") == 0) {
        // pista menos conclusiva
        raizPistas = inserirPistaBST(raizPistas, "Pegadas na entrada");
        inserirHash("Pegadas na entrada", "Maria");
    }
}

void explorarSalas(Sala *atual) {
    if (!atual) return;
    Sala *pos = atual;
    char escolha[8];
    while (1) {
        printf("\nVocê está na sala: %s\n", pos->nome);
        // ao entrar, registrar pistas automáticas (apenas uma vez por visita neste run)
        associarPistaSuspeito_porSala(pos->nome);
        printf("Escolha: (e) esquerda, (d) direita, (s) sair exploração\n> ");
        if (!fgets(escolha, sizeof(escolha), stdin)) return;
        if (escolha[0] == 'e' && pos->esquerda) pos = pos->esquerda;
        else if (escolha[0] == 'd' && pos->direita) pos = pos->direita;
        else if (escolha[0] == 's') break;
        else printf("Movimento inválido ou caminho ausente.\n");
    }
}

void printarMapaRec(Sala *r, int depth) {
    if (!r) return;
    for (int i = 0; i < depth; ++i) printf("  ");
    printf("- %s\n", r->nome);
    printarMapaRec(r->esquerda, depth+1);
    printarMapaRec(r->direita, depth+1);
}

void menuPrincipal(Sala *root) {
    int rodando = 1;
    char buf[256];
    while (rodando) {
        printf("\n=== Detective Quest - Menu ===\n");
        printf("1) Explorar mansão\n");
        printf("2) Ver mapa da mansão (árvore de salas)\n");
        printf("3) Revisar pistas coletadas (BST em-ordem)\n");
        printf("4) Ver suspeitos e associações (hash)\n");
        printf("5) Adicionar pista manualmente e associar a suspeito\n");
        printf("6) Mostrar suspeito mais provável\n");
        printf("0) Sair e liberar memória\n");
        printf("> ");
        if (!fgets(buf, sizeof(buf), stdin)) break;
        int opc = atoi(buf);
        switch (opc) {
            case 1:
                explorarSalas(root);
                break;
            case 2:
                printf("\nMapa da Mansão:\n");
                printarMapaRec(root, 0);
                break;
            case 3:
                printf("\nPistas (em ordem alfabética):\n");
                emOrdemPistas(raizPistas);
                break;
            case 4:
                listarAssociacoes();
                break;
            case 5: {
                char pista[200];
                char suspeito[64];
                printf("Digite o texto da pista: ");
                if (!fgets(pista, sizeof(pista), stdin)) break;
                pista[strcspn(pista, "\n")] = '\0';
                printf("Digite o nome do suspeito a associar: ");
                if (!fgets(suspeito, sizeof(suspeito), stdin)) break;
                suspeito[strcspn(suspeito, "\n")] = '\0';
                if (strlen(pista) > 0) {
                    raizPistas = inserirPistaBST(raizPistas, pista);
                    inserirHash(pista, suspeito[0] ? suspeito : "Desconhecido");
                    printf("Pista adicionada e associada.\n");
                } else printf("Pista vazia não adicionada.\n");
                break;
            }
            case 6: {
                Suspeito *s = suspeitoMaisProvavel();
                if (s) {
                    printf("\nSuspeito mais provável: %s (pistas: %d)\n", s->nome, s->contPistas);
                } else printf("\nNenhum suspeito registrado ainda.\n");
                break;
            }
            case 0:
                rodando = 0;
                break;
            default:
                printf("Opção inválida.\n");
        }
    }
}

int main() {
    // construir mapa fixo da mansão
    Sala *hall = criarSala("Hall de Entrada");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *cozinha = criarSala("Cozinha");
    Sala *sotao = criarSala("Sotao"); // evita acento por compatibilidade
    Sala *escritorio = criarSala("Escritorio");
    Sala *jardim = criarSala("Jardim");

    conectarSalas(hall, biblioteca, cozinha);
    conectarSalas(biblioteca, sotao, escritorio);
    conectarSalas(cozinha, jardim, NULL);

    // inicializa tabela hash
    for (int i = 0; i < HASH_SIZE; ++i) hashTable[i] = NULL;

    printf("Bem-vindo ao Detective Quest!\n");
    printf("Explore a mansão, colete pistas e associe suspeitos.\n");

    menuPrincipal(hall);

    // Ao sair, mostrar resumo
    printf("\nResumo final:\n");
    printf("Pistas coletadas (alfabético):\n");
    emOrdemPistas(raizPistas);
    listarAssociacoes();
    Suspeito *top = suspeitoMaisProvavel();
    if (top) printf("\nSuspeito mais provável no final: %s (pistas: %d)\n", top->nome, top->contPistas);
    else printf("\nNenhum suspeito definido.\n");

    // liberar memória
    liberarPistas(raizPistas);
    liberarHash();

    // liberar salas (simplesmente free, sem gerenciamento complexo)
    free(hall); free(biblioteca); free(cozinha); free(sotao); free(escritorio); free(jardim);

    printf("Obrigado por jogar!\n");
    return 0;
}
