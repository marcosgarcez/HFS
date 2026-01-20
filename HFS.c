#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define TAM_NOME 32
#define TAM_BLOCO 512
#define NUM_BLOCOS 1024
#define MAX_QS 100
#define MAX_EXT 3
#define MAX_TAGS 5
#define TAM_TAG 20

typedef struct { int bl_ini, n_bl; } Extent;

typedef struct {
    char nome[TAM_NOME];
    int tot_bl, liv_bl, tam_bl, prox_id;
    time_t criacao, modif;
} MDB;

typedef struct {
    int id, id_pai, eh_dir, tam, qtd_ext;
    char nome[TAM_NOME], cont[TAM_BLOCO*4];
    time_t criacao, modif;
    Extent ext[MAX_EXT];
    // ===== NOVOS CAMPOS PARA BUSCA INTELIGENTE =====
    char tags[MAX_TAGS][TAM_TAG];
    int num_tags;
} Reg;

typedef struct { MDB mdb; Reg cat[MAX_QS]; int qtd; int map[NUM_BLOCOS]; } SysHFS;
SysHFS fs;

char* f_data(time_t t) {
    static char b[20]; strftime(b, 20, "%d/%m/%Y %H:%M", localtime(&t)); return b;
}

// Função auxiliar para converter string para minúsculas
void str_tolower(char *dest, const char *src) {
    int i;
    for (i = 0; src[i]; i++) {
        dest[i] = tolower(src[i]);
    }
    dest[i] = '\0';
}

// Função auxiliar para verificar se uma string contém outra (case-insensitive)
int str_contains(const char *haystack, const char *needle) {
    char hay_lower[TAM_BLOCO*4], needle_lower[TAM_TAG];
    str_tolower(hay_lower, haystack);
    str_tolower(needle_lower, needle);
    return strstr(hay_lower, needle_lower) != NULL;
}

// Função auxiliar para obter o caminho completo de um arquivo
void obter_caminho(int idx, char *caminho) {
    if (idx < 0 || idx >= fs.qtd) {
        strcpy(caminho, "");
        return;
    }
    
    char temp[256] = "";
    int id_atual = fs.cat[idx].id;
    
    // Constrói o caminho de trás para frente
    while (id_atual != 1) {  // Enquanto não chegar na raiz
        int encontrado = 0;
        for (int i = 0; i < fs.qtd; i++) {
            if (fs.cat[i].id == id_atual) {
                char parte[TAM_NOME + 2];
                sprintf(parte, "/%s", fs.cat[i].nome);
                strcat(parte, temp);
                strcpy(temp, parte);
                id_atual = fs.cat[i].id_pai;
                encontrado = 1;
                break;
            }
        }
        if (!encontrado) break;
    }
    
    if (strlen(temp) == 0) {
        strcpy(caminho, "/");
    } else {
        strcpy(caminho, temp);
    }
}

int aloca(int qtd) {
    for (int i=0, j; i <= NUM_BLOCOS-qtd; i++) {
        int livre = 1;
        for (j=0; j<qtd; j++) if(fs.map[i+j]) { livre=0; break; }
        if (livre) {
            for (j=0; j<qtd; j++) fs.map[i+j] = 1;
            fs.mdb.liv_bl -= qtd; return i;
        }
    }
    return -1;
}

void libera(int ini, int qtd) {
    for (int i=ini; i<ini+qtd; i++) fs.map[i] = 0;
    fs.mdb.liv_bl += qtd;
}

int busca(char *nome, int pai) {
    for (int i=0; i<fs.qtd; i++) 
        if (fs.cat[i].id_pai == pai && strcmp(fs.cat[i].nome, nome) == 0) return i;
    return -1;
}

void init_hfs(char *nome) {
    memset(&fs, 0, sizeof(SysHFS));
    strcpy(fs.mdb.nome, nome);
    fs.mdb.tot_bl = fs.mdb.liv_bl = NUM_BLOCOS;
    fs.mdb.tam_bl = TAM_BLOCO;
    fs.mdb.criacao = fs.mdb.modif = time(NULL);
    fs.mdb.prox_id = 2;
    
    Reg *r = &fs.cat[fs.qtd++];
    r->id = 1;
    r->id_pai = 0;
    strcpy(r->nome, "/"); 
    r->eh_dir = 1;
    r->criacao = r->modif = time(NULL);
    r->num_tags = 0;
    printf("\n[SUCESSO] HFS Inicializado: Vol=%s, Blocos=%d, Tam=%d\n", nome, NUM_BLOCOS, TAM_BLOCO);
}

void cria_dir(char *nome, int pai) {
    int pai_existe = 0;
    for (int i=0; i<fs.qtd; i++) {
        if (fs.cat[i].id == pai && fs.cat[i].eh_dir) {
            pai_existe = 1;
            break;
        }
    }
    if (!pai_existe && pai != 0) {
        printf("\n[ERRO] Diretorio pai nao existe\n");
        return;
    }
    if (fs.qtd >= MAX_QS || busca(nome, pai) != -1) { 
        printf("\n[ERRO] Diretorio nao pode ser criado (Sistema cheio ou ja existe)\n"); 
        return; 
    }
    Reg *d = &fs.cat[fs.qtd++];
    d->id = fs.mdb.prox_id++; d->id_pai = pai; d->eh_dir = 1;
    strcpy(d->nome, nome); 
    d->criacao = d->modif = time(NULL);
    d->num_tags = 0;
    printf("\n[SUCESSO] Diretorio criado: %s (ID %d)\n", nome, d->id);
}

void cria_arq(char *nome, int pai, char *dados) {
    if (strlen(dados) >= TAM_BLOCO*4) {
        printf("\n[ERRO] Conteudo muito grande (max %d bytes)\n", TAM_BLOCO*4-1);
        return;
    }
    if (fs.qtd >= MAX_QS || busca(nome, pai) != -1) { 
        printf("\n[ERRO] Arquivo nao pode ser criado (Sistema cheio ou ja existe)\n"); 
        return; 
    }
    int blocos = (strlen(dados) + TAM_BLOCO - 1) / TAM_BLOCO;
    int ini = aloca(blocos);
    if (ini == -1) { 
        printf("\n[ERRO] Disco cheio - sem espaco para criar arquivo\n"); 
        return; 
    }

    Reg *a = &fs.cat[fs.qtd++];
    a->id = fs.mdb.prox_id++; a->id_pai = pai; a->tam = strlen(dados);
    strcpy(a->nome, nome); strcpy(a->cont, dados);
    a->criacao = a->modif = time(NULL);
    a->ext[0] = (Extent){ini, blocos}; a->qtd_ext = 1;
    a->num_tags = 0;
    printf("\n[SUCESSO] Arquivo criado: %s (ID %d, %d blocos)\n", nome, a->id, blocos);
}

void le_arq(char *nome, int pai) {
    int i = busca(nome, pai);
    if (i == -1 || fs.cat[i].eh_dir) { 
        printf("\n[ERRO] Arquivo nao encontrado ou e um diretorio\n"); 
        return; 
    }
    printf("\n========== %s (%d bytes) ==========\n", fs.cat[i].nome, fs.cat[i].tam);
    printf("%s\n", fs.cat[i].cont);
    
    // Mostra tags se houver
    if (fs.cat[i].num_tags > 0) {
        printf("\nTags: ");
        for (int j = 0; j < fs.cat[i].num_tags; j++) {
            printf("[%s]", fs.cat[i].tags[j]);
            if (j < fs.cat[i].num_tags - 1) printf(" ");
        }
        printf("\n");
    }
    printf("=====================================\n");
}

void escreve_arq(char *nome, int pai, char *dados) {
    int i = busca(nome, pai);
    if (i == -1 || fs.cat[i].eh_dir) { 
        printf("\n[ERRO] Arquivo nao encontrado ou e um diretorio\n"); 
        return; 
    }
    
    for (int k=0; k<fs.cat[i].qtd_ext; k++) 
        libera(fs.cat[i].ext[k].bl_ini, fs.cat[i].ext[k].n_bl);
    
    int blocos = (strlen(dados) + TAM_BLOCO - 1) / TAM_BLOCO;
    int ini = aloca(blocos);
    if (ini == -1) { 
        printf("\n[ERRO] Sem espaco para atualizar arquivo\n"); 
        return; 
    }

    fs.cat[i].ext[0] = (Extent){ini, blocos}; fs.cat[i].qtd_ext = 1;
    fs.cat[i].tam = strlen(dados); strcpy(fs.cat[i].cont, dados);
    fs.cat[i].modif = time(NULL);
    printf("\n[SUCESSO] Arquivo atualizado: %s\n", nome);
}

void del_arq(char *nome, int pai) {
    int i = busca(nome, pai);
    if (i == -1 || fs.cat[i].eh_dir) { 
        printf("\n[ERRO] Arquivo nao encontrado ou e um diretorio\n"); 
        return; 
    }
    for (int k=0; k<fs.cat[i].qtd_ext; k++) 
        libera(fs.cat[i].ext[k].bl_ini, fs.cat[i].ext[k].n_bl);
    
    for (; i < fs.qtd-1; i++) fs.cat[i] = fs.cat[i+1];
    fs.qtd--;
    printf("\n[SUCESSO] Arquivo deletado: %s\n", nome);
}

void del_dir(char *nome, int pai) {
    int i = busca(nome, pai);
    if (i == -1 || !fs.cat[i].eh_dir) { 
        printf("\n[ERRO] Diretorio nao encontrado ou e um arquivo\n"); 
        return; 
    }
    
    // Verifica se diretório está vazio
    for (int j=0; j<fs.qtd; j++) {
        if (fs.cat[j].id_pai == fs.cat[i].id) {
            printf("\n[ERRO] Diretorio nao esta vazio\n");
            return;
        }
    }
    
    for (; i < fs.qtd-1; i++) fs.cat[i] = fs.cat[i+1];
    fs.qtd--;
    printf("\n[SUCESSO] Diretorio deletado: %s\n", nome);
}

void listar(int pai) {
    printf("\n========== LISTAGEM (Diretorio ID: %d) ==========\n", pai);
    printf("%-6s %-6s %-6s %-25s %-10s\n", "ID", "PAI", "TIPO", "NOME", "TAMANHO");
    printf("------------------------------------------------------------------\n");
    
    int encontrou = 0;
    for (int i=0; i<fs.qtd; i++) {
        if (fs.cat[i].id_pai == pai) {
            printf("%-6d %-6d %-6s %-25s %-10d", 
                   fs.cat[i].id, 
                   fs.cat[i].id_pai,
                   fs.cat[i].eh_dir?"DIR":"ARQ", 
                   fs.cat[i].nome, 
                   fs.cat[i].tam);
            
            // Mostra tags se houver
            if (fs.cat[i].num_tags > 0) {
                printf(" [");
                for (int j = 0; j < fs.cat[i].num_tags; j++) {
                    printf("%s", fs.cat[i].tags[j]);
                    if (j < fs.cat[i].num_tags - 1) printf(",");
                }
                printf("]");
            }
            printf("\n");
            encontrou = 1;
        }
    }
    
    if (!encontrou) {
        printf("(vazio)\n");
    }
    printf("==================================================================\n");
}

void info() {
    printf("\n========== INFORMACOES DO VOLUME ==========\n");
    printf("Nome do Volume: %s\n", fs.mdb.nome);
    printf("Criado em: %s\n", f_data(fs.mdb.criacao));
    printf("Modificado em: %s\n", f_data(fs.mdb.modif));
    printf("Blocos Livres: %d/%d\n", fs.mdb.liv_bl, NUM_BLOCOS);
    printf("Uso do Disco: %.1f%%\n", 100.0*(NUM_BLOCOS-fs.mdb.liv_bl)/NUM_BLOCOS);
    printf("Tamanho do Bloco: %d bytes\n", TAM_BLOCO);
    printf("Arquivos/Diretorios: %d/%d\n", fs.qtd-1, MAX_QS);
    printf("===========================================\n");
}

void exibir_arvore_rec(int pai, int nivel) {
    for (int i=0; i<fs.qtd; i++) {
        if (fs.cat[i].id_pai == pai) {
            for (int j=0; j<nivel; j++) printf("  ");
            printf("|-- %s %s (ID:%d PAI:%d)", fs.cat[i].nome, fs.cat[i].eh_dir?"(DIR)":"", fs.cat[i].id, fs.cat[i].id_pai);
            
            // Mostra tags
            if (fs.cat[i].num_tags > 0) {
                printf(" [");
                for (int j = 0; j < fs.cat[i].num_tags; j++) {
                    printf("%s", fs.cat[i].tags[j]);
                    if (j < fs.cat[i].num_tags - 1) printf(",");
                }
                printf("]");
            }
            printf("\n");
            
            if (fs.cat[i].eh_dir) {
                exibir_arvore_rec(fs.cat[i].id, nivel+1);
            }
        }
    }
}

void exibir_arvore() {
    printf("\n========== ARVORE DE DIRETORIOS ==========\n");
    printf("/ (raiz) (ID:1)\n");
    exibir_arvore_rec(1, 1);
    printf("==========================================\n");
}

// ========== FUNÇÕES DE BUSCA INTELIGENTE ==========

void adicionar_tag(char *nome_arq, int pai, char *tag) {
    int i = busca(nome_arq, pai);
    if (i == -1) {
        printf("\n[ERRO] Arquivo/Diretorio nao encontrado\n");
        return;
    }
    
    if (fs.cat[i].num_tags >= MAX_TAGS) {
        printf("\n[ERRO] Numero maximo de tags atingido (%d)\n", MAX_TAGS);
        return;
    }
    
    // Verifica se a tag já existe
    for (int j = 0; j < fs.cat[i].num_tags; j++) {
        if (strcmp(fs.cat[i].tags[j], tag) == 0) {
            printf("\n[AVISO] Tag '%s' ja existe neste item\n", tag);
            return;
        }
    }
    
    strcpy(fs.cat[i].tags[fs.cat[i].num_tags], tag);
    fs.cat[i].num_tags++;
    printf("\n[SUCESSO] Tag '%s' adicionada a '%s'\n", tag, nome_arq);
}

void remover_tag(char *nome_arq, int pai, char *tag) {
    int i = busca(nome_arq, pai);
    if (i == -1) {
        printf("\n[ERRO] Arquivo/Diretorio nao encontrado\n");
        return;
    }
    
    int encontrou = -1;
    for (int j = 0; j < fs.cat[i].num_tags; j++) {
        if (strcmp(fs.cat[i].tags[j], tag) == 0) {
            encontrou = j;
            break;
        }
    }
    
    if (encontrou == -1) {
        printf("\n[ERRO] Tag '%s' nao encontrada\n", tag);
        return;
    }
    
    // Remove a tag deslocando as outras
    for (int j = encontrou; j < fs.cat[i].num_tags - 1; j++) {
        strcpy(fs.cat[i].tags[j], fs.cat[i].tags[j+1]);
    }
    fs.cat[i].num_tags--;
    
    printf("\n[SUCESSO] Tag '%s' removida de '%s'\n", tag, nome_arq);
}

void buscar_por_tag(char *tag) {
    printf("\n========== BUSCA POR TAG: '%s' ==========\n", tag);
    printf("%-6s %-6s %-30s %-50s\n", "ID", "TIPO", "NOME", "CAMINHO");
    printf("-----------------------------------------------------------------------------------\n");
    
    int encontrou = 0;
    for (int i = 0; i < fs.qtd; i++) {
        for (int j = 0; j < fs.cat[i].num_tags; j++) {
            if (str_contains(fs.cat[i].tags[j], tag)) {
                char caminho[256];
                obter_caminho(i, caminho);
                printf("%-6d %-6s %-30s %-50s\n", 
                       fs.cat[i].id,
                       fs.cat[i].eh_dir ? "DIR" : "ARQ",
                       fs.cat[i].nome,
                       caminho);
                encontrou = 1;
                break;
            }
        }
    }
    
    if (!encontrou) {
        printf("Nenhum item encontrado com esta tag.\n");
    }
    printf("===================================================================================\n");
}

void busca_fuzzy(char *termo) {
    printf("\n========== BUSCA INTELIGENTE: '%s' ==========\n", termo);
    printf("%-6s %-10s %-6s %-25s %-40s\n", "ID", "TIPO BUSCA", "TIPO", "NOME", "CAMINHO");
    printf("--------------------------------------------------------------------------------------------\n");
    
    int encontrou = 0;
    
    for (int i = 0; i < fs.qtd; i++) {
        int ja_listado = 0;
        char caminho[256];
        obter_caminho(i, caminho);
        
        // 1. Busca EXATA no nome (maior relevância)
        if (strcmp(fs.cat[i].nome, termo) == 0 && i != 0) {  // Ignora raiz
            printf("%-6d %-10s %-6s %-25s %-40s\n", 
                   fs.cat[i].id, 
                   "EXATA", 
                   fs.cat[i].eh_dir ? "DIR" : "ARQ",
                   fs.cat[i].nome,
                   caminho);
            encontrou = 1;
            ja_listado = 1;
        }
        
        // 2. Busca PARCIAL no nome
        if (!ja_listado && str_contains(fs.cat[i].nome, termo) && i != 0) {
            printf("%-6d %-10s %-6s %-25s %-40s\n", 
                   fs.cat[i].id, 
                   "NOME", 
                   fs.cat[i].eh_dir ? "DIR" : "ARQ",
                   fs.cat[i].nome,
                   caminho);
            encontrou = 1;
            ja_listado = 1;
        }
        
        // 3. Busca no CONTEÚDO (apenas arquivos)
        if (!ja_listado && !fs.cat[i].eh_dir && str_contains(fs.cat[i].cont, termo)) {
            printf("%-6d %-10s %-6s %-25s %-40s\n", 
                   fs.cat[i].id, 
                   "CONTEUDO", 
                   "ARQ",
                   fs.cat[i].nome,
                   caminho);
            encontrou = 1;
            ja_listado = 1;
        }
        
        // 4. Busca nas TAGS
        if (!ja_listado) {
            for (int j = 0; j < fs.cat[i].num_tags; j++) {
                if (str_contains(fs.cat[i].tags[j], termo)) {
                    printf("%-6d %-10s %-6s %-25s %-40s\n", 
                           fs.cat[i].id, 
                           "TAG", 
                           fs.cat[i].eh_dir ? "DIR" : "ARQ",
                           fs.cat[i].nome,
                           caminho);
                    encontrou = 1;
                    break;
                }
            }
        }
    }
    
    if (!encontrou) {
        printf("Nenhum resultado encontrado para '%s'.\n", termo);
        printf("\nDica: A busca procura em nomes, conteudo e tags.\n");
    }
    printf("============================================================================================\n");
}

void listar_todas_tags() {
    printf("\n========== TODAS AS TAGS DO SISTEMA ==========\n");
    
    char tags_unicas[MAX_QS * MAX_TAGS][TAM_TAG];
    int contadores[MAX_QS * MAX_TAGS] = {0};
    int num_tags_unicas = 0;
    
    // Coleta todas as tags únicas
    for (int i = 0; i < fs.qtd; i++) {
        for (int j = 0; j < fs.cat[i].num_tags; j++) {
            int encontrada = 0;
            for (int k = 0; k < num_tags_unicas; k++) {
                if (strcmp(tags_unicas[k], fs.cat[i].tags[j]) == 0) {
                    contadores[k]++;
                    encontrada = 1;
                    break;
                }
            }
            if (!encontrada && num_tags_unicas < MAX_QS * MAX_TAGS) {
                strcpy(tags_unicas[num_tags_unicas], fs.cat[i].tags[j]);
                contadores[num_tags_unicas] = 1;
                num_tags_unicas++;
            }
        }
    }
    
    if (num_tags_unicas == 0) {
        printf("Nenhuma tag cadastrada no sistema.\n");
    } else {
        printf("%-20s %s\n", "TAG", "QUANTIDADE");
        printf("----------------------------------\n");
        for (int i = 0; i < num_tags_unicas; i++) {
            printf("%-20s %d\n", tags_unicas[i], contadores[i]);
        }
    }
    printf("==============================================\n");
}

void busca_avancada() {
    char termo[TAM_NOME];
    int opcao;
    
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║       BUSCA INTELIGENTE - OPCOES        ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║  1 - Busca Fuzzy (nome/conteudo/tags)   ║\n");
    printf("║  2 - Busca por Tag Especifica            ║\n");
    printf("║  3 - Listar Todas as Tags                ║\n");
    printf("║  0 - Voltar                              ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    printf("Escolha: ");
    scanf("%d", &opcao);
    getchar();
    
    switch(opcao) {
        case 1:
            printf("\nDigite o termo de busca: ");
            fgets(termo, TAM_NOME, stdin);
            termo[strcspn(termo, "\n")] = 0;
            busca_fuzzy(termo);
            break;
        case 2:
            printf("\nDigite a tag: ");
            fgets(termo, TAM_NOME, stdin);
            termo[strcspn(termo, "\n")] = 0;
            buscar_por_tag(termo);
            break;
        case 3:
            listar_todas_tags();
            break;
        case 0:
            break;
        default:
            printf("\n[ERRO] Opcao invalida!\n");
    }
}

void menu() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║       SISTEMA DE ARQUIVOS HFS v2.0                 ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║  1  - Criar diretorio                              ║\n");
    printf("║  2  - Criar arquivo                                ║\n");
    printf("║  3  - Ler arquivo                                  ║\n");
    printf("║  4  - Escrever/Atualizar arquivo                   ║\n");
    printf("║  5  - Deletar arquivo                              ║\n");
    printf("║  6  - Deletar diretorio                            ║\n");
    printf("║  7  - Listar conteudo de diretorio                 ║\n");
    printf("║  8  - Exibir arvore de diretorios                  ║\n");
    printf("║  9  - Informacoes do volume                        ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║         🔍 FUNCIONALIDADES DE BUSCA 🔍             ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║  10 - Adicionar tag                                ║\n");
    printf("║  11 - Remover tag                                  ║\n");
    printf("║  12 - Busca inteligente                            ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║  0  - Sair                                         ║\n");
    printf("╚════════════════════════════════════════════════════╝\n");
    printf("Escolha uma opcao: ");
}

int main() {
    char nome_vol[TAM_NOME];
    char nome[TAM_NOME];
    char conteudo[TAM_BLOCO*4];
    char tag[TAM_TAG];
    int opcao, id_pai;
    
    printf("╔═════════════════════════════════════════════════╗\n");
    printf("║           SISTEMA DE ARQUIVOS HFS v2.0          ║\n");
    printf("╚═════════════════════════════════════════════════╝\n");
    
    printf("\nDigite o nome do volume: ");
    fgets(nome_vol, TAM_NOME, stdin);
    nome_vol[strcspn(nome_vol, "\n")] = 0;
    
    init_hfs(nome_vol);
    
    while (1) {
        menu();
        scanf("%d", &opcao);
        getchar();
        
        switch(opcao) {
            case 1:  // Criar Diretório
                printf("\nNome do diretorio: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("ID do diretorio pai: ");
                scanf("%d", &id_pai);
                getchar();
                
                cria_dir(nome, id_pai);
                break;
                
            case 2:  // Criar Arquivo
                printf("\nNome do arquivo: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("ID do diretorio pai: ");
                scanf("%d", &id_pai);
                getchar();
                
                printf("Conteudo do arquivo: ");
                fgets(conteudo, TAM_BLOCO*4, stdin);
                conteudo[strcspn(conteudo, "\n")] = 0;
                
                cria_arq(nome, id_pai, conteudo);
                break;
                
            case 3:  // Ler Arquivo
                printf("\nNome do arquivo: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("ID do diretorio pai: ");
                scanf("%d", &id_pai);
                getchar();
                
                le_arq(nome, id_pai);
                break;
                
            case 4:  // Escrever/Atualizar Arquivo
                printf("\nNome do arquivo: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("ID do diretorio pai: ");
                scanf("%d", &id_pai);
                getchar();
                
                printf("Novo conteudo: ");
                fgets(conteudo, TAM_BLOCO*4, stdin);
                conteudo[strcspn(conteudo, "\n")] = 0;
                
                escreve_arq(nome, id_pai, conteudo);
                break;
                
            case 5:  // Deletar Arquivo
                printf("\nNome do arquivo: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("ID do diretorio pai: ");
                scanf("%d", &id_pai);
                getchar();
                
                del_arq(nome, id_pai);
                break;
                
            case 6:  // Deletar Diretório
                printf("\nNome do diretorio: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("ID do diretorio pai: ");
                scanf("%d", &id_pai);
                getchar();
                
                del_dir(nome, id_pai);
                break;
                
            case 7:  // Listar
                printf("\nID do diretorio: ");
                scanf("%d", &id_pai);
                getchar();
                
                listar(id_pai);
                break;
                
            case 8:  // Exibir Árvore
                exibir_arvore();
                break;
                
            case 9:  // Informações
                info();
                break;
            
            // ===== NOVAS FUNCIONALIDADES =====
            case 10:  // Adicionar Tag
                printf("\nNome do arquivo/diretorio: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("ID do diretorio pai: ");
                scanf("%d", &id_pai);
                getchar();
                
                printf("Tag a adicionar: ");
                fgets(tag, TAM_TAG, stdin);
                tag[strcspn(tag, "\n")] = 0;
                
                adicionar_tag(nome, id_pai, tag);
                break;
            
            case 11:  // Remover Tag
                printf("\nNome do arquivo/diretorio: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("ID do diretorio pai: ");
                scanf("%d", &id_pai);
                getchar();
                
                printf("Tag a remover: ");
                fgets(tag, TAM_TAG, stdin);
                tag[strcspn(tag, "\n")] = 0;
                
                remover_tag(nome, id_pai, tag);
                break;
            
            case 12:  // Busca Inteligente
                busca_avancada();
                break;
                
            case 0:  // Sair
                printf("\n╔════════════════════════════════════════════════════╗\n");
                printf("║     ENCERRANDO O SISTEMA HFS. ATE LOGO!           ║\n");
                printf("╚════════════════════════════════════════════════════╝\n\n");
                return 0;
                
            default:
                printf("\n[ERRO] Opcao invalida! Tente novamente.\n");
        }
        
        printf("\nPressione ENTER para continuar...");
        getchar();
    }
    
    return 0;
}