const int rxLedIn = 2;
const int txLedIn = 3;
const int lnaEnableOut = 5;
const int paEnableOut = 4;
const int antGroundOut = 8;
const int antTrxSwitch1Out = 6;
const int antTrxSwitch2Out = 7;
const int lnaDiagnosticEnable = 10;
const int paDiagnosticEnable = 9;
const int lnaCurrentIn = A1;
const int paCurrentIn = A0;

// Maximum operation time of TE HF3 relays is 5ms.
// Maximum release time is 6ms.
// Exit time is mostly impacted by how long the LNA/PA take to "wind down" after power supply is cut.
const unsigned long rxEnterMillis = 25;
const unsigned long rxExitMillis = 15;
const unsigned long txEnterMillis = 15;
const unsigned long txExitMillis = 100;
const unsigned long minInvalidMillis = 50;

enum state_t {
  IDLE,
  RX_ENTER,
  RX,
  RX_EXIT,
  TX_ENTER,
  TX,
  TX_EXIT,
  INVALID
};

state_t state = IDLE;
state_t old_state = IDLE;
bool state_changed = false;

unsigned long stateEnter = millis();

void setup() {
  pinMode(lnaEnableOut, OUTPUT);
  pinMode(paEnableOut, OUTPUT);
  pinMode(antGroundOut, OUTPUT);
  pinMode(antTrxSwitch1Out, OUTPUT);
  pinMode(antTrxSwitch2Out, OUTPUT);

  pinMode(rxLedIn, INPUT);
  pinMode(txLedIn, INPUT);

  Serial.begin(115200);
}

void loop() {
  bool rx = !digitalRead(rxLedIn);
  bool tx = !digitalRead(txLedIn);
  old_state = state;

  switch (state) {
    case IDLE:
      digitalWrite(lnaEnableOut, LOW);
      digitalWrite(paEnableOut, LOW);
      digitalWrite(antGroundOut, LOW);
      digitalWrite(antTrxSwitch1Out, LOW);
      digitalWrite(antTrxSwitch2Out, LOW);

      if (rx) state = RX_ENTER;
      else if (tx) state = TX_ENTER;
      break;
    case RX_ENTER:
      digitalWrite(lnaEnableOut, LOW);
      digitalWrite(paEnableOut, LOW);
      digitalWrite(antGroundOut, HIGH);
      digitalWrite(antTrxSwitch1Out, LOW);
      digitalWrite(antTrxSwitch2Out, LOW);

      if (state_changed) stateEnter = millis();

      if (tx) state = RX_EXIT;
      else if (!rx) state = RX_EXIT;
      else if (millis() - stateEnter > rxEnterMillis) state = RX;

      break;
    case RX:
      digitalWrite(lnaEnableOut, HIGH);
      digitalWrite(paEnableOut, LOW);
      digitalWrite(antGroundOut, HIGH);
      digitalWrite(antTrxSwitch1Out, LOW);
      digitalWrite(antTrxSwitch2Out, LOW);

      if (tx) state = RX_EXIT;
      else if (!rx) state = RX_EXIT;

      break;
    case RX_EXIT:
      digitalWrite(lnaEnableOut, LOW);
      digitalWrite(paEnableOut, LOW);
      digitalWrite(antGroundOut, HIGH);
      digitalWrite(antTrxSwitch1Out, LOW);
      digitalWrite(antTrxSwitch2Out, LOW);

      if (state_changed) stateEnter = millis();

      if (tx && (millis() - stateEnter > rxExitMillis)) state = TX_ENTER;
      else if (rx && (millis() - stateEnter > rxEnterMillis)) state = RX;
      else if (!rx && (millis() - stateEnter > rxExitMillis)) state = IDLE;

      break;
    case TX_ENTER:
      digitalWrite(lnaEnableOut, LOW);
      digitalWrite(paEnableOut, LOW);
      digitalWrite(antGroundOut, HIGH);
      digitalWrite(antTrxSwitch1Out, HIGH);
      digitalWrite(antTrxSwitch2Out, HIGH);

      if (state_changed) stateEnter = millis();

      if (!tx) state = TX_EXIT;
      else if (millis() - stateEnter > txEnterMillis) state = TX;

      break;
    case TX:
      digitalWrite(lnaEnableOut, LOW);
      digitalWrite(paEnableOut, HIGH);
      digitalWrite(antGroundOut, HIGH);
      digitalWrite(antTrxSwitch1Out, HIGH);
      digitalWrite(antTrxSwitch2Out, HIGH);

      if (!tx) state = TX_EXIT;

      break;
    case TX_EXIT:
      digitalWrite(lnaEnableOut, LOW);
      digitalWrite(paEnableOut, LOW);
      digitalWrite(antGroundOut, HIGH);
      digitalWrite(antTrxSwitch1Out, HIGH);
      digitalWrite(antTrxSwitch2Out, HIGH);

      if (state_changed) stateEnter = millis();

      if (tx && (millis() - stateEnter > txEnterMillis)) state = TX;
      else if (!tx && (millis() - stateEnter > txExitMillis)) state = IDLE;

      break;
    case INVALID:
    default:
      // Switch to "TX Mode" but don't enable the PA
      digitalWrite(lnaEnableOut, LOW);
      digitalWrite(paEnableOut, LOW);
      digitalWrite(antGroundOut, HIGH);
      digitalWrite(antTrxSwitch1Out, HIGH);
      digitalWrite(antTrxSwitch2Out, HIGH);

      if (state_changed) stateEnter = millis();

      if ((!(rx && tx)) && (millis() - stateEnter > minInvalidMillis)) state == IDLE;

      break;
  }

  state_changed = old_state != state;
}
