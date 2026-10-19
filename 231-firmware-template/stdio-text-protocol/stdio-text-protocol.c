#include "stdio-text-protocol.h"
#include "pico/stdlib.h"
#include "log.h"




static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static int line_length;
command_t request;


void stdio_text_protocol_init(void){
    line_length = 0;
}


const command_t *stdio_text_protocol_handle(void)
{
    // читаем символ без ожидания; нет символа — возвращаем NULL
    int symbol = getchar_timeout_us(0);

    if (symbol == PICO_ERROR_TIMEOUT)
    {
        return NULL;
    }

    
    // строка получена. делим её на имя, аргументы и возращаем.
    if (symbol == '\r' || symbol == '\n')
    {
        putchar('\n');
        line[line_length] = '\0';

        request = (command_t){0};

        if (line_length > 0)
        {
            LOG_DBG("got %s\n", line);
            char *p = line;
            request.name = p;
            while (*p && *p != ' ') p++;
                if (*p == ' ') { *p = '\0'; p++; }

                // остальные аргументы
                while (*p) {
                    // пропускаем повторные пробелы
                    while (*p == ' ') p++;
                    if (!*p) break;

                    if (request.argc >= STDIO_TEXT_PROTOCOL_MAX_ARGS) {
                        request.truncated = true;
                        break;
                    }

                    request.argv[request.argc++] = p;
                    while (*p && *p != ' ') p++;
                    if (*p == ' ') { *p = '\0'; p++; }
                }
            line_length = 0;
            return &request;

            }

        }


    // Backspace (0x08 или 0x7f): убираем последний символ и стираем его на экране "\b \b"
    // печатный символ: кладём в буфер и повторяем в терминал;
    // буфер полон — помечаем строку как переполненную
    if (line_length + 1 < STDIO_TEXT_PROTOCOL_LINE_SIZE)
    {

        if (symbol == 0x08 || symbol == 0x7f) {
            if (line_length != 0){
                line_length = line_length -1;
                putchar(0x08);
                putchar(0x20);
                putchar(0x08);
            }
        } else if (symbol > 0x08 && symbol <= 0x7e){
            if (line_length <=63){
                line[line_length] = (char)symbol;
                line_length = line_length + 1;
                putchar(symbol);
            } else  if (line_length >63){
                
                request.truncated = true;
            }
        }

    }

    return NULL;
}


