#include "contiki.h"
#include "lib/aes-128.h"
#include "dev/radio/cc2420/cc2420.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>

#define TRANSFER_INTERVAL (5 * CLOCK_SECOND)
#define CHANNEL 11 // 26 channels in the 2.4 GHz band, numbered 11 to 26

void flip_bit(uint8_t *data, int position); // Function to flip a bit in a byte array, used for bit error rate testing

//Decryption: http://aes.online-domain-tools.com/
//Decryption key: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
static uint8_t encryption_key[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

PROCESS(assignment_final, "AES Encryption and Send Process");
AUTOSTART_PROCESSES(&assignment_final);

PROCESS_THREAD(assignment_final, ev, data)
{
    static struct etimer et;
    static rtimer_clock_t start_time, end_time, duration;
    PROCESS_BEGIN();

    cc2420_init();

    cc2420_set_channel(CHANNEL);
   
    // Initialize AES
    AES_128.set_key(encryption_key);    
    
    while(1) {
        // Set a timer to pace the sampling
        etimer_set(&et, TRANSFER_INTERVAL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        
        //zero padding, 2 padding bytes, 1 byte for length
        uint8_t plaintext[16] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0x00, 0x00, 0x02};

        // Start time measurement
        start_time = RTIMER_NOW();

        // Encrypt plaintext
        AES_128.encrypt(plaintext); // Ensure that encrypt function takes two arguments

        // End time measurement
        end_time = RTIMER_NOW();

        // Calculate duration
        duration = end_time - start_time;

        printf("Encryption time: %u ticks\n", (unsigned)duration);

        // Transmit ciphertext - replace with actual transmission code
        printf("Encrypted data sent: ");
        for(int i = 0; i < 16; i++) {
          printf("%02x", plaintext[i]);
        }
        printf("\n");

        //flip_bit(plaintext, 0); // Flip the first bit of the ciphertext
        flip_bit(plaintext, 1); // Flip the second bit of the ciphertext

        //calculate bit error rate
        //int bit_errors = 0;
        //for(int i = 0; i < 16; i++) {
        //  for(int j = 0; j < 8; j++) {
        //    if((plaintext[i] & (1 << j)) != 0) {
        //      bit_errors++;
        //    }
        //  }
        //}
        //printf("Bit error rate: %d/%d\n", bit_errors, 128);
    }

  PROCESS_END();
}


void flip_bit(uint8_t *data, int position) {
    int byte_pos = position / 8; // Find the byte in which the bit is located
    int bit_pos = position % 8;  // Find the position of the bit in that byte
    data[byte_pos] ^= (1 << bit_pos); // XOR with 1 shifted to the correct bit position flips the bit
}
