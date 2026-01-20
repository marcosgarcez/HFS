# 🗂️ HFS - Hierarchical File System

[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](https://github.com)

> Uma implementação educacional do sistema de arquivos HFS (Hierarchical File System) da Apple com recursos modernos de busca inteligente e organização por tags.

![HFS Demo](https://via.placeholder.com/800x400/1a1a2e/ffffff?text=HFS+v2.0+-+Demonstração)

## 📋 Índice

- [Sobre o Projeto](#-sobre-o-projeto)
- [Funcionalidades](#-funcionalidades)
- [Começando](#-começando)
- [Como Usar](#-como-usar)
- [Arquitetura](#-arquitetura)
- [Inovações](#-inovações)
- [Exemplos](#-exemplos)
- [Contribuindo](#-contribuindo)
- [Licença](#-licença)
- [Contato](#-contato)

## 🎯 Sobre o Projeto

Este projeto é uma implementação didática do **HFS (Hierarchical File System)**, o sistema de arquivos utilizado pela Apple de 1985 até 2017. O objetivo é demonstrar de forma prática os conceitos fundamentais de sistemas de arquivos hierárquicos.

### Por que HFS?

- **Histórico**: Sistema usado por mais de 30 anos pela Apple
- **Didático**: Arquitetura clara e bem documentada
- **Relevante**: Conceitos aplicados em sistemas modernos
- **Elegante**: Design simples mas eficaz

### O que foi implementado?

✅ Estrutura hierárquica de diretórios (árvore)  
✅ Catálogo centralizado de arquivos  
✅ Alocação de blocos com extents  
✅ Master Directory Block (MDB)  
✅ **Sistema de tags** (inovação)  
✅ **Busca inteligente** multi-critério (inovação)  
✅ Interface amigável via terminal  

## ✨ Funcionalidades

### Funcionalidades Básicas do HFS

- **Criar/Deletar Diretórios**: Estrutura hierárquica completa
- **Criar/Deletar Arquivos**: Até 2KB por arquivo
- **Ler/Escrever Arquivos**: Edição de conteúdo
- **Listar Conteúdo**: Visualização de diretórios
- **Árvore de Diretórios**: Visualização da hierarquia completa
- **Informações do Volume**: Estatísticas de uso

### Inovações Implementadas ⭐

- **🏷️ Sistema de tags**: Até 5 tags por arquivo/diretório
- **🔍 Busca inteligente**: Busca por nome, conteúdo e tags
- **📊 Busca Fuzzy**: Case-insensitive e parcial
- **🎯 Classificação por relevância**: Resultados ordenados
- **📂 Caminho completo**: Exibição do path absoluto

## 🚀 Começando

### Pré-requisitos

- GCC (ou qualquer compilador C compatível)
- Sistema operacional: Linux, macOS ou Windows (com MinGW)
- Terminal/Console

### Instalação

1. Clone o repositório:
```bash
git clone https://github.com/seu-usuario/hfs-file-system.git
cd hfs-file-system
```

2. Compile o código:
```bash
gcc -o hfs HFS.c -Wall
```

3. Execute:
```bash
./hfs
```

## 💻 Como Usar

### Início Rápido

Ao executar o programa, você verá:

```
╔═════════════════════════════════════════════════╗
║          SISTEMA DE ARQUIVOS HFS v2.0           ║
╚═════════════════════════════════════════════════╝

Digite o nome do volume: MeuHD
```

### Menu Principal

```
╔════════════════════════════════════════════════════╗
║           SISTEMA DE ARQUIVOS HFS v2.0             ║
╠════════════════════════════════════════════════════╣
║  1  - Criar diretório                              ║
║  2  - Criar arquivo                                ║
║  3  - Ler arquivo                                  ║
║  4  - Escrever/Atualizar arquivo                   ║
║  5  - Deletar arquivo                              ║
║  6  - Deletar diretório                            ║
║  7  - Listar conteúdo de diretório                 ║
║  8  - Exibir árvore de diretórios                  ║
║  9  - Informações do volume                        ║
╠════════════════════════════════════════════════════╣
║         🔍 FUNCIONALIDADES DE BUSCA 🔍             ║
╠════════════════════════════════════════════════════╣
║  10 - Adicionar tag                                ║
║  11 - Remover tag                                  ║
║  12 - Busca inteligente                            ║
╠════════════════════════════════════════════════════╣
║  0  - Sair                                         ║
╚════════════════════════════════════════════════════╝
```

### Exemplo de Uso

```bash
# 1. Criar diretório "documentos"
Opção: 1
Nome: documentos
ID pai: 1

# 2. Criar arquivo "projeto.txt"
Opção: 2
Nome: projeto.txt
ID pai: 2
Conteúdo: Este é meu projeto de TCC sobre sistemas de arquivos.

# 3. Adicionar tags
Opção: 10
Nome: projeto.txt
ID pai: 2
Tag: importante

# 4. Buscar por "TCC"
Opção: 12
Escolha: 1 (Busca Fuzzy)
Termo: tcc
# Resultado: Encontra "projeto.txt" por conteúdo!
```

## 🏗️ Arquitetura

### Componentes Principais

```
┌─────────────────────────────────────────────┐
│        MASTER DIRECTORY BLOCK (MDB)         │
│  • Nome do volume                           │
│  • Total de blocos: 1024                    │
│  • Blocos livres                            │
│  • Tamanho do bloco: 512 bytes              │
│  • Próximo ID                               │
└─────────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────────┐
│         CATÁLOGO DE ARQUIVOS                │
│  Array de até 100 registros (Reg)          │
│                                             │
│  Cada registro contém:                      │
│  • ID único                                 │
│  • ID do pai (hierarquia)                   │
│  • Nome                                     │
│  • Tipo (arquivo/diretório)                 │
│  • Conteúdo                                 │
│  • Extents (localização)                    │
│  • Tags (novo!)                             │
│  • Timestamps                               │
└─────────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────────┐
│           MAPA DE BLOCOS                    │
│  Bitmap de 1024 posições                    │
│  0 = livre, 1 = ocupado                     │
└─────────────────────────────────────────────┘
```

### Estruturas de Dados

```c
// Master Directory Block
typedef struct {
    char nome[TAM_NOME];
    int tot_bl, liv_bl, tam_bl, prox_id;
    time_t criacao, modif;
} MDB;

// Extent (blocos contíguos)
typedef struct { 
    int bl_ini, n_bl; 
} Extent;

// Registro (arquivo ou diretório)
typedef struct {
    int id, id_pai, eh_dir, tam, qtd_ext;
    char nome[TAM_NOME], cont[TAM_BLOCO*4];
    time_t criacao, modif;
    Extent ext[MAX_EXT];
    char tags[MAX_TAGS][TAM_TAG];  // Inovação
    int num_tags;                   // Inovação
} Reg;
```

### Alocação de Blocos

O sistema usa **extents** para alocação eficiente:

1. Calcula quantos blocos são necessários
2. Busca blocos **contíguos** no mapa
3. Marca blocos como ocupados
4. Registra extent (início + quantidade)

**Vantagem**: Reduz fragmentação

## 🌟 Inovações

### 1. Sistema de Tags

Organização moderna inspirada em sistemas como macOS Finder:

```c
// Adicionar tags a um arquivo
adicionar_tag("projeto.txt", 2, "importante");
adicionar_tag("projeto.txt", 2, "trabalho");

// Buscar por tag
buscar_por_tag("importante");
// Retorna todos os arquivos com essa tag
```

**Benefícios:**
- Organização flexível (não depende da hierarquia)
- Um arquivo pode ter múltiplas categorias
- Busca rápida por contexto

### 2. Busca Inteligente

Busca multi-critério com 3 níveis:

```c
busca_fuzzy("projeto");
```

**Procura em:**
1. **Nome do arquivo** (parcial, case-insensitive)
2. **Conteúdo do arquivo** (full-text search)
3. **Tags associadas**

**Classificação por relevância:**
- EXATA: Nome corresponde exatamente
- NOME: Nome contém o termo
- CONTEÚDO: Conteúdo contém o termo
- TAG: Tag contém o termo

### 3. Busca Fuzzy

Características:
- Case-insensitive: "TCC" = "tcc" = "Tcc"
- Busca parcial: "proj" encontra "projeto"
- Multi-campo: nome + conteúdo + tags

## 📊 Exemplos

### Criando uma Estrutura Completa

```c
// Estrutura:
// /
// ├── documentos/
// │   ├── tcc.txt [importante, faculdade]
// │   └── notas.txt [faculdade]
// └── projetos/
//     └── codigo.c [trabalho]

// 1. Criar diretórios
cria_dir("documentos", 1);
cria_dir("projetos", 1);

// 2. Criar arquivos
cria_arq("tcc.txt", 2, "Projeto sobre sistemas de arquivos HFS");
cria_arq("notas.txt", 2, "Anotações da aula de SO");
cria_arq("codigo.c", 3, "#include <stdio.h>\nint main() { return 0; }");

// 3. Adicionar tags
adicionar_tag("tcc.txt", 2, "importante");
adicionar_tag("tcc.txt", 2, "faculdade");
adicionar_tag("notas.txt", 2, "faculdade");
adicionar_tag("codigo.c", 3, "trabalho");

// 4. Visualizar
exibir_arvore();
```

### Busca Inteligente

```c
// Busca por "HFS"
busca_fuzzy("HFS");

// Resultados:
// ID     TIPO BUSCA  TIPO   NOME          CAMINHO
// 5      CONTEUDO    ARQ    tcc.txt       /documentos/tcc.txt

// Busca por tag
buscar_por_tag("faculdade");

// Resultados:
// ID     TIPO   NOME          CAMINHO
// 5      ARQ    tcc.txt       /documentos/tcc.txt
// 6      ARQ    notas.txt     /documentos/notas.txt
```

## 📈 Especificações Técnicas

| Característica | Valor |
|----------------|-------|
| Número máximo de arquivos/diretórios | 100 |
| Tamanho do bloco | 512 bytes |
| Total de blocos | 1024 (512 KB) |
| Tamanho máximo por arquivo | 2048 bytes |
| Extents por arquivo | 3 |
| Tags por item | 5 |
| Tamanho do nome | 32 caracteres |

## 🔧 Limitações

Este é um projeto **educacional**. Limitações conhecidas:

- ⚠️ Sem persistência em disco (dados em memória)
- ⚠️ Sem journaling
- ⚠️ Sem permissões de usuário
- ⚠️ Sem suporte a concorrência
- ⚠️ Sem compactação/criptografia
- ⚠️ Busca linear O(n) (adequado para 100 itens)


## 🤝 Contribuindo

Contribuições são bem-vindas! Para contribuir:

1. Fork o projeto
2. Crie sua feature branch (`git checkout -b feature/NovaFuncionalidade`)
3. Commit suas mudanças (`git commit -m 'Adiciona nova funcionalidade'`)
4. Push para a branch (`git push origin feature/NovaFuncionalidade`)
5. Abra um Pull Request

### Diretrizes

- Mantenha o código limpo e comentado
- Atualize a documentação
- Siga o estilo de código existente

## 📚 Referências

### Documentação Oficial
- [Apple Technical Note TN1150 - HFS Plus Volume Format](https://developer.apple.com/library/archive/technotes/tn/tn1150.html)
- [Inside Macintosh: Files](https://developer.apple.com/library/archive/documentation/mac/pdf/Files/File_Manager.pdf)

### Artigos e Livros
- Giampaolo, D. (1998). *Practical File System Design with the Be File System*
- Tanenbaum, A. (2014). *Modern Operating Systems* (4th ed.)
- Love, R. (2010). *Linux Kernel Development* (3rd ed.)

### Recursos Online
- [OSDev Wiki - File Systems](https://wiki.osdev.org/File_Systems)
- [File System Design - GeeksforGeeks](https://www.geeksforgeeks.org/file-system-design/)


## 👨‍💻 Autores

**Marcos Garcez**
- GitHub: [@marcosgarcez](https://github.com/seu-usuario)

**Lucas Lins**
- GitHub: [@Lucasllins](https://github.com/Lucasllins)



---

<div align="center">

### ⭐ Se este projeto foi útil, considere dar uma estrela!

**Feito com ❤️ e C**

[⬆ Voltar ao topo](#-hfs---hierarchical-file-system)

</div>