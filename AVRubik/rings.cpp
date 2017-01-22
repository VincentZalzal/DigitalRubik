#include "rings.h"
#include "avr_specific.h"

namespace Rings
{

// Délai d'attente entre le changement du registre à décalage et la lecture (en ms)
#define SETTLE_DELAY 1
// Seuil pour le registre à décalage
#define THRESHOLD 32
// Nombre d'anneaux à lire. Doit être un multiple de 8.
#define NB_RINGS 24

// Mémoire qui contient les 4 dernières valeurs lues pour chaque anneau
uint8_t g_InputRaw[NB_RINGS / 2] = { 0 };
// État des anneaux, 1 bit par anneau
uint8_t g_Status[NB_RINGS / 8]   = { 0 };

// Initialisation des ressources utilisées pour la détection des doigts.
void Init( void )
{
	// Initialisation des IO utilisés par le registre à décalage (output clear)
	SH_REG_DDR  |=  ( _BV( SH_REG_SER_IN ) | _BV( SH_REG_SRCK ) | _BV( SH_REG_RCK ) );
	SH_REG_PORT &= ~( _BV( SH_REG_SER_IN ) | _BV( SH_REG_SRCK ) | _BV( SH_REG_RCK ) );

	// Préparation du ADC
	// Référence de Vcc utilisée par défaut
	ADMUX = 0;
	// (1 << ADEN) : active le ADC
	ADCSRA = (1 << ADEN);
	// (1 << ADLAR) : Ajusté à gauche
	ADCSRB = (1 << ADLAR);
	// Réduire la consommation
	DIDR0 |= (1 << ADC1D);
}

// Décale 1 bit dans le registre à décalage. Les bits sont décalés
// du moins significatif vers le plus significatif.
void Shift( void )
{
	// Horloge sérielle
	SH_REG_PORT |=  _BV( SH_REG_SRCK );
	SH_REG_PORT &= ~_BV( SH_REG_SRCK );

	// Horloge de latch
	SH_REG_PORT |=  _BV( SH_REG_RCK );
	SH_REG_PORT &= ~_BV( SH_REG_RCK );
}

// Lecture de l'état du prochain anneau
bool ReadOnce( void )
{
	// Décaler le "1" pour lire le prochain anneau
	Shift();

	// Remise à 0 de SER_IN
	SH_REG_PORT &= ~_BV( SH_REG_SER_IN );

	// Délai requis avant de pouvoir faire une lecture
	_delay_ms( SETTLE_DELAY );

	// Démarre une conversion
	ADCSRA |= _BV( ADSC );

	// Attend la fin de la conversion
	loop_until_bit_is_set( ADCSRA, ADIF );
	//while ( bit_is_clear( ADCSRA, ADIF ) ) {}

	// Lecture du ADC
	uint8_t Res = ADCH;

	// Remet ADIF à 0
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
	// Sérialiser un 1
	SH_REG_PORT |=  (1 << SH_REG_SER_IN);

	for (uint8_t i = 0; i < NB_RINGS / 2; i++)
	{
		// Décaler les bits pour conserver en mémoire les 3 derniers bits lus
		g_InputRaw[i] <<= 1;
		// Le décalage va mettre le bit 0 à 0, mais il faut aussi mettre le bit 4 à 0
		g_InputRaw[i]  &= ~_BV( 4 );
		// Remplacer les bits 0 et 4 par les nouvelles lectures
		g_InputRaw[i]  |= ReadPair();
	}

	// Ajouter un "0" pour s'assurer qu'aucun anneau ne reçoit des
	// un signal à 1 plus longtemps que les autres.
	Shift();
}

// Fonction à double utilité:
// 	- Applique un filtre anto-rebond sur les lectures des anneaux
//	- Ordonne les bits dans la variable finale
void Debounce( void )
{
	uint8_t j = 0;

	for (uint8_t i = 0; i < NB_RINGS / 8; i++)
	{
		g_Status[i] = 0;

		for (uint8_t tmp = 0; tmp < 4; tmp++)
		{
			g_Status[i] >>= 1;
			if (( g_InputRaw[j] & 0x0F ) != 0) g_Status[i] |= _BV( 7 );

			g_Status[i] >>= 1;
			if (( g_InputRaw[j] & 0xF0 ) != 0) g_Status[i] |= _BV( 7 );

			++j;
		}
	}
}

// Lecture de tous les anneaux. Fonction devant être appelée de l'externe.
void Read( void )
{
	ReadRaw();
	Debounce();
}

}
