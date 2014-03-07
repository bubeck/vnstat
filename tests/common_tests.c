#include "vnstat_tests.h"
#include "common_tests.h"
#include "common.h"
#include "dbaccess.h"
#include "cfg.h"

START_TEST(printe_options)
{
	noexit = 2;
	cfg.uselogging = 0;
	ck_assert_int_eq(printe(PT_Info), 1);

	cfg.uselogging = 1;
	ck_assert_int_eq(printe(PT_Multiline), 1);

	noexit = 0;
	strcpy(errorstring, "dummy string");
	suppress_output();
	ck_assert_int_eq(printe(PT_Info), 1);
	ck_assert_int_eq(printe(PT_Error), 1);
	ck_assert_int_eq(printe(PT_Config), 1);
	ck_assert_int_eq(printe(PT_Multiline), 1);
	ck_assert_int_eq(printe(PT_ShortMultiline), 1);
	ck_assert_int_eq(printe(5), 1);
}
END_TEST

START_TEST(logprint_options)
{
	cfg.uselogging = 0;
	ck_assert_int_eq(logprint(PT_Info), 0);

	cfg.uselogging = 1;
	strcpy(cfg.logfile, "/dev/null");
	strcpy(errorstring, "dummy string");
	ck_assert_int_eq(logprint(PT_Info), 1);
	ck_assert_int_eq(logprint(PT_Error), 1);
	ck_assert_int_eq(logprint(PT_Config), 1);
	ck_assert_int_eq(logprint(PT_Multiline), 0);
	ck_assert_int_eq(logprint(PT_ShortMultiline), 1);
	ck_assert_int_eq(logprint(5), 1);
}
END_TEST

START_TEST(dmonth_return_within_range)
{
	int m;
	m = dmonth(_i);
	ck_assert_int_ge(m, 28);
	ck_assert_int_le(m, 31);
}
END_TEST

START_TEST(mosecs_return_values)
{
	initdb();
	defaultcfg();
	ck_assert_int_eq(cfg.monthrotate, 1);
	ck_assert_int_eq((int)mosecs(), 0);
	sleep(1);
	data.lastupdated = time(NULL);
	ck_assert_int_ne((int)mosecs(), 0);
}
END_TEST

START_TEST(countercalc_no_change)
{
	uint64_t a, b;

	a = b = 0;
	ck_assert_int_eq(countercalc(&a, &b), 0);
	a = b = 1;
	ck_assert_int_eq(countercalc(&a, &b), 0);
}
END_TEST

START_TEST(countercalc_small_change)
{
	uint64_t a, b;

	a = 0;
	b = 1;
	ck_assert_int_eq(countercalc(&a, &b), 1);
	a = 1;
	b = 2;
	ck_assert_int_eq(countercalc(&a, &b), 1);
	b = 3;
	ck_assert_int_eq(countercalc(&a, &b), 2);
}
END_TEST

START_TEST(countercalc_32bit)
{
	uint64_t a, b;

	a = 1;
	b = 0;
	ck_assert(countercalc(&a, &b)==(FP32-1));
}
END_TEST

START_TEST(countercalc_64bit)
{
	uint64_t a, b;

	a = FP32+1;
	b = 0;
	ck_assert(countercalc(&a, &b)==(FP64-FP32-1));
}
END_TEST

START_TEST(addtraffic_does_not_add_zero_traffic)
{
	uint64_t srcmb, destmb;
	int srckb, destkb;

	srcmb=srckb=destmb=destkb=0;
	addtraffic(&destmb, &destkb, srcmb, srckb);

	ck_assert_int_eq(srcmb, 0);
	ck_assert_int_eq(srckb, 0);
	ck_assert_int_eq(destmb, 0);
	ck_assert_int_eq(destkb, 0);
}
END_TEST

START_TEST(addtraffic_with_simple_mb_addition)
{
	uint64_t srcmb, destmb;
	int srckb, destkb;

	destmb=destkb=0;
	srcmb=1;
	srckb=0;
	addtraffic(&destmb, &destkb, srcmb, srckb);

	ck_assert_int_eq(srcmb, 1);
	ck_assert_int_eq(srckb, 0);
	ck_assert_int_eq(destmb, 1);
	ck_assert_int_eq(destkb, 0);
}
END_TEST

START_TEST(addtraffic_with_simple_kb_addition)
{
	uint64_t srcmb, destmb;
	int srckb, destkb;

	destmb=destkb=0;
	srcmb=0;
	srckb=1;
	addtraffic(&destmb, &destkb, srcmb, srckb);

	ck_assert_int_eq(srcmb, 0);
	ck_assert_int_eq(srckb, 1);
	ck_assert_int_eq(destmb, 0);
	ck_assert_int_eq(destkb, 1);
}
END_TEST

START_TEST(addtraffic_with_simple_mixed_addition)
{
	uint64_t srcmb, destmb;
	int srckb, destkb;

	destmb=destkb=0;
	srcmb=1;
	srckb=1;
	addtraffic(&destmb, &destkb, srcmb, srckb);

	ck_assert_int_eq(srcmb, 1);
	ck_assert_int_eq(srckb, 1);
	ck_assert_int_eq(destmb, 1);
	ck_assert_int_eq(destkb, 1);
}
END_TEST

START_TEST(addtraffic_with_multiple_mixed_additions)
{
	uint64_t srcmb, destmb;
	int srckb, destkb, i;

	destmb=destkb=0;
	srcmb=1;
	srckb=1;

	for (i=1; i<=10; i++) {
		addtraffic(&destmb, &destkb, srcmb, srckb);

		ck_assert_int_eq(srcmb, 1);
		ck_assert_int_eq(srckb, 1);
		ck_assert_int_eq(destmb, (uint64_t)i);
		ck_assert_int_eq(destkb, i);
	}
}
END_TEST

START_TEST(addtraffic_with_exact_kb_to_mb_conversion)
{
	uint64_t srcmb, destmb;
	int srckb, destkb;

	destmb=destkb=0;
	srcmb=0;
	srckb=1024;
	addtraffic(&destmb, &destkb, srcmb, srckb);

	ck_assert_int_eq(srcmb, 0);
	ck_assert_int_eq(srckb, 1024);
	ck_assert_int_eq(destmb, 1);
	ck_assert_int_eq(destkb, 0);
}
END_TEST

START_TEST(addtraffic_with_inexact_kb_to_mb_conversion)
{
	uint64_t srcmb, destmb;
	int srckb, destkb;

	destmb=destkb=0;
	srcmb=0;
	srckb=1025;
	addtraffic(&destmb, &destkb, srcmb, srckb);

	ck_assert_int_eq(srcmb, 0);
	ck_assert_int_eq(srckb, 1025);
	ck_assert_int_eq(destmb, 1);
	ck_assert_int_eq(destkb, 1);
}
END_TEST

START_TEST(addtraffic_with_multiple_kb_to_mb_conversions)
{
	uint64_t destmb;
	int destkb;

	destmb=destkb=0;

	addtraffic(&destmb, &destkb, 0, 1023);
	ck_assert_int_eq(destmb, 0);
	ck_assert_int_eq(destkb, 1023);

	addtraffic(&destmb, &destkb, 0, 1);
	ck_assert_int_eq(destmb, 1);
	ck_assert_int_eq(destkb, 0);

	addtraffic(&destmb, &destkb, 0, 1);
	ck_assert_int_eq(destmb, 1);
	ck_assert_int_eq(destkb, 1);

	addtraffic(&destmb, &destkb, 0, 1023);
	ck_assert_int_eq(destmb, 2);
	ck_assert_int_eq(destkb, 0);

	addtraffic(&destmb, &destkb, 0, 1024);
	ck_assert_int_eq(destmb, 3);
	ck_assert_int_eq(destkb, 0);

	addtraffic(&destmb, &destkb, 0, 1025);
	ck_assert_int_eq(destmb, 4);
	ck_assert_int_eq(destkb, 1);

	addtraffic(&destmb, &destkb, 0, 512);
	ck_assert_int_eq(destmb, 4);
	ck_assert_int_eq(destkb, 513);

	addtraffic(&destmb, &destkb, 0, 512);
	ck_assert_int_eq(destmb, 5);
	ck_assert_int_eq(destkb, 1);

	addtraffic(&destmb, &destkb, 0, 2048);
	ck_assert_int_eq(destmb, 7);
	ck_assert_int_eq(destkb, 1);
}
END_TEST

void add_common_tests(Suite *s)
{
	/* Common test cases */
	TCase *tc_common = tcase_create("Common");
	tcase_add_test(tc_common, printe_options);
	tcase_add_test(tc_common, logprint_options);
	tcase_add_loop_test(tc_common, dmonth_return_within_range, 0, 12);
	tcase_add_test(tc_common, mosecs_return_values);
	tcase_add_test(tc_common, countercalc_no_change);
	tcase_add_test(tc_common, countercalc_small_change);
	tcase_add_test(tc_common, countercalc_32bit);
	tcase_add_test(tc_common, countercalc_64bit);
	tcase_add_test(tc_common, addtraffic_does_not_add_zero_traffic);
	tcase_add_test(tc_common, addtraffic_with_simple_mb_addition);
	tcase_add_test(tc_common, addtraffic_with_simple_kb_addition);
	tcase_add_test(tc_common, addtraffic_with_simple_mixed_addition);
	tcase_add_test(tc_common, addtraffic_with_multiple_mixed_additions);
	tcase_add_test(tc_common, addtraffic_with_exact_kb_to_mb_conversion);
	tcase_add_test(tc_common, addtraffic_with_inexact_kb_to_mb_conversion);
	tcase_add_test(tc_common, addtraffic_with_multiple_kb_to_mb_conversions);
	suite_add_tcase(s, tc_common);
}
