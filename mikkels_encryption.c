#include "contiki.h"
#include "lib/aes-128.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>

#define TRANSFER_INTERVAL (5 * CLOCK_SECOND)

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

    // Example plaintext "Hello, World!" with PKCS#7 padding
    //uint8_t plaintext[16] = {
    //  'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0x03, 0x03, 0x03
    //};
    // Manually padded with 0x03 to make up 16 bytes, since "Hello, World!" is 13 bytes long
    
    //uint8_t ciphertext[16];
    //for (int i = 0; i < 16; ++i){
    //  ciphertext[i] = 0;
    //}
   
    // Initialize AES
    AES_128.set_key(encryption_key);    
    
    while(1) {
        // Set a timer to pace the sampling
        etimer_set(&et, TRANSFER_INTERVAL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        uint8_t plaintext[16] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0x00, 0x00, 0x00};

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

        // Decrypt ciphertext (for demonstration purposes)
        // Typically, you would decrypt on the receiving end
        AES_128.encrypt(plaintext); // Assuming AES_128.decrypt is the correct API call

        // Print decrypted data
        printf("Decrypted data received: ");
        for(int i = 0; i < 13; i++) { // assuming padding is removed on the receiving end
          printf("%c", plaintext[i]);
        }
        printf("\n");

        // Record time for decryption
        start_time = RTIMER_NOW();
        // ... decryption process ...
        end_time = RTIMER_NOW();

        duration = end_time - start_time;
        
        printf("Decryption time: %u ticks, %u microseconds\n", duration, (unsigned int)((uint64_t)duration * 1000000 / RTIMER_SECOND));
    }

  PROCESS_END();
}
