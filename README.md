# 🌐 Projeto de Aplicação de Bate-Papo Baseada no MQTT

Este projeto demonstra uma aplicação simples de bate-papo utilizando o **protocolo MQTT**. Através dele, é possível enviar e receber mensagens em tópicos específicos usando um usuário e senha definidos.

---

## ⚙️ Pré-requisitos

Instale a biblioteca MQTT para C:

```bash
sudo apt install libpaho-mqtt-dev
```

---

## 🔑 Configuração do Usuário

Crie um usuário para autenticação no Mosquitto:

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd {nome_do_usuario}
```

> Você será solicitado a inserir a senha do usuário.

---

## 🚀 Rodando o Chat

### 1️⃣ Assinando um Tópico

Abra um terminal e rode o assinante para escutar mensagens em um tópico específico:

```bash
mosquitto_sub -h localhost -t "{topico}" -u "{nome_do_usuario}" -P "{senha_do_usuario}"
```

> O terminal ficará aguardando mensagens publicadas no tópico.

### 2️⃣ Publicando uma Mensagem

Em outro terminal, envie uma mensagem para o mesmo tópico:

```bash
mosquitto_pub -h localhost -t "{topico}" -m "{mensagem}" -u "{nome_do_usuario}" -P "{senha_do_usuario}"
```

> Assim que a mensagem for enviada, ela aparecerá no terminal do assinante.

---

## ⚠️ Observações

* O `{topico}` pode ser qualquer string que represente o canal de comunicação.
* Certifique-se de usar o mesmo usuário e senha nos comandos de publicação e assinatura.
* Para encerrar o assinante, pressione **`Ctrl + C`**.

---

💡 Dica: Use nomes de tópicos diferentes para canais separados e teste múltiplos usuários para simular um chat real.
