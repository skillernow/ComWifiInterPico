#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include <math.h>

// Pins pour le contrôle des moteurs
const uint motor1_pwm_pin = 21; // PWM
const uint motor1_in1_pin = 22; // direction 1
const uint motor1_in2_pin = 20; // direction 2

const uint motor2_pwm_pin = 28; // PWM
const uint motor2_in1_pin = 27; // direction 1
const uint motor2_in2_pin = 26; // direction 2

const uint motor3_pwm_pin = 18; // PWM
const uint motor3_in1_pin = 19; // direction 1
const uint motor3_in2_pin = 17; // direction 2

#define MOTOR_SPEED 150   // PWM duty cycle (0-255)
#define FORWARD_TIME 5000 // en millisecondes
#define TURN_TIME 1500

static char *PATH = NULL;

void setup_motor_pwm(uint pwm_pin)
{
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);
}

void setup_motors()
{
    // Initialisation des pins PWM
    setup_motor_pwm(motor1_pwm_pin);
    setup_motor_pwm(motor2_pwm_pin);
    setup_motor_pwm(motor3_pwm_pin);

    // Initialisation des pins de direction
    gpio_init(motor1_in1_pin);
    gpio_set_dir(motor1_in1_pin, GPIO_OUT);
    gpio_init(motor1_in2_pin);
    gpio_set_dir(motor1_in2_pin, GPIO_OUT);

    gpio_init(motor2_in1_pin);
    gpio_set_dir(motor2_in1_pin, GPIO_OUT);
    gpio_init(motor2_in2_pin);
    gpio_set_dir(motor2_in2_pin, GPIO_OUT);

    gpio_init(motor3_in1_pin);
    gpio_set_dir(motor3_in1_pin, GPIO_OUT);
    gpio_init(motor3_in2_pin);
    gpio_set_dir(motor3_in2_pin, GPIO_OUT);
}

void set_motor(uint pwm_pin, uint in1_pin, uint in2_pin, bool forward, uint duty)
{
    if (forward)
    {
        gpio_put(in1_pin, 1);
        gpio_put(in2_pin, 0);
    }
    else
    {
        gpio_put(in1_pin, 0);
        gpio_put(in2_pin, 1);
    }
    pwm_set_gpio_level(pwm_pin, duty);
}

// Nouveau helper: commande AVANCER
// Active seulement les roues 2 et 3 en sens opposés (roue2 forward, roue3 reverse) et arrête après la durée.
void avancer(uint duty, uint32_t duration_ms)
{
    // Arrêt de la roue 1
    set_motor(motor1_pwm_pin, motor1_in1_pin, motor1_in2_pin, true, 0);
    // Calcul de la puissance 90% pour la roue 2
    uint duty90 = (uint)(duty * 0.85);
    // Activation: roue2 avance à 85% et roue3 recule à 100%
    set_motor(motor2_pwm_pin, motor2_in1_pin, motor2_in2_pin, true, duty90);
    set_motor(motor3_pwm_pin, motor3_in1_pin, motor3_in2_pin, false, duty);
    sleep_ms(duration_ms);
    // Arrêt des moteurs activés
    set_motor(motor2_pwm_pin, motor2_in1_pin, motor2_in2_pin, true, 0);
    set_motor(motor3_pwm_pin, motor3_in1_pin, motor3_in2_pin, true, 0);
}

// Nouveau helper: commande TOURNER
// Active toutes les roues dans le même sens selon le paramètre 'forward'.
// 'duty' est commun à chaque roue.
void tourner(bool forward, uint duty, uint32_t duration_ms)
{
    set_motor(motor1_pwm_pin, motor1_in1_pin, motor1_in2_pin, forward, duty);
    set_motor(motor2_pwm_pin, motor2_in1_pin, motor2_in2_pin, forward, duty);
    set_motor(motor3_pwm_pin, motor3_in1_pin, motor3_in2_pin, forward, duty);
    sleep_ms(duration_ms);
    // Arrêt de toutes les roues
    set_motor(motor1_pwm_pin, motor1_in1_pin, motor1_in2_pin, true, 0);
    set_motor(motor2_pwm_pin, motor2_in1_pin, motor2_in2_pin, true, 0);
    set_motor(motor3_pwm_pin, motor3_in1_pin, motor3_in2_pin, true, 0);
}

void process_command(char cmd)
{
    if (cmd == '1')
    {
        // Allumer la LED
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        tourner(false, MOTOR_SPEED, 400);
        sleep_ms(100);
        avancer(MOTOR_SPEED, 400);
    }
    else if (cmd == '0')
    {
        // Éteindre la LED
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        tourner(true, MOTOR_SPEED, 400);
        sleep_ms(100);
        avancer(MOTOR_SPEED, 400);
    }
}

#define TCP_PORT 4242
#define WIFI_SSID "RobotControl"
#define WIFI_PASSWORD "robot1234"
static struct tcp_pcb *tcp_pcb = NULL;

err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
        // Libère l'ancienne payload si nécessaire
        free(PATH);
        // Alloue et copie la nouvelle payload (+1 pour le '\0')
        PATH = malloc(p->len + 1);
        if (PATH != NULL)
        {
            memcpy(PATH, p->payload, p->len);
            PATH[p->len] = '\0';
        }

        // Traitement caractère par caractère
        for (int i = 0; i < p->len; i++)
        {
            char cmd = ((char *)p->payload)[i];
            process_command(cmd);
            sleep_ms(500);
        }
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    }
    return ERR_OK;
}

err_t tcp_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_pcb = newpcb;
    tcp_recv(newpcb, tcp_recv_callback);
    return ERR_OK;
}

void setup_wifi()
{
    if (cyw43_arch_init())
    {
        printf("Failed to initialize WiFi\n");
        return;
    }
    gpio_init(0);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(1000);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(1000);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    cyw43_arch_enable_ap_mode(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);

    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, TCP_PORT);
    pcb = tcp_listen(pcb);
    //
    tcp_accept(pcb, tcp_accept_callback);

    printf("WiFi AP started - SSID: %s, Password: %s\n", WIFI_SSID, WIFI_PASSWORD);
}

int main()
{
    stdio_init_all();
    setup_wifi();
    setup_motors();

    while (true)
    {
        cyw43_arch_poll();
        sleep_ms(200);
    }

    return 0;
}
