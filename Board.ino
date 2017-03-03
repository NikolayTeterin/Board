#define swPin 5 //пин включения приемника
#define rxPin 2 //пин приемника

void grab();

void setup()
{
	pinMode(rxPin, INPUT);
	// всегда устанавливаем пин включения приемника в 0 и не трогаем его больше
	pinMode(swPin, OUTPUT);
	digitalWrite(swPin, LOW);

	Serial.begin(9600);  // тут поставьте свою скорость
	Serial.println("Came started");

	attachInterrupt(0, grab, CHANGE); // обратите внимание на первый параметр - если у вас другая ардуина, он может быть не 1, а 0!
	interrupts();
}

volatile static long code = 0;
long lastCode = 0;
int i, j;
long variant;
long someCode = 1234;
void SendCame4(long Code);

void loop()
{

}

// ВЕСЬ КОД НИЖЕ - ПРИЕМ

#define MAX_DELTA 200
// длительность импульсов может плавать в зависимости от заряда батареии - макс допуск 200 мкс
boolean CheckValue(unsigned int base, unsigned int value)
{
	return ((value == base) || ((value > base) && ((value - base) < MAX_DELTA)) || ((value < base) && ((base - value) < MAX_DELTA)));
}
volatile unsigned long prevtime;
volatile unsigned int lolen, hilen, state;
volatile static byte cameCounter = 0;    // count of bits stored
volatile static long cameCode = 0;       // code itself

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
		// последовательность закончилась
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
			code = cameCode & 0xfff;
			cameCounter = 0;
			cameCode = 0;
		}
	if (code != 0)
	{
		if (code == lastCode)
		{
			Serial.print("*");
			code = 0;
		}
		else
		{
			Serial.println(code);
			lastCode = code;
			code = 0;
		}
	}
}