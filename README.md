# 💣 Final Sequence

Simulador de desarme de bomba criado como projeto para a disciplina de **Sistemas Digitais**.
O jogador deve desconectar os fios na ordem correta usando um circuito com **Arduino Uno**, baseando-se no Manual de Instruções.


![Final Sequence]()


## 🛠️ Componentes Utilizados
- Arduino Uno  
- Protoboard  
- Fios jumper  
- LEDs (verde, vermelho, azul e amarelo)  
- 2 LCDs I2C 16x2 (um para mensagens e outro para o tempo restante)

## 🧠 Como Funciona

- Existem **6 fios jumper** conectados aos pinos digitais 8 a 13 do Arduino.
  
- A cada tentativa, uma **ordem específica de remoção dos fios** é selecionada aleatoriamente dentre **16 sequências possíveis**.
  
- Essa ordem correta é **codificada por LEDs coloridos** (vermelho, verde, azul, amarelo), formando um "código visual" exclusivo para cada sequência.
  
- O jogador deve **desconectar os fios fisicamente na sequência certa**, de acordo com o manual.
  
- Se errar a ordem, o sistema exibe "BOOM!" e reinicia após reconectar todos os fios.
  
- Se o tempo máximo for excedido, o sistema também reinicia com mensagem de tempo esgotado.

## 🔢 Detalhes Técnicos

- As sequências são compostas por **índices dos fios** (de 0 a 5), mapeados aos pinos físicos 8 a 13.
- Os LCDs exibem mensagens como "Correto!", "Incorreto!", tempo restante e mensagens de início/fim.
- O jogador deve **reconectar todos os fios** para começar uma nova tentativa após erro ou sucesso.

- A sequência correta para o funcionamento do manual é:  
  **vermelho, laranja, amarelo, verde, azul e roxo**, nessa ordem específica.

- Por padrão, o código assume que esses fios estão conectados assim:
  - **Pino 8**: vermelho
  - **Pino 9**: laranja
  - **Pino 10**: amarelo
  - **Pino 11**: verde
  - **Pino 12**: azul
  - **Pino 13**: roxo

- ⚠️ **Importante**: o manual só funcionará corretamente se essa ordem for mantida, independentemente dos pinos utilizados.  
  Caso a ordem dos fios seja alterada fisicamente, o código deverá ser ajustado para refletir a nova correspondência.

## 📋 Exemplo de Feedback no LCD
- ✅ Correto: `Correto! / Continue!`
- ❌ Erro: `Incorreto! / BOOM!`
- 🕒 Tempo: `Tempo restante: 01:59`
- ⏰ Erro por tempo: `ACABOU O TEMPO! / Reconecte tudo`
- 🧠 Sucesso: `SUCESSO! / Sequencia completa`

## 💡 Como Jogar

1. Ligue o circuito (caso necessário reconecte os fios) e espere a mensagem “Comece!”.
2. Observe a combinação de LEDs acesos – ela representa uma das 16 sequências.
3. Consulte o manual impresso que mapeia os padrões de LED para a sequência correta.
4. Desconecte fisicamente os fios jumper um por um na ordem correta.
5. Em caso de erro ou tempo esgotado, reconecte todos os fios para reiniciar.

## 📸 Imagem do Circuito

![Circuito no Tinkercad](https://github.com/user-attachments/assets/1478be42-fa04-4149-a3de-a88576012a39)

## 📄 Licença
Este projeto é aberto e pode ser usado com fins educacionais.  
Distribuído sob licença [MIT](LICENSE).
