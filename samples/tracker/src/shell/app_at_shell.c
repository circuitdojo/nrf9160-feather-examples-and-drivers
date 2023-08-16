/*
 * Copyright (c) 2022 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <zephyr/shell/shell.h>
#include <nrf_modem_at.h>

static char response[CONFIG_SHELL_AT_CMD_RESPONSE_MAX_LEN + 1];
static uint8_t raw_buf[4096] = {0};
static int raw_len = 0;
static int raw_buf_offset = 0;

static const char at_usage_str[] =
    "Usage: at <subcommand>\n"
    "\n"
    "Subcommands:\n"
    "  raw <len>    Bypass the shell to account for newlines.\n"
    "\n"
    "Any other subcommand is interpreted as AT command and sent to the modem.\n";

static void bypass_cb(const struct shell *sh, uint8_t *data, size_t len)
{
    int err;

    for (int i = 0; i < len; i++)
    {

        // Skip if we get a newline or carriage return
        if (raw_buf_offset == 0 && (data[0] == '\n' || data[0] == '\r'))
            continue;

        raw_buf[raw_buf_offset] = data[i];
        raw_buf_offset += 1;

        /* Disable bypass once we get the byte count as expected. */
        if (raw_len == raw_buf_offset)
        {
            shell_set_bypass(sh, NULL);

            /* Execute AT command */
            err = nrf_modem_at_printf("%.*s", raw_len, (const char *)raw_buf);
            if (err < 0)
            {
                printk("ERROR %i", err);
            }
            else
            {
                printk("OK");
            }

            /* Reset variables */
            raw_buf_offset = 0;
            raw_len = 0;
            memset(raw_buf, 0, sizeof(raw_buf));

            break;
        }
    }
}

int at_shell(const struct shell *shell, size_t argc, char **argv)
{
    int err;

    /* Make sure we have another arguement */
    if (argc < 2)
    {
        shell_print(shell, "%s", at_usage_str);
        return 0;
    }

    /* Get the command */
    char *command = argv[1];

    /* Compare command */
    if (!strcmp(command, "raw"))
    {
        if (argc < 3)
        {
            shell_print(shell, "%s", at_usage_str);
            return 0;
        }

        /* Store the length */
        raw_len = atoi(argv[2]);

        /* Bypass */
        shell_set_bypass(shell, bypass_cb);

        /* Send ok to start raw bits */
        shell_print(shell, "OK");
    }
    else
    {
        err = nrf_modem_at_cmd(response, sizeof(response), command);
        if (err < 0)
        {
            shell_print(shell, "Sending AT command failed with error code %d", err);
            return err;
        }
        else if (err)
        {
            shell_print(shell, "%s", response);
            return -EINVAL;
        }

        shell_print(shell, "%s", response);
    }

    return 0;
}

SHELL_CMD_REGISTER(at, NULL, "AT commands with bypass for longer ones.", at_shell);
