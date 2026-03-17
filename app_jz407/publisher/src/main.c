#include <stdio.h>
#include <unistd.h>
#include <zenoh-pico.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/mem_stats.h>
#include <zephyr/sys/sys_heap.h>

#include <zephyr/net/net_if.h>

// Uncomment if you have network problems

// /*
void restart_network_interface(void) {
  struct net_if *iface = net_if_get_default();

  if (iface) {
    net_if_down(iface);
    k_msleep(500);
    net_if_up(iface);
  }
}
// */


int main(void) {
  static z_owned_session_t s;
  while (true) {
    printk("\n--- Zenoh-Pico Configuration Check ---\n");
    printk("Z_FRAG_MAX_SIZE:        %d bytes\n", Z_FRAG_MAX_SIZE);
    printk("Z_BATCH_UNICAST_SIZE:   %d bytes\n", Z_BATCH_UNICAST_SIZE);

    k_msleep(5000);
    z_owned_config_t config;
    z_config_default(&config);
    zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, "client");
    zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY, "tcp/192.168.8.220:7447");
    zp_config_insert(z_loan_mut(config), Z_CONFIG_MULTICAST_SCOUTING_KEY, "false");

    if (z_open(&s, z_move(config), NULL) < 0) {
      printk("Unable to open session!\n");
      z_drop(z_move(s));
      restart_network_interface();
      k_msleep(500);
      continue;
    }

    zp_start_read_task(z_loan_mut(s), NULL);
    zp_start_lease_task(z_loan_mut(s), NULL);

    printk("Session opened!\n");

    z_owned_publisher_t pub_str;

    z_view_keyexpr_t ke_str;
    z_view_keyexpr_from_str_unchecked(&ke_str, "example/zenoh-pico-pub");

    if (z_declare_publisher(z_loan(s), &pub_str, z_loan(ke_str), NULL) < 0) {
      printk("Unable to declare publisher for string topic!\n");
      continue;
    }

    printk("Ok!\n");
    char hello[] = "Hello world!";
    while (true) {
      z_owned_bytes_t payload;
      z_bytes_copy_from_str(&payload, hello);

      if (z_publisher_put(z_loan(pub_str), z_move(payload), NULL) < 0) {
        printk("Send failed! zenohd might be down.\n");
        break;
      }
      printk("Send!\n");
      k_msleep(1000);
    }
    printk("Closing Zenoh Session...");
    z_drop(z_move(pub_str));

    z_drop(z_move(s));
    printk("OK!\n");
  }
  return 0;
}