# GS 2025 – SmartBreak IoT

Diogo Julio Oliveira - RM553837

**Estação de Bem-Estar e Produtividade para o Futuro do Trabalho**

## 1. Visão Geral

O projeto **SmartBreak IoT** propõe uma estação inteligente para o posto de trabalho (home office ou escritório híbrido) que monitora condições de conforto e rotina de pausas do profissional, gerando alertas em tempo real e enviando os dados para a nuvem via **MQTT**.

A solução dialoga com o tema **“O Futuro do Trabalho”**, integrando:
- Bem-estar e saúde ocupacional;
- Ambientes híbridos e conectados;
- Uso de IoT para tomada de decisão baseada em dados;
- Possibilidade de dashboards e relatórios para RH/gestores no futuro.

---

## 2. Problema

Com o aumento do home office e dos ambientes híbridos, muitos profissionais passam longos períodos sentados, em ambientes pouco ventilados, com iluminação inadequada e quase sem pausas. Isso aumenta o risco de:
- fadiga;
- queda de produtividade;
- dores musculares;
- estresse e esgotamento.

Falta uma solução simples que ajude a monitorar essas condições e **sugerir pausas inteligentes**, alinhando saúde e desempenho.

---

## 3. Solução Proposta

A estação **SmartBreak IoT** usa um **ESP32** conectado a sensores para:
- Medir **temperatura e umidade** do ambiente (DHT22);
- Medir **nível de iluminação** (LDR);
- Registrar quando o colaborador realiza uma **pausa** (botão);
- Disparar **alertas visuais e sonoros** (LED + buzzer) quando:
  - as condições estão fora da faixa recomendada; ou
  - o colaborador fica muito tempo sem pausa.
- Enviar periodicamente os dados via **MQTT**, em formato JSON.

Esse conjunto permite, em uma etapa futura, conectar dashboards, relatórios de bem-estar e políticas de ergonomia baseadas em dados reais.

---

## 4. Componentes Utilizados

- 1x ESP32 DevKit V1
- 1x Sensor DHT22 (temperatura/umidade)
- 1x Sensor LDR (fotoresistor) com resistor (módulo pronto do Wokwi)
- 1x LED (alerta visual)
- 1x Buzzer ativo (alerta sonoro)
- 1x Botão (registro de pausa)
- Jumpers / Protoboard (no caso físico)

---

## 5. Esquema do Circuito (Wokwi)

No repositório há o arquivo `wokwi-diagram.json` com o circuito pronto para importação.

Conexões principais:
- **DHT22**
  - VCC → 3V3 do ESP32
  - GND → GND
  - DATA → GPIO 15
- **LDR (fotoresistor)** (módulo)
  - VCC → 3V3
  - GND → GND
  - SIG → GPIO 34 (entrada analógica)
- **LED**
  - Anodo (A) → GPIO 2
  - Catodo (C) → GND
- **Buzzer ativo**
  - + → GPIO 4
  - - → GND
- **Botão de pausa**
  - Um terminal → GPIO 5
  - Outro terminal → GND
  - Configurado com `INPUT_PULLUP` no código

---

## 6. Como Executar no Wokwi (Passo a Passo)

1. Acesse o site do **Wokwi**.
2. Crie um novo projeto com **ESP32**.
3. Substitua o código padrão pelo conteúdo do arquivo `smartbreak_iot.ino` deste repositório.
4. Crie (ou edite) o arquivo `diagram.json` do projeto e cole o conteúdo de `wokwi-diagram.json`.
5. Clique em **Play ▶** para simular.
6. Abra o **Serial Monitor** do Wokwi para acompanhar:
   - Conexão ao Wi-Fi;
   - Conexão ao broker MQTT;
   - Leituras dos sensores e alertas publicados.
7. Ajuste os controles dos sensores virtuais (temperatura, umidade, luz) para forçar cenários de alerta e demonstrar o funcionamento.

> Para entrega, deixe o projeto como **Público** no Wokwi e coloque o link no README e no ambiente da disciplina.

---

## 7. MQTT – Tópicos e Formato das Mensagens

**Broker utilizado (demo):** `test.mosquitto.org` porta `1883`  
*(público, apenas para fins acadêmicos; em produção usar broker privado)*

### Tópicos

- `gs2025/smartbreak/estacao1/sensores`
  - Envia, a cada 5 segundos, um JSON com as leituras atuais:
  ```json
  {
    "temp": 24.5,
    "hum": 52.3,
    "light": 70,
    "minsSinceBreak": 3,
    "alert": false
  }
  ```

- `gs2025/smartbreak/estacao1/alertas`
  - Publicado somente quando há alguma condição crítica ou muito tempo sem pausa:
  ```json
  {
    "motivo": "Temperatura acima do ideal; Muito tempo sem pausa; "
  }
  ```

Esses tópicos podem ser consumidos por um painel web, mobile ou dashboard corporativo no futuro, reforçando o alinhamento com **"O Futuro do Trabalho"**.

---

## 8. Lógica de Funcionamento

1. O ESP32 conecta ao Wi-Fi (`Wokwi-GUEST`) e ao broker MQTT.
2. A cada 5 segundos:
   - Lê temperatura e umidade (DHT22);
   - Lê luminosidade (LDR);
   - Calcula o tempo desde a última pausa registrada pelo botão.
3. Verifica as regras de bem-estar:
   - Temperatura fora de 20–26 ºC;
   - Umidade fora de 30–70%;
   - Luminosidade muito baixa;
   - Tempo sem pausa acima do limite (1 minuto na demo; sugerido 50 minutos na aplicação real).
4. Se houver problema:
   - Acende o LED;
   - Emite dois bipes no buzzer;
   - Publica mensagem no tópico de alertas.
5. Sempre envia o JSON com as leituras no tópico de sensores.
6. Quando o usuário aperta o botão, o tempo sem pausa é zerado, simulando uma pausa realizada.

---


## 9. Como Adaptar para Protótipo Físico

- Utilizar os mesmos componentes da simulação.
- Configurar a rede Wi-Fi real da FIAP (ou hotspot) nas constantes `ssid` e `password`.
- Manter o mesmo código-fonte, apenas ajustando o tempo real de pausa (50 minutos) e, se desejado, trocar o broker MQTT.

---
