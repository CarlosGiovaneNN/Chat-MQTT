

# Chat Seguro com MQTT em C

Uma aplicação de bate-papo robusta e segura, desenvolvida em C, que utiliza o protocolo MQTT para comunicação em tempo real. O projeto implementa conversas privadas e em grupo, com foco em segurança, persistência de dados e uma arquitetura multithread para garantir uma experiência de usuário fluida e responsiva.


## Funcionalidades Principais

* **Comunicação Segura**: Todas as mensagens trocadas são criptografadas de ponta a ponta utilizando **AES-256**, garantindo a confidencialidade das conversas.
* **Conversas Privadas e em Grupo**: Crie chats privados com outros usuários ou participe de grupos temáticos.
* **Gerenciamento de Grupos**: Crie grupos, adicione participantes e gerencie convites. Cada grupo possui um líder com permissões para aceitar novos membros.
* **Status de Usuário**: Veja quais usuários estão online ou offline em tempo real.
* **Persistência de Dados**: Mensagens e informações de usuários/grupos são salvas em arquivos locais, garantindo que os dados não sejam perdidos ao reiniciar a aplicação.
* **Interface de Linha de Comando**: Um menu interativo e simples de usar para navegar por todas as funcionalidades do chat.
* **Arquitetura Multithread**: O sistema utiliza threads para gerenciar a interface do usuário e as tarefas de rede de forma independente, evitando bloqueios e garantindo que o programa continue responsivo.


## Como Executar

### Pré-requisitos

Antes de começar, certifique-se de que você tem os seguintes componentes instalados:

1.  **GCC (Compilador C)**: Essencial para compilar o projeto.
2.  **Make**: Para automatizar o processo de compilação.
3.  **Broker MQTT**: O broker [Mosquitto](https.mosquitto.org) deve estar instalado e rodando na máquina local.
4. **Bibliotecas de Desenvolvimento**:
    * **Paho MQTT C**: Biblioteca para comunicação MQTT.
    * **OpenSSL**: Biblioteca para as funções de criptografia.
    

### Compilação

Com todas as dependências instaladas, clone o repositório e compile o projeto usando o `Makefile` fornecido:

```bash
git clone https://github.com/CarlosGiovaneNN/Chat-MQTT.git
cd Chat-MQTT
make
```

Este comando irá compilar todos os arquivos `.c` e gerar um executável chamado `main`.

### Execução

Para iniciar o cliente de chat, execute o programa a partir do terminal, passando um nome de usuário único como argumento:

```bash
./main <seu_nome_de_usuario>
```

**Exemplo:**

```bash
./main user
```

Você pode abrir múltiplos terminais e executar o programa com diferentes nomes de usuário para simular uma conversa entre eles.

## Arquitetura do Sistema

O sistema é modular e projetado em torno do protocolo MQTT, utilizando uma estrutura de tópicos para gerenciar a comunicação.

### Estrutura de Tópicos

  * `USERS`: Tópico global usado para anunciar o status (online/offline) de todos os usuários. Mensagens de "connected" e "disconnected" são publicadas aqui.
  * `GROUPS`: Tópico para a criação e gerenciamento de grupos. Quando um novo grupo é criado ou um membro é adicionado, uma mensagem é publicada aqui para notificar outros clientes.
  * `<username>_CONTROL`: Tópico de controle individual para cada usuário. É usado para negociações que não devem ser públicas, como convites para chats privados e pedidos para entrar em grupos.
  * Tópicos de Chat Dinâmicos:
      * **Grupos**: O nome do grupo serve como tópico para as mensagens (`<nome_do_grupo>`).
      * **Privados**: Um tópico único é gerado para cada conversa privada, combinando os nomes dos usuários e um timestamp (`usuario1_usuario2_timestamp`), garantindo que a conversa seja exclusiva.

### Criptografia

A segurança é um pilar deste projeto. Antes de serem publicadas, todas as mensagens são formatadas, criptografadas com **AES-256**, e então publicadas no tópico MQTT. Ao receber uma mensagem, o cliente a descriptografa antes de exibi-la ao usuário.

## Monitoramento e Debug

Para facilitar o desenvolvimento e a depuração, o projeto inclui um script de monitoramento (`monitor_mqtt.sh`) que abre três terminais para observar a atividade do broker:

1.  **Visualizador de Mensagens**: Inscreve-se em todos os tópicos (`#`) e exibe todas as mensagens publicadas em tempo real.
2.  **Logs do Broker**: Mostra os logs do serviço Mosquitto, útil para identificar erros de conexão e autenticação.
3.  **Analisador de Tráfego (TCPDump)**: Captura e exibe o tráfego de rede bruto na porta 1883, permitindo uma análise de baixo nível dos pacotes MQTT.

Para usar, dê permissão de execução e rode o script:

```bash
chmod +x monitor_mqtt.sh
./monitor_mqtt.sh
```

## Limpeza

Para remover todos os arquivos de objeto (`.o`) e o executável `main`, utilize o comando:

```bash
make clean
```
