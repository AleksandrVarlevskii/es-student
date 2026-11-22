#include "api-commands.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>









void api_handle(const command_t *command)
{
    if (command == NULL || command->name == NULL) {
        return;
    }

    if (command->truncated) {
        printf("error: command truncated (max 63 chars, max 4 args)\r\n");
        return;
    }

    // help — вне таблицы: печатает все команды со справкой
    if (strcmp(command->name, "help") == 0) {
        printf("%-18s %s\r\n", "command", "description");
        for (uint32_t i = 0; i < api_command_count; i++) {
            printf("%-18s %s\r\n", api_commands[i].name, api_commands[i].help);
        }
        return;
    }

    for (uint32_t i = 0; i < api_command_count; i++) {
        if (strcmp(command->name, api_commands[i].name) == 0) {
            api_commands[i].callback(command);
            return;
        }
    }

    printf("error: unknown command '%s', try help\r\n", command->name);
}

