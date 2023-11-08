#include "contiki.h"
#include "os/sys/rtimer.h"
#include "lib/aes-128.h"
#include "sys/rtimer.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>

#define TRANSFER_INTERVAL (5 * CLOCK_SECOND)

static uint8_t encryption_key[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

PROCESS(assignment_final, "AES Encryption and Send Process");
AUTOSTART_PROCESSES(&assignment_final);

PROCESS_THREAD(assignment_final, ev, data)
{
    static struct etimer et;
    static rtimer_clock_t start_time, end_time, duration;
    PROCESS_BEGIN();    
    
    //initialise rtimer
    rtimer_init();

    // Example plaintext "Hello, World!" with PKCS#7 padding
    static uint8_t plaintext[16] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    //size_t len = strlen((char*)plaintext);
    //// Apply zero padding
    //for (size_t i = len; i < 16; i++) {
    //  plaintext[i] = 0;
    //}
    

    // Initialize AES
    AES_128.set_key(encryption_key);    
    
    while(1) {
        // Wait for sensor data or command to be available
        //PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && data != NULL);
        //uint8_t *plaintext = (uint8_t *)data;
        //uint8_t ciphertext[16];

        // Set a timer to pace the sampling
        etimer_set(&et, TRANSFER_INTERVAL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        // Start time measurement
        start_time = RTIMER_NOW();

        // Encrypt plaintext
        AES_128.encrypt(plaintext);

        // End time measurement
        end_time = RTIMER_NOW();

        // Calculate duration
        duration = end_time - start_time;

        printf("Encryption time: %u ticks\n", (unsigned)duration);

        // Print out encryption time in microseconds
        printf("Encryption time: %u ticks, %u microseconds\n", duration, (unsigned int)((uint64_t)duration * 1000000 / RTIMER_SECOND));

        // Transmit ciphertext - replace with actual transmission code
        printf("Encrypted data sent: ");
        for(int i = 0; i < 16; i++) {
          printf("%02x", plaintext[i]);
        }
        printf("\n");


        // Decrypt ciphertext
        start_time = RTIMER_NOW();
        AES_128.encrypt(plaintext);
        end_time = RTIMER_NOW();

        duration = end_time - start_time;
        
        printf("Decrypted data recievied: ");
        for(int i = 0; i < 16; i++) {
          if(plaintext[i] != 0x00) {
            printf("%c", plaintext[i]);
          }
        }
        printf("\n");

        printf("Decryption time: %u ticks, %u microseconds\n", duration, (unsigned int)((uint64_t)duration * 1000000 / RTIMER_SECOND));


    }

  PROCESS_END();
}
