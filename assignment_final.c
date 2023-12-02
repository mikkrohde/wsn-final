#include "contiki.h"
#include "lib/aes-128.h"
#include "dev/radio/cc2420/cc2420.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>

#define TRANSFER_INTERVAL (5 * CLOCK_SECOND)
#define CHANNEL 11 // 26 channels in the 2.4 GHz band, numbered 11 to 26
#define packet_length 16 // 16 bytes = 128 bits

//zero padding, 2 padding bytes, 1 byte for length
//static const uint8_t crosschecker[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0x00, 0x00, 0x02};

void flip_bit(uint8_t *data, int position); // Function to flip a bit in a byte array, used for bit error rate testing
uint8_t compute_parity(const uint8_t *data, size_t len);

unsigned int count_bit_errors(const uint8_t *received, size_t length);
//void encrypt_and_send(const uint8_t *plaintext);

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
    static unsigned int total_bit_errors = 0;
    static unsigned int total_bits_received = 0;

    

    PROCESS_BEGIN();

    cc2420_init();
    cc2420_set_channel(CHANNEL);
    AES_128.set_key(encryption_key); // Initialize AES
    
    while(1) {
        // Set a timer to pace the sampling
        etimer_set(&et, TRANSFER_INTERVAL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        
        uint8_t plaintext[packet_length] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0x00, 0x00, 0x02};        

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

        //calculate parity
        uint8_t parity_before = compute_parity(plaintext, sizeof(plaintext));
        printf("Parity bit: %d\n", parity_before);

        flip_bit(plaintext, 1); // Flip the second bit of the ciphertext

        //compute parity after bit flip
        uint8_t parity_after = compute_parity(plaintext, sizeof(plaintext));
        printf("Parity bit after bit flip: %d\n", parity_after);



        /*
        no work, why?
        unsigned int new_bit_errors = count_bit_errors(plaintext, sizeof(plaintext));
        
        printf("Bit errors: %d\n", new_bit_errors);

        total_bit_errors += new_bit_errors;
        total_bits_received += packet_length * 8;

        int ber = total_bit_errors / total_bits_received;
        printf("Updated Bit Error Rate (BER): %d \n", ber);*/
    }

  PROCESS_END();
}


void flip_bit(uint8_t *data, int position) {
    int byte_pos = position / 8; // Find the byte in which the bit is located
    int bit_pos = position % 8;  // Find the position of the bit in that byte
    data[byte_pos] ^= (1 << bit_pos); // XOR with 1 shifted to the correct bit position flips the bit
}

uint8_t compute_parity(const uint8_t *data, size_t len) {
    uint8_t parity = 0;
    for (size_t i = 0; i < len; ++i) {
        parity ^= data[i]; // XOR all bytes
    }

    // Simplify parity to a single bit (even parity)
    parity = parity ^ (parity >> 4);
    parity = parity ^ (parity >> 2);
    parity = parity ^ (parity >> 1);
    return parity & 1;
}

unsigned int count_bit_errors(const uint8_t *received, size_t length) {
    unsigned int bit_errors = 0;
    for (size_t i = 0; i < length; i++) {
        uint8_t xor = crosschecker[i] ^ received[i];

        printf("Byte %zu, Expected: 0x%02x, Received: 0x%02x, XOR: 0x%02x\n", i, crosschecker[i], received[i], xor);

        while (xor > 0) {
            bit_errors += xor & 1;
            xor >>= 1;
        }
    }
    return bit_errors;
}

/*
void encrypt_and_send(const uint8_t *plaintext) {
    uint8_t encrypted[16]; // AES-128 outputs 128 bits (16 bytes)
    AES_128.encrypt(plaintext, encrypted); // Assuming this function exists

    // Compute parity bit
    uint8_t parity = compute_parity(encrypted, sizeof(encrypted));

    // Send encrypted data
    printf("Encrypted data sent: ");
    for(int i = 0; i < 16; i++) {
      printf("%02x", plaintext[i]);
    }
    printf("\n");

    // Send parity bit
    printf("Parity bit: %d\n", parity);
}*/

/*
bool receive_and_check(uint8_t *encrypted_data) {
    // Receive encrypted data and parity bit
    // ... (receive 16 bytes of encrypted data and 1 parity bit) ...

    // Compute expected parity
    uint8_t expected_parity = compute_parity(encrypted_data, 16);

    // Compare parities
    if (received_parity == expected_parity) {
        // Decrypt data
        //AES_128.decrypt(encrypted_data);
        return true; // Data is valid
    } else {
        return false; // Parity check failed, data is corrupted
    }
}
*/


