/*
 * TEST_ADDITEL_ETHERNET.ino
 * Teste de comunicação com Additel usando Ethernet (W5500)
 * 
 * BASEADO NO CÓDIGO ORIGINAL QUE FUNCIONAVA!
 * 
 * IP Additel: 192.168.0.180 (CORRIGIDO)
 * Porta: 8000
 */

#include <SPI.h>
#include <Ethernet.h>

// Pino de Chip Select para a W5500
#define W5500_CS 27

// Configurações de Rede
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip_esp(192, 168, 0, 201);      
IPAddress ip_additel(192, 168, 0, 180);  // CORRIGIDO: 180 em vez de 182

EthernetClient client;

// Variáveis para o temporizador de 30 segundos
unsigned long ultimaExecucao = 0;
const unsigned long intervalo = 30000;

void lerResposta(String prefixo) {
  delay(500); 
  if (client.available()) {
    Serial.print(prefixo);
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Ethernet.init(W5500_CS);
  Serial.println("\n=== TEST ADDITEL ETHERNET ===\n");
  Serial.println("Inicializando Ethernet...");
  Ethernet.begin(mac, ip_esp);
  
  delay(2000); 
  Serial.print("✅ IP do ESP32: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Tentando conectar a Additel: ");
  Serial.print(ip_additel);
  Serial.println(":8000");

  // --- ENVIO DO SETPOINT (APENAS UMA VEZ NO ARRANQUE) ---
  enviarSetpointUmaVez();
}

void enviarSetpointUmaVez() {
  Serial.println("\n--- Configurando Setpoint Inicial ---");
  if (client.connect(ip_additel, 8000)) {
    Serial.println("✅ Conectado ao Additel!");
    Serial.println("Enviando Setpoint 50°C...");
    client.print("SOURce:TEMPerature:TARGet 50,1001\r\n");
    delay(200);
    lerResposta("Resposta: ");
    client.stop();
    Serial.println("✅ Setpoint configurado com sucesso.");
  } else {
    Serial.println("❌ ERRO: Nao foi possivel conectar ao Additel.");
    Serial.println("Possíveis causas:");
    Serial.println("- Additel offline");
    Serial.println("- IP incorreto (192.168.0.180)");
    Serial.println("- Porta bloqueada (8000)");
    Serial.println("- W5500 nao esta funcionando");
  }
}

void loop() {
  // Verifica se passaram 30 segundos para pedir a medição
  if (millis() - ultimaExecucao >= intervalo) {
    solicitarMedicao();
    ultimaExecucao = millis(); 
  }
}

void solicitarMedicao() {
  Serial.println("\n--- Lendo Medicao Atual (a cada 30s) ---");
  if (client.connect(ip_additel, 8000)) {
    Serial.println("✅ Conectado ao Additel");
    // Comando para leitura
    client.print("MEASure:SCALar:CONTrol?\r\n");
    
    lerResposta("Valor medido no Additel: ");

    client.stop();
  } else {
    Serial.println("❌ FALHA: Conexao perdida para leitura.");
  }
}
