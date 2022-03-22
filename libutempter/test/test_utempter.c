#include <stdio.h>
#include <stdlib.h>
#include <pty.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../utempter.h"
#include "CUnit.h"
#include "Automated.h"

int check_update_utempter_success(const char * command)
{
    FILE* wtmp_fd;
    char buff[255];

    system("utmpdump /var/log/wtmp > wtmp.txt");
    wtmp_fd = fopen("wtmp.txt", "r");
    if (wtmp_fd == NULL) {
        printf("open wtmp failed, err = %d\n", errno);
        return -1;
    }

    while((fgets(buff, 255, wtmp_fd)) != NULL) {
        continue;
    }
    fclose(wtmp_fd);

    if (strstr(buff, "test_libutempter") != NULL) {
        // find test hostname, print success
        if (strcmp(command, "add") == 0) {
            printf("utempter add success\n");
            return 0;
        } else {
            printf("utempter add failed\n");
            return -1;
        }
    } else {
        if (strcmp(command, "del") == 0) {
            printf("utempter remove success\n");
            return 0;
        } else {
            printf("utempter remove failed\n");
            return -1;
        }
    }
}

void utcase_test_add_remove()
{
    int master;
    int slave;
    int err;
    int SUCCESS = 1;
    char pid[32];

    err = openpty(&master, &slave, NULL, NULL, NULL);
    if(0 > err) {
        printf("Error: %s\n", strerror(err));
        return;
    }

    err = utempter_add_record(master, "test_libutempter");
    printf("add_record result = %d\n", err);
    CU_ASSERT(err == SUCCESS)

    err = check_update_utempter_success("add");
    CU_ASSERT(err == 0);

    err = utempter_remove_added_record();
    printf("remove_record result = %d\n", err);

    CU_ASSERT(err == SUCCESS);
    err = check_update_utempter_success("del");
    CU_ASSERT(err == 0);

    close(slave);
    close(master);
}

void utcase_test_set_path()
{
    utempter_set_helper("/usr/lib/utempter/utempter");
    return;
}

static CU_TestInfo ut_cases[] =
{
    {"case:test_set_path", utcase_test_set_path},
    {"case:test_add_remove", utcase_test_add_remove},
    CU_TEST_INFO_NULL,
};

int suite_init(void)
{
    return 0;
}

int suite_clean(void)
{
    return 0;
}

static CU_SuiteInfo ut_suites[] = 
{
    {"my_first_suite", suite_init, suite_clean, NULL, NULL, ut_cases},
    CU_SUITE_INFO_NULL,
};

int main() {
    int rc = 0;
    CU_ErrorCode err = CUE_SUCCESS;

    err = CU_initialize_registry();
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to initialize registry, error %d", err);
        rc = 1;
        goto l_out;
    }

    err = CU_register_suites(ut_suites);
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to register suites, error %d, %s", err, CU_get_error_msg());
        rc = 1;
        goto l_clean_register;
    }

    CU_set_output_filename("cunit_sample");

    err = CU_list_tests_to_file();
    if (err != CUE_SUCCESS) {
        fprintf(stderr, "failed to list tests to file, error %d, %s", err, CU_get_error_msg());
        rc = 1;
        goto l_clean_register;
    }

    CU_automated_run_tests();

l_clean_register:
    CU_cleanup_registry();

l_out:
    return rc;
}
