# CP5-EDGE
No projeto, enfrentamos o desafio de desenvolver um sistema integrado de sensores de luz, umidade e temperatura e são transmitidos para um dashboard dinâmico, para garantir a conservação ideal dos vinhos em uma vinícola.

# Descrição
Este projeto implementa um sistema de monitoramento de ambiente utilizando um microcontrolador ESP32, um sensor DHT22 para medir temperatura e umidade, além de um sensor de luminosidade. Os dados coletados são enviados a um broker MQTT para serem processados ou armazenados. 

# Componentes Utilizados:
- ESP32: Microcontrolador com conectividade Wi-Fi e suporte a MQTT.
- DHT22: Sensor digital para medir temperatura e umidade.
- Broker MQTT: Responsável por gerenciar a comunicação de mensagens entre o ESP32 e o servidor. (IP do broker configurado no código)
- Sensor de Luminosidade: Utiliza o pino ADC do ESP32 para medir o nível de luz ambiente.

# Bibliotecas Necessárias:
- WiFi.h: Para conectar o ESP32 a uma rede Wi-Fi.
- PubSubClient.h: Implementa o protocolo MQTT para comunicação com o broker.
- DHT.h: Para ler os valores de temperatura e umidade do sensor DHT22.

#Links importantes:
- Link de simulção do wokwi: https://wokwi.com/projects/410403677713263617
- link do video no Youtube:  https://youtu.be/dnu_Ifo0wfc

# Integrantes  do grupo:
- Estevam Melo: RM555124
- Gustavo Morais: RM554972
- Leonardo Novaes: RM554807
- Miguel Carmo: RM557622


