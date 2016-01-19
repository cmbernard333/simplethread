#include "cutest.h"

int main() {
  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  if (-1 == setup_bus_thread_suite())
    return CU_get_error();
  if (-1 == setup_bus_blocking_queue_suite())
    return CU_get_error();

  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();

  if (CU_get_number_of_failure_records() > 0) {
    return 1;
  }

  CU_cleanup_registry();

  if (CU_get_error() != 0) {
    return 1;
  }

  return 0;
}
