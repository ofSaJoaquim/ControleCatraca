#include <Wiegand.h>
WIEGAND wg;

//Saídas
const unsigned int estatus = 12;
const unsigned int sirene = 10;
const unsigned int  saida = 9;

//Constantes de tempo e comprimento
const unsigned long tempoEspera = 30000;
const unsigned int tempoBaseSirene = 3000;
const unsigned int tempoBaseSaida = 1000;
const unsigned int comprimentoFila = 50;
const unsigned int vezesLedEstatus = 5;
const unsigned int piscaWG = 200;
const unsigned int piscaNormal = 600;
bool modoTeste = true;

unsigned int tempoLedEstatus = 0;
unsigned int tempoLedSelec = piscaNormal;
unsigned int numeroLedPisca = 0;
bool ledEstadoOcupado = false;
bool ledEstado = false;



//Variaveis globais de controle
unsigned int fila[comprimentoFila];
unsigned long tempoSirene = 0;
unsigned long tempoSaida = 0;
unsigned long tempoAtual = 0;
unsigned int proximoLivre = 0;
unsigned int proximoRetira = 0;

int r = 0;

void piscaEstatus(unsigned long tempo, unsigned int modo) {
  if (ledEstadoOcupado) {
    tempoLedSelec = modo;
    numeroLedPisca = vezesLedEstatus;
    ledEstadoOcupado=false;
  }
  if (tempo > tempoLedEstatus) {
    tempoLedEstatus = tempo + tempoLedSelec;
    if (ledEstado)digitalWrite(estatus, HIGH);
    else digitalWrite(estatus, LOW);
    ledEstado = !ledEstado;
    if (numeroLedPisca > 0) {
      numeroLedPisca--;
    }
    else {
      tempoLedSelec = piscaNormal;
      ledEstadoOcupado = true;
    }

  }
}

void zeraTempos(unsigned long tempo) {
  if (tempo >= 0xffffffff ) {
    tempoSirene = 0;
    tempoSaida = 0;
    tempoAtual = 0;
    Serial.println("Overflow Millis()");
  }
}

void acionaSaida(unsigned long tempo) {
  tempoSaida = tempo + tempoBaseSaida;
  digitalWrite(saida, HIGH);
}

void desligaSaida(unsigned long tempo) {
  if (tempo > tempoSaida ) {
    digitalWrite(saida, LOW);
  }

}

void acionaSirene(unsigned long tempo) {
  tempoSirene = tempo + tempoBaseSirene;
  digitalWrite(sirene, HIGH);
}

void desligaSirene(unsigned long tempo) {
  if (tempo > tempoSirene) {
    digitalWrite(sirene, LOW);
  }
}

void adicionaId( int id) {
  fila[proximoLivre] = id;

  if (proximoLivre < comprimentoFila - 1) {

    proximoLivre++;
  }
  else {
    if (proximoRetira == proximoLivre)proximoRetira = 0;
    proximoLivre = 0;
  }
}

void removeId(unsigned long tempo) {
  if (tempo >= tempoAtual + tempoEspera) {

    tempoAtual = tempo + tempoEspera;

    if (fila[proximoRetira] > 0) {



      fila[proximoRetira] = 0;
      if (proximoRetira < comprimentoFila - 1) {
        proximoRetira++;
      }
      else {
        proximoRetira = 0;
      }

    }
  }
}

void testaId( int id, unsigned long tempo) {
  bool teste = false;
  for (int i = 0; i < comprimentoFila; i++) {

    if (fila[i] == id) {

      teste = true;
      break;
    }
  }
  if (teste) {
    Serial.print("Bloqueado---");
    Serial.println(id);
    acionaSirene(tempo);
  }
  else {
    Serial.print("Liberado---");
    Serial.println(id);
    acionaSaida(tempo);
    adicionaId(id);


  }
}

int testeEntrada() {
  if (Serial.available() > 0) {
    int incomingByte = Serial.read();
    return incomingByte - 48;
  }
}

int leitura;
void setup() {

  Serial.begin(9600);
  wg.begin();
  pinMode(estatus, OUTPUT);
  pinMode(sirene, OUTPUT);
  pinMode(saida, OUTPUT);

  Serial.println("Placa comando para catraca");

}

void loop() {
  unsigned long tempo = millis();
  int leitura = 0;
  if (modoTeste == true) {
    int e = 0;
    e = testeEntrada();
    if (r != e) {
      r = e;
      if (e > 0 && e < 58) {
        leitura = e;
        ledEstadoOcupado=true;
        piscaEstatus(tempo, piscaWG);
      }
    }
  }

  else if (wg.available())
  {
    leitura = wg.getCode();
    piscaEstatus(tempo, piscaWG);
  }

  piscaEstatus(tempo, piscaNormal);


  if (leitura > 0)testaId(leitura, tempo);
  desligaSaida(tempo);
  desligaSirene(tempo);
  removeId(tempo);
  zeraTempos(tempo);









}







