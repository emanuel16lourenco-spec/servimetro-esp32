/*
 * TEST_ADDITEL_TCP.ino
 * Ficheiro de teste APENAS para comunicação com Additel 875
 * 
 * Objetivo: Verificar se conseguimos conectar e enviar SCPI
 * Sem nenhuma outra funcionalidade (SPI, TFT, Touchscreen, etc)
 * 
 * IP Additel: 192.168.0.180
 * Porta: 8000
 * Comando SCPI: SOURce:TEMPerature:TARGet 50,1001\r\n
 */

#include <WiFi.h>
#include <lwip/sockets.h>

// Credenciais WiFi
const char* ssid = "MEO-85D6E0";
const char* password = "";  // Sem password

// Additel
IPAddress ip_additel(192, 168, 0, 180);
const uint16_t porta_additel = 8000;

// Status
bool wifiConectado = false;
bool additelConectado = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== TEST ADDITEL TCP ===\n");
  Serial.println("Iniciando WiFi...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Esperar WiFi conectar (máx 20 segundos)
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 40) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConectado = true;
    Serial.println("\n✅ WiFi CONECTADO!");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println("\n❌ WiFi NÃO CONECTOU!");
    return;
  }
  
  Serial.println("\nAguardando 2 segundos antes de tentar Additel...");
  delay(2000);
}

void loop() {
  if (!wifiConectado) {
    Serial.println("WiFi não está conectado. Abortando.");
    delay(5000);
    return;
  }
  
  Serial.println("\n=== TENTATIVA DE CONEXÃO ===\n");
  
  // Criar socket
  Serial.println("1. Criando socket TCP...");
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    Serial.println("❌ Erro ao criar socket");
    delay(5000);
    return;
  }
  Serial.println("   ✅ Socket criado");
  
  // Configurar timeouts
  Serial.println("2. Configurando timeouts...");
  struct timeval tv;
  tv.tv_sec = 10;  // 10 segundos
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  Serial.println("   ✅ Timeouts configurados (10s)");
  
  // Preparar endereço
  Serial.println("3. Preparando endereço...");
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(porta_additel);
  servaddr.sin_addr.s_addr = ip_additel;
  Serial.print("   Servidor: ");
  Serial.print(ip_additel);
  Serial.print(":");
  Serial.println(porta_additel);
  
  // Conectar
  Serial.println("4. Conectando ao Additel...");
  int resultado = connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));
  
  if (resultado < 0) {
    Serial.print("❌ FALHA! Erro: ");
    Serial.println(resultado);
    Serial.println("\nPossíveis causas:");
    Serial.println("- Additel offline");
    Serial.println("- IP incorreto");
    Serial.println("- Porta bloqueada por firewall");
    Serial.println("- Additel não aceita TCP na porta 8000");
    close(sock);
    Serial.println("\nTentando novamente em 5 segundos...\n");
    delay(5000);
    return;
  }
  
  Serial.println("   ✅ CONECTADO AO ADDITEL!");
  additelConectado = true;
  
  // Enviar comando SCPI
  Serial.println("5. Enviando comando SCPI...");
  const char* comando = "SOURce:TEMPerature:TARGet 50,1001\r\n";
  Serial.print("   Comando: ");
  Serial.println(comando);
  
  int sent = send(sock, (const char*)comando, strlen(comando), 0);
  
  if (sent < 0) {
    Serial.print("❌ Erro ao enviar: ");
    Serial.println(sent);
  } else {
    Serial.print("   ✅ Enviados ");
    Serial.print(sent);
    Serial.println(" bytes");
  }
  
  // Aguardar resposta
  Serial.println("6. Aguardando resposta (até 10s)...");
  char buffer[512];
  int bytes = recv(sock, (unsigned char*)buffer, sizeof(buffer) - 1, 0);
  
  if (bytes > 0) {
    buffer[bytes] = '\0';
    Serial.print("   ✅ Resposta recebida (");
    Serial.print(bytes);
    Serial.println(" bytes):");
    Serial.print("   ");
    Serial.println(buffer);
  } else if (bytes == 0) {
    Serial.println("   ⚠️ Conexão fechada pelo Additel");
  } else {
    Serial.println("   ⚠️ Timeout ou erro ao receber");
  }
  
  // Fechar
  close(sock);
  Serial.println("\n✅ Socket fechado");
  
  Serial.println("\n=== AGUARDANDO 10 SEGUNDOS PARA PRÓXIMA TENTATIVA ===\n");
  delay(10000);
}
