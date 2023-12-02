#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "lib/aes-128.h"
#include "dev/radio/cc2420/cc2420.h"
#include "sys/etimer.h"
#include <string.h>
#include <stdio.h> /* For printf() */

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)
#define packet_length 16 // 16 bytes = 128 bits

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */


//zero padding, 2 padding bytes, 1 byte for length
static uint8_t crosschecker[packet_length] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0x00, 0x00, 0x02};

// Function to flip a bit in a byte array, used for bit error rate testing
void flip_bit(uint8_t *data, int position); 
uint8_t compute_parity(const uint8_t *data, size_t len);
unsigned int count_bit_errors(const uint8_t *received, size_t length);
//void encrypt_and_send(const uint8_t *plaintext);

//Decryption: http://aes.online-domain-tools.com/
//Decryption key: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
static uint8_t encryption_key[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};




//Things to check:
//1. processing time          CHECK
//2. power consumption
//3. bit error rate           CHECK
//4. memory footprint










/*---------------------------------------------------------------------------*/
PROCESS(encrypt_transmit, "encrypt & transmit data");
AUTOSTART_PROCESSES(&encrypt_transmit);







//RECIEVING
/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  //LOG_INFO("len: %d \n", len );
  if(len > 10) {
    unsigned char ciphertext[len];
    memcpy(&ciphertext, data, sizeof(ciphertext));
    printf("----------------- ENCRYPTION INPUT -----------------\n");
    printf("Encrypted data: ");
    for(int i = 0; i < 16; i++) {
      printf("%02x", ciphertext[i]);
    }
    printf(" from ");
    LOG_INFO_LLADDR(src);
    printf("\n");

    printf("------------------------------------------------------\n\n");

    //VERNERS KODE HER

  }
  else
  {
    printf("----------------- ENCRYPTION INPUT -----------------\n");
    uint8_t parity_bit;
    memcpy(&parity_bit, data, sizeof(parity_bit));
    printf("Recieved parity bit (%u) from ", parity_bit);
    LOG_INFO_LLADDR(src);
    printf("\n");
    printf("------------------------------------------------------\n\n");

    //VERNERS KODE HER

  }
}



//TRANSMITTING
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(encrypt_transmit, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned char ciphertext[] = "b5a64ea9a4bb5de657090d3cfbf9934e";
  static rtimer_clock_t start_time, end_time, duration;
  static unsigned int total_bit_errors = 0;
  static unsigned int total_bits_received = 0;

  PROCESS_BEGIN();

  AES_128.set_key(encryption_key); //Initialize AES
  AES_128.encrypt(crosschecker); //Precompute the encrypted data
  
  #if MAC_CONF_WITH_TSCH
    tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
  #endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (unsigned char *)&ciphertext;
  nullnet_len = sizeof(ciphertext);


  /* Check to see if we recieved anything */
  nullnet_set_input_callback(input_callback);


  etimer_set(&periodic_timer, SEND_INTERVAL);


  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    //uint8_t plaintext[packet_length] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0x00, 0x00, 0x02};
    
    uint8_t ciphertext[packet_length] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0x00, 0x00, 0x02};

    //ENCRYPTION START
    // Start time measurement
    start_time = RTIMER_NOW();

    // Encrypt plaintext
    AES_128.encrypt(ciphertext); // Ensure that encrypt function takes two arguments

    // End time measurement
    end_time = RTIMER_NOW();
    
    // Calculate duration
    duration = end_time - start_time;
    // Print Encryption output
    printf("----------------- ENCRYPTION OUTPUT -----------------\n");
    printf("Encrypted data: ");
    for(int i = 0; i < 16; i++) {
      printf("%02x", ciphertext[i]);
    }
    printf("\n");

    printf("Encryption time: %u ticks\n", (unsigned)duration);
    printf("------------------------------------------------------\n\n");

    //TRANSMISSION START
    printf("------------|||||| Transmitting encrypted data\n\n");
    nullnet_buf = (unsigned char *)&ciphertext;
    nullnet_len = sizeof(ciphertext);

    NETSTACK_NETWORK.output(NULL);
    etimer_reset(&periodic_timer);


    //calculate parity
    printf("----------------- PARITY OUTPUT -----------------\n");
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    uint8_t parity_before = compute_parity(ciphertext, sizeof(ciphertext));
    printf("Parity bit b4 flip: %u\n", parity_before);

    //Flip the second bit of the ciphertext
    flip_bit(ciphertext, 1); 

    //compute parity after bit flip
    uint8_t parity_after = compute_parity(ciphertext, sizeof(ciphertext));
    printf("Parity bit after flip: %u\n", parity_after);
    


    printf("----------------- Calculating BER -----------------\n");
    //Counting bit errors
    unsigned int new_bit_errors = count_bit_errors(ciphertext, sizeof(ciphertext));
    printf("Bit errors: %d\n", new_bit_errors);
    total_bit_errors += new_bit_errors;
    total_bits_received += packet_length * 8;
    int ber = total_bit_errors / total_bits_received;
    printf("Updated Bit Error Rate (BER): %d \n", ber);

    printf("Sending parity bit: %u \n", parity_after);
    
    nullnet_buf = (unsigned char *)&parity_after;
    nullnet_len = sizeof(parity_after);

    NETSTACK_NETWORK.output(NULL);
    etimer_reset(&periodic_timer);
    printf("------------------------------------------------------\n\n");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/






























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

        //printf("Byte %d, Expected: 0x%02x, Received: 0x%02x, XOR: 0x%02x\n", i, crosschecker[i], received[i], xor);

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


