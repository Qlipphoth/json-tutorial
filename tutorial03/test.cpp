#include<iostream>
#include<cstdio>
#include<cstring>
#include "tinyjson.h"
using namespace std;

static int main_ret = 0;  // main 函数返回值
static int test_count = 0;  // 测试案例个数
static int test_pass = 0;  // 通过的个数

// 定义了一个用于测试的宏
// equality: 评判标准
// expect: 期望值
// actual: 实际值
// format: 数据打印格式
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

// 用于测试返回值是否与期望的整型相同的情况
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

// 用于测试返回值是否与期望的浮点型相同
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

// 用于测试返回的字符串是否与指定的相同
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")

// 测试返回值是否为指定的布尔型
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

// 测试节点值是否为期望的数字
#define TEST_NUMBER(expect, json)\
    do{\
        tiny_node node;\
        EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, json));\
        EXPECT_EQ_INT(TINY_NUMBER, tiny_get_type(&node));\
        EXPECT_EQ_DOUBLE(expect, tiny_get_number(&node));\
    } while(0)

// 测试错误情况的宏
#define TEST_ERROR(error, json)\
    do{\
        tiny_node node;\
        node.type = TINY_FALSE;\
        EXPECT_EQ_INT(error, tiny_parse(&node, json));\
        EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));\
    } while(0)

// TODO: 重构 null、true、false的测试函数

/// @brief 测试是否能解析出 null 
static void test_parse_null(){
    tiny_node node;  // 一个 json 节点
    node.type = TINY_FALSE;  // 先将节点数据类型置为 false
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "null"));  // 应正常解析，返回 OK 状态码
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));  // 若正确解析完，节点数据类型应被置为 null 
}

/// @brief 测试是否能解析出 true 
static void test_parse_true(){
    tiny_node node;  // 一个 json 节点
    node.type = TINY_FALSE;  // 先将节点数据类型置为 false
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "true"));  // 应正常解析，返回 OK 状态码
    EXPECT_EQ_INT(TINY_TRUE, tiny_get_type(&node));  // 若正确解析完，节点数据类型应被置为 null 
}

/// @brief 测试是否能解析出 false
static void test_parse_false(){
    tiny_node node;  // 一个 json 节点
    node.type = TINY_FALSE;  // 先将节点数据类型置为 false
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "false"));  // 应正常解析，返回 OK 状态码
    EXPECT_EQ_INT(TINY_FALSE, tiny_get_type(&node));  // 若正确解析完，节点数据类型应被置为 null 
}

/// @brief 测试数字解析
static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
        tiny_node v;\
        tiny_init(&v);\
        EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&v, json));\
        EXPECT_EQ_INT(TINY_STRING, tiny_get_type(&v));\
        EXPECT_EQ_STRING(expect, tiny_get_string(&v), tiny_get_string_length(&v));\
        tiny_free(&v);\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
#if 1
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
#endif
}



/// @brief 测试输入为空白的情况
static void test_parse_expect_value(){
    TEST_ERROR(TINY_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(TINY_PARSE_EXPECT_VALUE, " ");
}

/// @brief 测试输入为非法字符的情况
static void test_parse_invalid_value(){
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "?");

#if 1
    /* invalid number */
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "nan");
#endif

}

/// @brief 测试一个值后除了空白还有其他值的情况
static void test_parse_root_not_singular(){
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "null x");

#if 0
    /* invalid number */
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "0x123");
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "0.1.1.1");
#endif

}

/// @brief 测试数字越界的情况
static void test_parse_number_too_big() {
#if 1
    TEST_ERROR(TINY_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(TINY_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(TINY_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(TINY_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
#if 1
    TEST_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
#endif
}

static void test_parse_invalid_string_char() {
#if 1
    TEST_ERROR(TINY_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(TINY_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
#endif
}

static void test_access_null() {
    tiny_node node;
    tiny_init(&node);
    // 先把值设为字符串，那么做可以测试设置其他类型时，有没有调用 `tiny_free()` 去释放内存。
    tiny_set_string(&node, "a", 1);
    tiny_set_null(&node);
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));
    tiny_free(&node);
}

static void test_access_boolean() {
    tiny_node node;
    tiny_init(&node);
    tiny_set_string(&node, "a", 1);
    tiny_set_boolean(&node, 1);
    EXPECT_TRUE(tiny_get_boolean(&node));
    tiny_set_boolean(&node, 0);
    EXPECT_FALSE(tiny_get_boolean(&node));
    tiny_free(&node);
}

static void test_access_number() {
    tiny_node node;
    tiny_init(&node);
    tiny_set_string(&node, "a", 1);
    tiny_set_number(&node, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, tiny_get_number(&node));
    tiny_free(&node);
}

static void test_access_string() {
    tiny_node node;
    tiny_init(&node);
    tiny_set_string(&node, "", 0);
    EXPECT_EQ_STRING("", tiny_get_string(&node), tiny_get_string_length(&node));
    tiny_set_string(&node, "Hello", 5);
    EXPECT_EQ_STRING("Hello", tiny_get_string(&node), tiny_get_string_length(&node));
    tiny_free(&node);
}

/// @brief 进行所有测试
static void test_parse(){
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

int main(){
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}


