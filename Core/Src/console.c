#include "console.h"
#include "uart_driver.h"
#include "traffic_light.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define CONSOLE_LINE_MAX    48U
#define CONSOLE_TOKENS_MAX  4U
#define CONSOLE_NUMBER_MAX_DIGITS  7U
#define CHAR_BACKSPACE      0x08U
#define CHAR_DELETE         0x7FU
#define CHAR_PRINTABLE_MIN  0x20U
#define CHAR_PRINTABLE_MAX  0x7EU

static char    line[CONSOLE_LINE_MAX];
static uint8_t line_length;
static bool    previous_was_cr;

static void reply(const char *text)
{
    uart_send_string(text);
    uart_send_string("\r\n");
}

static void reply_result(bool success)
{
    reply(success ? "OK" : "Invalid argument");
}

static bool parse_unsigned(const char *text, uint32_t *value)
{
    uint32_t result = 0U;
    uint32_t digits = 0U;

    for (; *text != '\0'; text++, digits++) {
        if ((*text < '0') || (*text > '9') || (digits >= CONSOLE_NUMBER_MAX_DIGITS)) {
            return false;
        }
        result = result * 10U + (uint32_t)(*text - '0');
    }

    *value = result;
    return digits > 0U;
}

static uint8_t tokenize(char *text, char *tokens[])
{
    uint8_t count = 0U;

    while (*text != '\0') {
        while (*text == ' ') {
            *text++ = '\0';
        }
        if (*text == '\0') {
            break;
        }
        if (count == CONSOLE_TOKENS_MAX) {
            return CONSOLE_TOKENS_MAX + 1U;
        }
        tokens[count++] = text;
        while ((*text != '\0') && (*text != ' ')) {
            text++;
        }
    }

    return count;
}

static void report_status(void)
{
    char text[80];

    snprintf(text, sizeof(text), "%s, mode %u, timeout %lu, %c",
             traffic_light_state_name(),
             (unsigned)traffic_light_get_mode(),
             (unsigned long)traffic_light_get_timeout_s(),
             uart_interrupts_enabled() ? 'I' : 'P');
    reply(text);
}

static void set_mode(const char *argument)
{
    if (strcmp(argument, "1") == 0) {
        traffic_light_set_mode(TRAFFIC_LIGHT_MODE_1);
        reply_result(true);
    } else if (strcmp(argument, "2") == 0) {
        traffic_light_set_mode(TRAFFIC_LIGHT_MODE_2);
        reply_result(true);
    } else {
        reply_result(false);
    }
}

static void set_timeout(const char *argument)
{
    uint32_t seconds;
    reply_result(parse_unsigned(argument, &seconds) &&
                 traffic_light_set_timeout_s(seconds));
}

static void set_interrupts(const char *argument)
{
    if (strcmp(argument, "on") == 0) {
        uart_set_interrupts(true);
        reply_result(true);
    } else if (strcmp(argument, "off") == 0) {
        uart_set_interrupts(false);
        reply_result(true);
    } else {
        reply_result(false);
    }
}

static void execute_set(const char *parameter, const char *argument)
{
    if (strcmp(parameter, "mode") == 0) {
        set_mode(argument);
    } else if (strcmp(parameter, "timeout") == 0) {
        set_timeout(argument);
    } else if (strcmp(parameter, "interrupts") == 0) {
        set_interrupts(argument);
    } else {
        reply("Unknown command");
    }
}

static void execute_line(void)
{
    char    *tokens[CONSOLE_TOKENS_MAX];
    uint8_t  count;

    line[line_length] = '\0';
    count = tokenize(line, tokens);

    if (count == 0U) {
        return;
    }

    if ((count == 1U) && (strcmp(tokens[0], "?") == 0)) {
        report_status();
    } else if ((count == 3U) && (strcmp(tokens[0], "set") == 0)) {
        execute_set(tokens[1], tokens[2]);
    } else {
        reply("Unknown command");
    }
}

static void process_char(uint8_t c)
{
    if ((c == '\n') && previous_was_cr) {
        previous_was_cr = false;
        return;
    }
    previous_was_cr = (c == '\r');

    if ((c == '\r') || (c == '\n')) {
        uart_send_string("\r\n");
        execute_line();
        line_length = 0U;
    } else if ((c == CHAR_BACKSPACE) || (c == CHAR_DELETE)) {
        if (line_length > 0U) {
            line_length--;
            uart_send_string("\b \b");
        }
    } else if ((c >= CHAR_PRINTABLE_MIN) && (c <= CHAR_PRINTABLE_MAX) &&
               (line_length < (CONSOLE_LINE_MAX - 1U))) {
        line[line_length++] = (char)c;
        uart_send(&c, 1U);
    }
}

void console_init(void)
{
    line_length     = 0U;
    previous_was_cr = false;
}

void console_update(void)
{
    uint8_t c;

    while (uart_receive_byte(&c)) {
        process_char(c);
    }
}
