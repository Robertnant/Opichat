#include <criterion/criterion.h>
#include <stdlib.h>

#include "../src/opichat_client.h"
#include "../src/utils/lexer.h"
#include "../src/utils/tools.h"
#include "../src/utils/xalloc.h"

// Redirects standard output.
void redirect_all_stdout(void)
{
    cr_redirect_stdout();
}

Test(CLIENT, gen_message_login_1)
{
    size_t size = 6;
    int status = 0;
    char *command = "LOGIN";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "Robert");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "6\n0\nLOGIN\n\nRobert";
    cr_assert_eq(expected, request, "Expected: %s. got: %s", expected, request);
}

Test(CLIENT, gen_message_login_2)
{
    size_t size = 13;
    int status = 0;
    char *command = "LOGIN";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "BetterThanACU");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "13\n0\nLOGIN\n\nBetterThanACU";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_ping)
{
    size_t size = 0;
    int status = 0;
    char *command = "PING";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    char *request = gen_message(size, status, command, params);
    char *expected = "0\n0\nPING\n\n";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_list-user)
{
    size_t size = 0;
    int status = 0;
    char *command = "LIST-USERS";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    char *request = gen_message(size, status, command, params);
    char *expected = "0\n0\nLIST-USERS\n\n";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_send-dm)
{
    size_t size = 4;
    int status = 0;
    char *command = "SEND-DM";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    add_param(params, "User", "acu");
    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "2022");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "4\n0\nSEND-DM\nUser=acu\n\n2022";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_broadcast)
{
    size_t size = 4;
    int status = 0;
    char *command = "BROADCAST";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "2022");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "4\n0\nBROADCAST\n\n2022";
    cr_assert__eq_str(request, expected);
}

Test(CLIENT, gen_message_create-room)
{
    size_t size = 8;
    int status = 0;
    char *command = "CREATE-ROOM";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "FlagRoom");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "8\n0\nCREATE-ROOM\n\nFlagRoom";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_list-rooms)
{
    size_t size = 0;
    int status = 0;
    char *command = "LIST-ROOMS";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    char *request = gen_message(size, status, command, params);
    char *expected = "0\n0\nLIST-ROOMS\n\n";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_join-room)
{
    size_t size = 8;
    int status = 0;
    char *command = "JOIN-ROOM";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "FlagRoom");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "8\n0\nJOIN-ROOM\n\npayload";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_leave-room)
{
    size_t size = 8;
    int status = 0;
    char *command = "LEAVE-ROOM";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "FlagRoom");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "8\n0\nLEAVE-ROOM\n\npayload";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_send-room)
{
    size_t size = 4;
    int status = 0;
    char *command = "SEND-ROOM";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    add_param(params, "Room", "FlagRoom");
    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "2022");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "4\n0\nSEND-ROOM\nRoom=FlagRoom\n\n2022";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_delete-room)
{
    size_t size = 8;
    int status = 0;
    char *command = "DELETE-ROOM";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "FlagRoom");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "8\n0\nDELETE-ROOM\n\nFlagRoom";
    cr_assert_eq_str(request, expected);
}

Test(CLIENT, gen_message_profile)
{
    size_t size = 0;
    int status = 0;
    char *command = "PROFILE";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    char *request = gen_message(size, status, command, params);
    char *expected = "0\n0\nPROFILE\n\n";
    cr_assert_stdout_eq_str(request, expected);
}

/* Test(CLIENT, gen_message_0)
{
    size_t size = 0;
    int status = 0;
    char *command = "";
    struct params_payload *params = xcalloc(1, sizeof(struct params_payload));

    add_param(params, "", "");
    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "");
    new->next = params;

    char *request = gen_message(size, status, command, new);
    char *expected = "size\nstatus\ncommand\nparams\npayload";
    cr_assert_eq_str(request, expected);
} */
