#include "client.h"

bool login_prompt_username(char *out)
{
    char line[256];

    while (true) {
        printf("Username: ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            return false;

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (username_valid(line)) {
            strcpy(out, line);
            return true;
        }

        printf("Invalid username. Use 1-%d letters, digits, '_', '-' or '.'.\n", USERNAME_MAX_LEN);
    }
}
