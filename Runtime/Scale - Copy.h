/*
 * Scale.h
 *
 * Created: 17.04.2020 11:14:47
 * Author: bomarius
 *
 * abridged from HX711 library for Arduino
 * https://github.com/bogde/HX711
 *
 * MIT License
 * (c) 2018 Bogdan Necula
  */ 


#ifndef SCALE_H_
#define SCALE_H_

#include "Basics.h"
#include "DigiPort.h"

// Eine µs entspricht 16 Takten bei 16MHz. Nop benoetigt einen Takt
#define NOP  asm volatile("nop")
#define DELAY_1US  {NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;}

#define MAX_NUM_OF_SENSORS       (4)          // Maximale Anzahl der Sensoren, die zu einer Waage gehoeren und am gleichen Port liegen koennen
#define SENSOR_PORT_DDR_PATTERN  (0b10101010) // abwechselnd beginnend beim MSB: CLK (out==1), DATA (in==0)
#define KANAL_A_GAIN_128         (1)
#define KANAL_B_GAIN_32          (2)
#define KANAL_A_GAIN_64          (3)

// Masken fuer die DATA Pins
#define CH3_DM  (0b01000000)
#define CH2_DM  (0b00010000)
#define CH1_DM  (0b00000100)
#define CH0_DM  (0b00000001)
// Masken fuer die CLK Pins
#define CH3_CM  (0b10000000)
#define CH2_CM  (0b00100000)
#define CH1_CM  (0b00001000)
#define CH0_CM  (0b00000010)

class Scale {
	private:
		DigiPortRaw port;
        const uint8_t num_of_sensors;     // an einem Port koennen bis zu MAX_NUM_OF_SENSORS Sensoren angeschlossen sein
		float   scaling_factor;           // Skalierung des Summenwerts auf eine Ausgabeeinheit (typ. kG)
		uint8_t chan_gain;                // Gemeinsamer gain Wert (Kanalwahl und Verstaerkungsfaktor)
		uint8_t port_data_mask;           // Maske fuer die Selektion der benutzten DATA Pins des Ports
		uint8_t port_clk_mask;            // Maske fuer die Selektion der benutzen CLK Pins des Ports
		uint8_t data[MAX_NUM_OF_SENSORS][3]; // 3 x 8 Bit shift-in Variablen pro Kanal
		int32_t value[MAX_NUM_OF_SENSORS];   // 32 Bit raw data Wert pro Kanal
		int32_t tare[MAX_NUM_OF_SENSORS];    // Tara Wert pro Kanal


		
	public:
			// Warte bis alle angeschlossenen HX711 Messverstaerker bereit sind und liefere
			// eine Messung von allen angeschlossenen Sensoren parallel.
			void read_raw();
	
		Scale (uint8_t p_handle, uint8_t n_o_s = 4, uint8_t cg = KANAL_A_GAIN_128) : 
		          port(p_handle, SENSOR_PORT_DDR_PATTERN, SET_ACTIVE_LOW),
				  num_of_sensors(n_o_s),
				  chan_gain (cg)
		{
			scaling_factor = 1.f;
			tare[0] = 0; tare[1] = 0; tare[2] = 0; tare [3] = 0;
			// Setze die Masken so, dass nur die benutzen Pins selektiert werden:
			port_clk_mask = 0;
			port_data_mask = 0;
			switch(n_o_s){
				case 4: port_data_mask |= CH3_DM; port_clk_mask |= CH3_CM;
				case 3: port_data_mask |= CH2_DM; port_clk_mask |= CH2_CM;
				case 2: port_data_mask |= CH1_DM; port_clk_mask |= CH1_CM;
				case 1:	port_data_mask |= CH0_DM; port_clk_mask |= CH0_CM;
			}
			port.on(port_clk_mask); // power up der Messverstaerker
		}
		
		// set the gain factor and choose channel; takes effect only after a call to read()
		// channel A can be set for a 128 or 64 gain; channel B has a fixed 32 gain
		// depending on the parameter, the channel is also set to either A or B
		void set_gain(uint8_t cg=KANAL_A_GAIN_128) {chan_gain=cg;}
			
		void set_scaling_factor(float sf = 1.f) {scaling_factor = sf;}	

		// Warte bis alle angeschlossenen HX711 Messverstaerker bereit sind.
		// Aus dem Datenblatt: When output data is not ready for retrieval, digital output (DATA)
		// is high and CLK should be low. When DATA goes to low, it indicates data is ready for 
		// retrieval. Wir pruefen das fuer alle angeschlossenen Verstaerker parallel.
		bool sensors_are_ready() { return 0 != port.read_raw(port_data_mask);}
		
		// Bestimme den individuellen Tara Wert jedes Kanals (da die Weret des HX711 auch negativ
		// sein koennen, kann man keine Summe der Tara Werte ueber alle Kanaele verwenden.
		void set_tare(uint8_t times=5);
		
		//int32_t get_raw_total_value() ;
		
		void get_unit_values(int16_t*v0, int16_t*v1, int16_t*v2, int16_t*v3);
		void get_tare_values(int16_t*v0, int16_t*v1, int16_t*v2, int16_t*v3);
		int32_t get_total_value();
		int32_t get_unit_total_value() {return get_total_value() / scaling_factor;}
			
		void power_down(){
			port.on(port_clk_mask); 
			for (uint8_t i = 70; i > 0; i--) DELAY_1US
			port.off(port_clk_mask);};
			
		void power_up(){port.on(port_clk_mask);};
};

#endif /* SCALE_H_ */