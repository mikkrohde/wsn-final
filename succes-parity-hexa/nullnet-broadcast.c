#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> /* For printf() */

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet unicast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  //LOG_INFO("len: %d \n", len );
  if(len > 10) {
    unsigned char ciphertext[len];
    memcpy(&ciphertext, data, sizeof(ciphertext));
    LOG_INFO("Recieving encrypted ciphertext (%s) from ", ciphertext);
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");
  }
  else
  {
    unsigned parity_bit;
    memcpy(&parity_bit, data, sizeof(parity_bit));
    LOG_INFO("Recieved parity bit (%u) from ", parity_bit);
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");
  }
  
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned char ciphertext[] = "b5a64ea9a4bb5de657090d3cfbf9934e";
  static unsigned parity_bit = 0x01;


  PROCESS_BEGIN();

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (unsigned char *)&ciphertext;
  nullnet_len = sizeof(ciphertext);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("Sending encrypted data (%s) to ", ciphertext);
    //LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");
    
    nullnet_buf = (unsigned char *)&ciphertext;
    nullnet_len = sizeof(ciphertext);

    NETSTACK_NETWORK.output(NULL);
    etimer_reset(&periodic_timer);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("Sending parity bit (%u) to ", parity_bit);
    //LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");
    
    nullnet_buf = (uint8_t *)&parity_bit;
    nullnet_len = sizeof(parity_bit);

    NETSTACK_NETWORK.output(NULL);
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/