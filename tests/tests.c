#include <criterion/criterion.h>

#include "opichat_server.h"

// Redirects standard output.
void redirect_all_stdout(void)
{
    cr_redirect_stdout();
}

Test(PING, ping, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(PING, ping_payload, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(LOGIN, login, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(LOGIN, invalid_username, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(LOGIN, duplicate_username, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(LIST_USERS, empty, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(LIST_USERS, users, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(SEND_DM, logged_in, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(SEND_DM, anonymous, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(SEND_DM, unknown_user, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}

Test(BROADCAST, broadcast, .init = redirect_all_stdout)
{
    char *request;
    char *expected;
    cr_assert_stdout_eq_str(expected);
}
