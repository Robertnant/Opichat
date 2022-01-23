#define _GNU_SOURCE
#include <criterion/criterion.h>

#include <stdio.h>
#include "utils/lexer.h"
#include "utils/xalloc.h"

// Checks if parsed tokens match expected results.
int match(char **expected, char **actual, int expected_count, int actual_count)
{
    if (expected_count != actual_count)
        return 0;

    for (int i = 0; i < actual_count; i++)
    {
        if (strcmp(actual[i], expected[i]) != 0)
            return 0;
    }

    return 1;
}

Test(CLIENT, ping)
{
    char *message = "0\n0\nPING\n\n";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[4] = { "0", "0", "PING", ""};
    int expected_count = 4;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(CLIENT, ping_payload)
{
    char *message = "40\n0\nPING\n\nIt is what it is and it do be like that\n";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[5] = { "40", "0", "PING", "",
        "It is what it is and it do be like that\n" };
    int expected_count = 5;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(CLIENT, payload_newline)
{
    char *message = "1\n0\nPING\n\n\n";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[5] = { "1", "0", "PING", "", "\n" };
    int expected_count = 5;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(CLIENT, parse_login)
{
    char *message = "3\n0\nLOGIN\n\nacu";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[5] = { "3", "0", "LOGIN", "", "acu" };
    int expected_count = 5;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(CLIENT, list_users)
{
    char *message = "0\n0\nLIST_USERS\n\n";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[4] = { "0", "0", "LIST_USERS", "" };
    int expected_count = 4;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(LIST_ROOMS, rooms)
{
    char *message = "0\n0\nLIST_ROOMS\n\n";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[4] = { "0", "0", "LIST_ROOMS", "" };
    int expected_count = 4;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(SEND_DM, send_dm_payload)
{
    char *message = "4\n0\nSEND_DM\nUser=acu\n\n2021";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[6] = { "4", "0", "SEND_DM", "User=acu", "", "2021" };
    int expected_count = 6;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(SEND_DM, send_dm_empty)
{
    char *message = "0\n0\nSEND_DM\nUser=acu\n\n";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[5] = { "0", "0", "SEND_DM", "User=acu", "" };
    int expected_count = 5;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(BROADCAST, broadcast_empty)
{
    char *message = "0\n0\nBROADCAST\n\n";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[4] = { "0", "0", "BROADCAST", "" };
    int expected_count = 4;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(BROADCAST, broadcast_payload)
{
    char *message = "5\n0\nBROADCAST\n\nSnap\n";
    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);
    char *expected[5] = { "5", "0", "BROADCAST", "", "Snap\n" };
    int expected_count = 5;

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(CLIENT, multi_message)
{
    char *message =
        "0\n0\nLIST_USERS\n\n0\n0\nPING\n\n4\n0\nSEND-DM\nUser=acu\n\n2022";
    char ***expected = xmalloc(2 * sizeof(char **));
    for (int i = 0; i < 2; i++)
    {
        expected[i] = xmalloc(sizeof(char *));
    }

    asprintf(&expected[0][0], "0");
    asprintf(&expected[0][1], "0");
    asprintf(&expected[0][2], "LIST_USERS");
    expected[0][3] = calloc(1, sizeof(char));

    asprintf(&expected[1][0], "4");
    asprintf(&expected[1][1], "0");
    asprintf(&expected[1][2], "SEND-DM)");
    asprintf(&expected[1][3], "User=acu");
    expected[1][4] = calloc(1, sizeof(char));
    asprintf(&expected[1][5], "2022");

    int expected_counts[] = { 4, 6 };

    int all_match = 1;

    for (int i = 0; i < 2; i++)
    {
        int count = 0;
        char **parsed = NULL;
        parsed = lexer(&message, &count);

        int curr_match = match(expected[i], parsed, expected_counts[i], count);
        all_match &= curr_match;

        if (parsed)
            free(parsed);
    }

    cr_assert_eq(all_match, 1);
}

Test(CLIENT, partial_message)
{
    char *message =
        "0\n0\nLIST_U";
    int expected_count = 0;
    char **expected = NULL;

    int count = 0;
    char **parsed = NULL;
    parsed = lexer(&message, &count);

    cr_assert_eq(match(expected, parsed, expected_count, count), 1);

    if (parsed)
        free(parsed);
}

Test(CLIENT, full_partial_message)
{
    char *message =
        "0\n0\nLIST_USERS\n\n0\n0\nPI";
    int expected_counts[] = { 4, 0 };
    char ***expected = xcalloc(2, sizeof(char **));

    asprintf(&expected[0][0], "0");
    asprintf(&expected[0][1], "0");
    asprintf(&expected[0][2], "LIST_USERS");
    expected[0][3] = calloc(1, sizeof(char));

    int all_match = 1;

    for (int i = 0; i < 2; i++)
    {
        int count = 0;
        char **parsed = NULL;
        parsed = lexer(&message, &count);

        int curr_match = match(expected[i], parsed, expected_counts[i], count);
        all_match &= curr_match;

        if (parsed)
            free(parsed);
    }

    cr_assert_eq(all_match, 1);
}
