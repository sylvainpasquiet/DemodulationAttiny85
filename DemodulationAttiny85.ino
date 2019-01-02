#define F_CPU 16000000UL
//#include <avr/io.h>
//#include <util/delay.h>
//#include <avr/interrupt.h>

#define LEDPIN 0  //PB0
#define PINRX  1   //PB1
#define PINTX  2   //PB2

#define TICK_16MS_FILTRE 20

/*volatile byte AncienEtat = HIGH;*/
volatile unsigned long CommunicationActive=0;
         unsigned long OldCommunicationActive=0;
volatile unsigned long CptHigh=0;
volatile unsigned long CptLow=0;
volatile unsigned long Cpt100ms=0;

void setup() 
{
  cli();//disable interrupts during setup
  pinMode(LEDPIN, OUTPUT);
  pinMode(PINTX,  OUTPUT);
  pinMode(PINRX,  INPUT_PULLUP);
  
  //AncienEtat = HIGH;
  
  digitalWrite(LEDPIN, HIGH); //set the LED to off
  //digitalWrite(PINTX,  AncienEtat);
  digitalWrite(PINTX,HIGH);
  
  TCCR0A = 0b00000000; 
  TCCR0B = 0b00000100;  // clk/256 est incrémenté toutes les 16uS
  GTCCR |= (1 << PSR0);
  TCNT0  = 256-1;     // 160us (16us*1) de maintien d'un etat haut minimum     
  TIMSK  = 0b01000010; // TOIE0 OCIE1A 
  
  // 244 count, match every 2.00 secs, 8MHz clock, prescaller @ 16,384
  OCR1A = 24; 
  TCCR1 = 0;  // set all bits in TCCR1 register to 0
  // clear timer on compare match
  TCCR1 |= (1 << CTC1);
  // set CS10, CS11,CS12, and CS13 bits for 16,384 prescaler
  TCCR1 |= (1 << CS10);
  TCCR1 |= (1 << CS11);
  TCCR1 |= (1 << CS12);
  TCCR1 |= (1 << CS13);
  // reset Timer/Counter prescaler
  GTCCR |= (1 << PSR1);
  // Zero (Reset) Timer1 8-bit up counter value
  TCNT1 = 0;
  // enable Timer1 compare interrupt
  TIMSK  = 0b01000010; // TOIE0 OCIE1A
  
  sei();               
}

int main()
{
  setup();
  while(true)
  {
    digitalWrite(LEDPIN, HIGH); //set the LED to off
    MyDelay(200);
    if (OldCommunicationActive!=CommunicationActive)
    {
      OldCommunicationActive=CommunicationActive;
      digitalWrite(LEDPIN, LOW); //set the LED to on
      MyDelay(200);
    }
  }
}

// routine d'interruption du timer
ISR (TIMER0_OVF_vect) 
{  
  cli();
  /*GTCCR |= (1 << PSR0);
  TCNT0  = 256-1;     // 160us (16us*1) de maintien d'un etat haut minimum     
  TIMSK  = 0b01000010; // TOIE0 OCIE1A */
  if (CptLow<TICK_16MS_FILTRE) CptLow++;
  if (((PINB >> PINRX)& 1))
  {
    if (CptHigh<TICK_16MS_FILTRE) CptHigh++;
    else
    {
      CptLow=0;
      digitalWrite(PINTX,HIGH);
    }
  }
  else
  {
    CptHigh=0;
    if (CptLow>=TICK_16MS_FILTRE)
    {
      digitalWrite(PINTX,LOW); 
    }
    CommunicationActive++;
  }
  //GTCCR |= (1 << PSR0);
  TCNT0  = 256-1;     // 160us (16us*1) de maintien d'un etat haut minimum   
  sei();
} 

void MyDelay(unsigned long l_delay)
{
  GTCCR |= (1 << PSR1);
  // Zero (Reset) Timer1 8-bit up counter value
  TCNT1 = 0;
  Cpt100ms=0;
  while(Cpt100ms<l_delay); 
}

ISR(TIMER1_COMPA_vect)
{
  //cli();
  Cpt100ms=Cpt100ms+100;
  //sei();
}
