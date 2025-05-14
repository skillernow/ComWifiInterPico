#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/dns.h"

#define WIFI_SSID_CLIENT "RobotControl"
#define WIFI_PASSWORD_CLIENT "robot1234"

#define STATIC_IP_CLIENT "192.168.4.2"
#define SUBNET_MASK_CLIENT "255.255.255.0"
#define GATEWAY_IP_CLIENT "192.168.4.1"

#define MESSAGE "010100110011010100110011010100110011"
#define TCP_SERVER_PORT 4242

static err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(len);
    printf("TCP message sent, closing connection.\n");
    tcp_close(tpcb);
    return ERR_OK;
}

static err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    if (err == ERR_OK)
    {
        printf("TCP connected to gateway. Sending message: %s\n", MESSAGE);
        tcp_sent(tpcb, tcp_sent_callback);
        err_t write_err = tcp_write(tpcb, MESSAGE, strlen(MESSAGE), TCP_WRITE_FLAG_COPY);
        if (write_err != ERR_OK)
        {

            printf("Error writing TCP data: %d\n", write_err);
            tcp_close(tpcb);
            return write_err;
        }
        err_t output_err = tcp_output(tpcb);
        if (output_err != ERR_OK)
        {
            printf("Error sending TCP data (tcp_output): %d\n", output_err);
            tcp_close(tpcb);
            return output_err;
        }
    }
    else
    {
        printf("TCP connection error: %d\n", err);
        tcp_close(tpcb);
    }
    return err;
}

static void tcp_error_callback(void *arg, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    printf("TCP error callback: %d\n", err);
}

static bool send_tcp_message_to_gateway()
{
    struct tcp_pcb *pcb;
    ip_addr_t gw_ip;

    ip4addr_aton(GATEWAY_IP_CLIENT, &gw_ip);

    pcb = tcp_new_ip_type(IP_GET_TYPE(&gw_ip));
    if (!pcb)
    {
        printf("Error creating TCP PCB.\n");
        return false;
    }

    tcp_err(pcb, tcp_error_callback);
    err_t err = tcp_connect(pcb, &gw_ip, TCP_SERVER_PORT, tcp_connected_callback);
    if (err != ERR_OK)
    {
        printf("Error connecting TCP: %d\n", err);
        memp_free(MEMP_TCP_PCB, pcb);
        return false;
    }
    return true;
}

int main()
{
    stdio_init_all();
    cyw43_arch_init();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); // LED off initialement
    sleep_ms(1000);
    // Clignotement initial pour montrer que le programme démarre
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(250);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(250);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(250);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(250);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(250);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    // Enable wifi station
    cyw43_arch_enable_sta_mode();

    ip4_addr_t ip, mask, gw;
    ip4addr_aton(STATIC_IP_CLIENT, &ip);
    ip4addr_aton(SUBNET_MASK_CLIENT, &mask);
    ip4addr_aton(GATEWAY_IP_CLIENT, &gw);
    netif_set_ipaddr(netif_default, &ip);
    netif_set_netmask(netif_default, &mask);
    netif_set_gw(netif_default, &gw);

    printf("Attempting to connect to Wi-Fi: %s...\n", WIFI_SSID_CLIENT);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID_CLIENT, WIFI_PASSWORD_CLIENT, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Failed to connect to Wi-Fi.\n");
        // Boucle d'erreur : faire clignoter la LED rapidement
        while (true)
        {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(100);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(100);
            // Il faut appeler cyw43_arch_poll() même en cas d'erreur pour maintenir le système cyw43 actif
            // si des fonctionnalités de bas niveau sont toujours utilisées ou pour un redémarrage propre.
            // Cependant, dans ce cas simple, une boucle de clignotement suffit pour indiquer l'erreur.
        }
        // return 1; // Dans un OS, on retournerait. Ici, on boucle.
    }
    else
    {
        printf("Connected to Wi-Fi.\n");
        printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
        printf("Subnet Mask: %s\n", ip4addr_ntoa(netif_ip4_netmask(netif_default)));
        printf("Gateway: %s\n", ip4addr_ntoa(netif_ip4_gw(netif_default)));

        // Allumer la LED pour indiquer la connexion Wi-Fi réussie
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(1000);                                // Garder la LED allumée pendant 1s
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); // Puis l'éteindre avant de tenter l'envoi TCP

        printf("Attempting to send TCP message...\n");
        if (!send_tcp_message_to_gateway())
        {
            // Cette condition est atteinte si tcp_new_ip_type ou tcp_connect échouent immédiatement.
            printf("Failed to initiate TCP connection process (e.g., PCB creation or connect call failed).\n");
            // Gérer l'échec de l'initiation ici si nécessaire
        }
        // Si send_tcp_message_to_gateway retourne true, cela signifie que la tentative de connexion a été initiée.
        // Le succès réel de la connexion et de l'envoi sera indiqué par les callbacks TCP et leurs printf.
        // La LED indiquant l'envoi réussi devrait idéalement être gérée dans tcp_sent_callback.
    }

    // Boucle principale pour permettre à LwIP de traiter les événements réseau
    // et pour que les callbacks TCP soient exécutés.
    while (true)
    {
        // cyw43_arch_poll() est la fonction clé pour faire fonctionner LwIP
        // en mode NO_SYS=1 avec le pilote cyw43.
        // Elle gère les E/S réseau, les temporisateurs LwIP et permet l'exécution des callbacks.
        cyw43_arch_poll();

        // Optionnel: mettre le CPU en veille pour économiser de l'énergie
        // si aucune tâche immédiate n'est nécessaire.
        // Pour le débogage, un polling continu est souvent plus simple.
        // sleep_ms(1); // Décommenter pour réduire l'utilisation du CPU, mais peut ralentir la réponse réseau.
    }

    // Le code ci-dessous ne sera jamais atteint dans une boucle while(true) typique d'un système embarqué.
    // cyw43_arch_deinit();
    // return 0;
}
