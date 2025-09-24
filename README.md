# Aplicação de Bate-Papo com Protocolo MQTT

## Sumário
1. Introdução  
2. Objetivos  
3. Requisitos de Desenvolvimento  
4. Estrutura de Controle por Tópicos  
   4.1 Usuários  
   4.2 Conversas One-to-One  
   4.3 Grupos  
5. Funcionalidades  
6. Etapas do Projeto  
   6.1 Etapa 1  
   6.2 Etapa 2  
7. Arquitetura do Sistema  
8. Execução  
9. Considerações Finais  


# 1. Introdução
O presente trabalho tem como objetivo o desenvolvimento de uma aplicação de **bate-papo (chat)** utilizando exclusivamente o protocolo **MQTT**.  
A aplicação deverá oferecer comunicação **um-a-um (one-to-one)** e **em grupo (group chat)**, garantindo persistência de dados para usuários offline.  


# 2. Objetivos
O projeto visa implementar uma solução de comunicação baseada em MQTT, atendendo aos seguintes objetivos:  
- Comunicação entre usuários de forma privada e em grupo.  
- Garantir unicidade dos identificadores de usuários (ID).  
- Fornecer um mecanismo de persistência para usuários offline.  
- Implementar uma interface em modo texto para interação do usuário.  


# 3. Requisitos de Desenvolvimento
- **Sistema Operacional**: Linux  
- **Biblioteca**: Eclipse Paho  
- **Linguagem de Programação**: C  
- **Protocolo**: MQTT  


# 4. Estrutura de Controle por Tópicos

## 4.1 Usuários
- `USERS`: tópico global para publicação do estado online/offline de todos os usuários.  
- `ID_Control`: tópico individual para cada usuário, usado em negociações de sessão.  

## 4.2 Conversas One-to-One
- Sessões são criadas com tópicos exclusivos no formato:  

   `usuario1\_usuario2\_timestamp`

- O identificador da sessão é comunicado ao solicitante via publicação no tópico de controle.  

## 4.3 Grupos
- `GROUPS`: tópico global para informações de grupos.  
- Cada grupo possui um líder responsável pela aceitação de novos membros.  
- A atualização da lista de membros é publicada no tópico `GROUPS`.  


# 5. Funcionalidades
A aplicação contempla:  
1. Listagem de usuários e status (online/offline).  
2. Criação de grupos, com o criador assumindo o papel de líder.  
3. Listagem de grupos cadastrados, incluindo líder e membros.  
4. Solicitação e aceite de conversas one-to-one.  
5. Gerenciamento de grupos, com inclusão de novos membros.  
6. Persistência de mensagens para usuários offline.  


# 6. Etapas do Projeto

## 6.1 Etapa 1
- Definição da arquitetura.  
- Estruturação dos tópicos de controle.  
- Implementação da interface em modo texto.  

## 6.2 Etapa 2
- Implementação da comunicação efetiva.  
- Elaboração do relatório descritivo.  


# 7. Arquitetura do Sistema
A arquitetura é composta por:  
- **Camada de Comunicação MQTT**: utiliza a biblioteca Paho para enviar e receber mensagens.  
- **Camada de Controle**: gerencia usuários, grupos, solicitações e status.  
- **Camada de Persistência**: armazena dados de sessões e mensagens.  
- **Interface de Usuário**: menus interativos em modo texto.  

