#define swPin 3 //��� ��������� ���������
#define rxPin 2 //��� ���������
//#define led 13 //��� ���������
#define lock 4
#define drUnlock 5
#define pasUnlock  6
#define indicator 13
#define horn 7
#define IMPULS_TIME 200 // ������������ ��������� ����� ������� � ����������� �� ������ �������� - ���� ������ 200 ���
#define DOOR_MOTOR_TIME 300
#define INDICATION_TIME 100

volatile unsigned long prevtime;
volatile unsigned int lolen, hilen, state;
volatile static byte cameCounter = 0;    // count of bits stored
volatile static long cameCode = 0;       // code itself
volatile static long code = 0;
static int DoorPins[] = { lock, drUnlock, pasUnlock };
static int opIndex = 0;
long time;
bool mute = true;

boolean CheckValue(unsigned int base, unsigned int value)
{
	return ((value == base) || ((value > base) && ((value - base) < IMPULS_TIME)) || ((value < base) && ((base - value) < IMPULS_TIME)));
}

void grab()
{
	state = digitalRead(rxPin);
	if (state == HIGH)
		lolen = micros() - prevtime;
	else
		hilen = micros() - prevtime;
	prevtime = micros();

	if (state == LOW)
	{
		// ������������������ �����������
		if (CheckValue(320, hilen) && CheckValue(640, lolen)) // valid 1
		{
			cameCode = (cameCode << 1) | 1;
			cameCounter++;
		}
		else if (CheckValue(640, hilen) && CheckValue(320, lolen)) // valid 0
		{
			cameCode = (cameCode << 1) | 0;
			cameCounter++;
		}
		else cameCounter = 0;
	}
	else
		if (lolen > 1000 &&
			(cameCounter == 12 || cameCounter == 13) &&
			((cameCode & 0xfff) != 0xfff))
		{
			code = code == cameCode & 0xfff ? 0 : cameCode & 0xfff;
			cameCounter = 0;
			cameCode = 0;
		}
}

void Indication(bool beep)
{
	if (mute)
		beep = false;
	digitalWrite(indicator, HIGH);
	if (beep)
		digitalWrite(horn, LOW);

	delay(INDICATION_TIME);

	digitalWrite(indicator, LOW);
	if (beep)
		digitalWrite(horn, HIGH);
}

int DelayTime(int time)
{
	return time > 0 ? time : 0;
}

void DoorOperation(int i)
{
	int time = 0;
	bool beep = i == 2 ? false : true;

	digitalWrite(DoorPins[i], LOW);
	Indication(beep);
	time += INDICATION_TIME;
	if (i != 0)
	{
		delay(time);
		Indication(beep);
		time += 2 * INDICATION_TIME;
	}
	time = DOOR_MOTOR_TIME > time ? DOOR_MOTOR_TIME - time : 0;
	delay(time);

	digitalWrite(DoorPins[i], HIGH);

	time = IMPULS_TIME > max(INDICATION_TIME, DOOR_MOTOR_TIME) ? IMPULS_TIME - max(INDICATION_TIME, DOOR_MOTOR_TIME) : 0;
	delay(time > 0 ? time : 0);
}

void setup()
{
	pinMode(rxPin, INPUT);
	// ������ ������������� ��� ��������� ��������� � 0 � �� ������� ��� ������
	pinMode(swPin, OUTPUT);
	digitalWrite(swPin, LOW);

	pinMode(drUnlock, OUTPUT);
	digitalWrite(drUnlock, HIGH);

	pinMode(pasUnlock, OUTPUT);
	digitalWrite(pasUnlock, HIGH);

	pinMode(lock, OUTPUT);
	digitalWrite(lock, HIGH);

	pinMode(horn, OUTPUT);
	digitalWrite(horn, HIGH);

	pinMode(indicator, OUTPUT);
	digitalWrite(indicator, LOW);

	Serial.begin(9600);  // ��� ��������� ���� ��������
	Serial.println("Came started");

	attachInterrupt(0, grab, CHANGE); // �������� �������� �� ������ �������� - ���� � ��� ������ �������, �� ����� ���� �� 1, � 0!
	interrupts();


}

void loop()
{
	if (code != 0)
		if (code == 1234)
		{
			Serial.println(code);
			if (opIndex == 1)
				time = millis();
			if (opIndex == 2 && millis() - time > 3000)
				opIndex++;
			if (opIndex == 3)
				opIndex = 0;
			DoorOperation(opIndex);
			opIndex++;
			
			code = 0;
		}
}
