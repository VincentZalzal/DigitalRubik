#include "rings.h"
#include "avr_specific.h"
#include "../Cube/controls.h"

namespace
{

// D�lai d'attente entre le changement du registre � d�calage et la lecture (en ms)
#define SETTLE_DELAY 1
// Seuil pour le registre � d�calage
#define THRESHOLD 32

// M�moire qui contient les 4 derni�res valeurs lues pour chaque anneau
uint8_t g_InputRaw[Controls::NumSensors / 2] = { 0 };

// D�cale 1 bit dans le registre � d�calage. Les bits sont d�cal�s
// du moins significatif vers le plus significatif.
void Shift( void )
{
	// Horloge s�rielle
	SH_REG_PORT |=  _BV( SH_REG_SRCK );
	SH_REG_PORT &= ~_BV( SH_REG_SRCK );

	// Horloge de latch
	SH_REG_PORT |=  _BV( SH_REG_RCK );
	SH_REG_PORT &= ~_BV( SH_REG_RCK );
}

// Lecture de l'�tat du prochain anneau
bool ReadOnce( void )
{
	// D�caler le "1" pour lire le prochain anneau
	Shift();

	// Remise � 0 de SER_IN
	SH_REG_PORT &= ~_BV( SH_REG_SER_IN );

	// D�lai requis avant de pouvoir faire une lecture
	_delay_ms( SETTLE_DELAY );

	// D�marre une conversion
	ADCSRA |= _BV( ADSC );

	// Attend la fin de la conversion
	loop_until_bit_is_set( ADCSRA, ADIF );
	//while ( bit_is_clear( ADCSRA, ADIF ) ) {}

	// Lecture du ADC
	uint8_t Res = ADCH;

	// Remet ADIF � 0
	ADCSRA |= _BV( ADIF );

	return Res > THRESHOLD;
}

// Lecture d'une paire d'anneaux
uint8_t ReadPair( void )
{
	uint8_t Res = 0;

	if (ReadOnce()) Res |= _BV( 0 );
	if (ReadOnce()) Res |= _BV( 4 );

	return Res;
}

// Lecture de tous les anneaux, avant le debouncing.
void ReadRaw( void )
{
	// S�rialiser un 1
	SH_REG_PORT |=  (1 << SH_REG_SER_IN);

	for (uint8_t i = 0; i < Controls::NumSensors / 2; i++)
	{
		// D�caler les bits pour conserver en m�moire les 3 derniers bits lus
		g_InputRaw[i] <<= 1;
		// Le d�calage va mettre le bit 0 � 0, mais il faut aussi mettre le bit 4 � 0
		g_InputRaw[i]  &= ~_BV( 4 );
		// Remplacer les bits 0 et 4 par les nouvelles lectures
		g_InputRaw[i]  |= ReadPair();
	}

	// Ajouter un "0" pour s'assurer qu'aucun anneau ne re�oit des
	// un signal � 1 plus longtemps que les autres.
	Shift();
}

void UpdateCounter(int8_t* pSensorCounter, bool SensorIsOn)
{
	if (SensorIsOn)
	{
		if (*pSensorCounter <= 0)
			*pSensorCounter = 1;
		else if (*pSensorCounter < 127)
			++(*pSensorCounter);
	}
	else
	{
		if (*pSensorCounter >= 0)
			*pSensorCounter = -1;
		else if (*pSensorCounter > -128)
			--(*pSensorCounter);
	}
}

// Fonction � double utilit�:
// 	- Applique un filtre anto-rebond sur les lectures des anneaux
//	- Ordonne les bits dans la variable finale
void Debounce( void )
{
	const uint8_t* pInputRaw  = g_InputRaw;
	const uint8_t* const pEnd = pInputRaw + Controls::NumSensors / 2;

	int8_t* pSensorCounter = Controls::g_SensorCounters;

	do
	{
		UpdateCounter(pSensorCounter++, (*pInputRaw & 0x0F) != 0);
		UpdateCounter(pSensorCounter++, (*pInputRaw & 0xF0) != 0);
	} while (++pInputRaw != pEnd);
}

}

namespace Rings
{

// Initialisation des ressources utilis�es pour la d�tection des doigts.
void Init( void )
{
	// Initialisation des IO utilis�s par le registre � d�calage (output clear)
	SH_REG_DDR  |=  ( _BV( SH_REG_SER_IN ) | _BV( SH_REG_SRCK ) | _BV( SH_REG_RCK ) );
	SH_REG_PORT &= ~( _BV( SH_REG_SER_IN ) | _BV( SH_REG_SRCK ) | _BV( SH_REG_RCK ) );

	// Pr�paration du ADC
	// R�f�rence de Vcc utilis�e par d�faut, prescaler /8
	ADMUX = _BV(ADPS1) | _BV(ADPS0);
	// (1 << ADEN) : active le ADC
	ADCSRA = _BV(ADEN);
	// (1 << ADLAR) : Ajust� � gauche
	ADCSRB = _BV(ADLAR);
	// R�duire la consommation
	DIDR0 = _BV(ADC0D);
	
	Reset();
}

// Lecture de tous les anneaux. Fonction devant �tre appel�e de l'externe.
void Read( void )
{
	ReadRaw();
	Debounce();
}

// Remet toutes les valeurs lues � 0.
void Reset()
{
	for (uint8_t i = 0; i < Controls::NumSensors / 2; i++)
		g_InputRaw[i] = 0;
}

}
